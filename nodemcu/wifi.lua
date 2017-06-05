WanIP="10.0.0.1"
WanUpdateURL="www"

function readWiFiConfigFile(fname)
	local f = file.open(fname,"r")
	local ssid = string.gsub(f:readline(),"\n","")
	local pwd = string.gsub(f:readline(),"\n","")
	WanUpdateURL = string.gsub(f:readline(),"\n","")
	f:close()
	f = file.open("wan.cfg","r")
	WanIP = string.gsub(f:readline(),"\n","")
	f:close()
	return ssid,pwd
end

function UpdateWanIP(cb,cbchange)
	http.get("http://icanhazip.com",nil,
		function(code,data)
			if(data~=nil)then
				NewIP=string.sub(data,0,-2)
				if(NewIP~=WanIP)then
					WanIP=NewIP
					http.get(WanUpdateURL,nil,function(code,data) print(data) end)
					local f = file.open("wan.cfg","w")
					f:writeline(tostring(WanIP))
					f:close()
					if(cbcbchange~=nil)then
						cbchange(WanIP)
					end
				end
			end
			if(cb~=nil)then
				cb(WanIP)
			end
		end)
end

function startWiFi(cb)
	wifi.setmode(wifi.STATION)
	local ssid,pwd = readWiFiConfigFile("wifi.cfg")
	print_message("> "..ssid)
	wifi.sta.config(ssid,pwd)

	local wificontmr=tmr.create()
	wificontmr:register(5000,tmr.ALARM_AUTO,
		function(t)
			if(wifi.sta.getip()==nil)then
				return
			end
			t:unregister()
			print_message("WiFi ready")
			print_message(tostring(wifi.sta.getip()))
			UpdateWanIP( function(wip)
							print_message("WAN ready")
							print_message(tostring(wip))
						end
						,nil)
			sntp.sync({"0.fr.pool.ntp.org","1.fr.pool.ntp.org","2.fr.pool.ntp.org","3.fr.pool.ntp.org"},
				function(sec, usec, server, info)
					print_message("SNTP sync'd")
				end,
				function() print("SNTP failed") end)
			local wanupdatetmr=tmr.create()
			wanupdatetmr:register(1800000,tmr.ALARM_AUTO,
				function(t)
					UpdateWanIP( nil , function(wip)
						print_message("WAN IP update")
						print_message(tostring(wip))
					end)
				end)
			wanupdatetmr:start()
			if(cb~=nil)then
				cb()
			end
		end)
	
	wificontmr:start()
end
