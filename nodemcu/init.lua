print("--- NodeMCU Web Interface ---")
wifi.setmode(wifi.STATION)
file.open("wifi.cfg")
local ssid=string.gsub(file.readline(),"\n","")
local pwd=string.gsub(file.readline(),"\n","")
PREVIP=string.gsub(file.readline(),"\n","")
WanUpdateURL=string.gsub(file.readline(),"\n","")
IPUpdateCount=0
wifi.sta.config(ssid,pwd)
UpdateWanIP=function()
	http.get("http://icanhazip.com",nil,
		function(code,data)
			NewIP=string.sub(data,0,-2)
			if(NewIP~=PREVIP)then
				print("Change Wan IP from "..PREVIP.." to "..NewIP.." (count="..IPUpdateCount..")")
				PREVIP=NewIP
				http.get(WanUpdateURL,nil,function(code,data) print(data) end)
			end
			if(IPUpdateCount==0)then print("Wan IP = "..PREVIP) end
			IPUpdateCount=IPUpdateCount+1
		end)
end
tmr.register(0,8000,tmr.ALARM_SINGLE,
    function()
		print("")
        print("Local IP = "..wifi.sta.getip())
		UpdateWanIP()
        dofile("httpsrv.lua")
        tmr.unregister(0)
        tmr.register(0,1800000,tmr.ALARM_AUTO,UpdateWanIP)
    end)
tmr.start(0)
print("Waiting for connection")
