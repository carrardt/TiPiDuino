httpsrvdbg=false
printToLCD=true
LCDScreenOn=true

uart.setup(0,19200,8,0,1,1)

screensavetmr=tmr.create()
screensavetmr:register(60000,tmr.ALARM_AUTO,
	function(t)
		if(printToLCD and LCDScreenOn)then
			print("&~p")
			tmr.delay(100000)
			LCDScreenOn=false
		end
	end)
screensavetmr:start()

function clear_console()
	if(printToLCD)then
			print("-- CLEAR --")
			tmr.delay(100000)
			print("&~C")
			tmr.delay(100000)
	end
end

function set_screen_contrast(i)
	if(printToLCD)then
		print("&~c"..i)
		tmr.delay(100000)
	end
end

function print_message(s)
		if(printToLCD and not LCDScreenOn)then
			print("&~P")
			tmr.delay(100000)
			LCDScreenOn=true
		end
		print(s)
		if(printToLCD)then
			tmr.delay(100000)
			screensavetmr:stop()
			screensavetmr:start()
		end
end

clear_console()
print_message("IoT Switch")
wifi.setmode(wifi.STATION)

file.open("wifi.cfg")
local ssid=string.gsub(file.readline(),"\n","")
local pwd=string.gsub(file.readline(),"\n","")
WanIP=string.gsub(file.readline(),"\n","")
WanUpdateURL=string.gsub(file.readline(),"\n","")
PwrSwitchState=false
PwrSwitchForced=false
IPUpdateCount=0
wifi.sta.config(ssid,pwd)

function UpdateWanIP()
	http.get("http://icanhazip.com",nil,
		function(code,data)
			NewIP=string.sub(data,0,-2)
			if(NewIP~=WanIP)then
				WanIP=NewIP
				http.get(WanUpdateURL,nil,function(code,data) print(data) end)
			end
			if(IPUpdateCount==0)then
				print_message(tostring(WanIP))
			end
			IPUpdateCount=IPUpdateCount+1
		end)
end

function getRTCtime(tz)
   function isleapyear(y) if ((y%4)==0) or (((y%100)==0) and ((y%400)==0)) == true then return 2 else return 1 end end
   function daysperyear(y) if isleapyear(y)==2 then return 366 else return 365 end end           
   local monthtable = {{31,28,31,30,31,30,31,31,30,31,30,31},{31,29,31,30,31,30,31,31,30,31,30,31}}
   local secs=rtctime.get()
   local d=secs/86400
   local y=1970   
   local m=1
   local wday=1+(d+3)%7
   while (d>=daysperyear(y)) do d=d-daysperyear(y) y=y+1 end
   while (d>=monthtable[isleapyear(y)][m]) do d=d-monthtable[isleapyear(y)][m] m=m+1 end
   secs=secs-1104494400-1104494400+(tz*3600)
   return (secs%86400)/3600,(secs%3600)/60,secs%60,m,d+1,y,wday
end

function readWeekProgram(fname)
	local f=file.open(fname,"r")
	local l=nil
	local wp={}
	repeat
		l=f:readline()
		if(l~=nil)then
			local spc=l:find(' ')
			local eol=l:find('\n')
			local d=string.sub(l,1,spc-1)
			local p=string.sub(l,spc+1,eol-1)
			wp[d]=p
		end
	until l==nil
	return wp
end

function WeekProgRun()
	local hour,minute,second,month,day,year,wday=getRTCtime(2)
	local d=weekdays[wday]
	local k=string.format("%d:%d",hour,minute)
	local a=""
	local progHit=false
	for w in string.gmatch(weekprog[d],"([^;]+)") do
		local e=w:find('=')
		if(e~=nil)then
			local t=w:sub(1,e-1)
			local h=nil
			local m=nil
			local s=t:find(':')
			if(s~=nil)then
				h=tonumber(t:sub(1,s-1))
				m=tonumber(t:sub(s+1))
			end
			if(h~=nil and m~=nil and (h<hour or (h==hour and m<=minute)))then
				if(h==hour and m==minute)then
					PwrSwitchForced=false
					progHit=true;
				end
				if(not PwrSwitchForced)then
					a=w:sub(e+1)
				end
			end
		end
	end
	if(progHit)then
		print_message(d)
		print_message(k.." "..a)
	end
	if(a:upper()=="ON")then
		PwrSwitchState=true
		gpio.mode(7,gpio.OUTPUT)
		gpio.write(7,gpio.HIGH)
	end
	if(a:upper()=="OFF")then
		PwrSwitchState=false
		gpio.mode(7,gpio.OUTPUT)
		gpio.write(7,gpio.LOW)
	end
end

weekdays={"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"}
weekprog=readWeekProgram("weekprog.txt")
for k,v in pairs(weekdays) do
	if(weekprog[v]==nil)then weekprog[v]="0:0=off" end
end

local wificontmr=tmr.create()
wificontmr:register(8000,tmr.ALARM_SINGLE,
    function(t)
        t:unregister()
        print_message(tostring(wifi.sta.getip()))
		UpdateWanIP()
		sntp.sync({"0.fr.pool.ntp.org","1.fr.pool.ntp.org","2.fr.pool.ntp.org","3.fr.pool.ntp.org"},
			function(sec, usec, server, info)
				print_message('SNTP sync')
			end,
			function() print('SNTP sync failed') end)
        dofile("httpsrv.lua")
    end)
wificontmr:start()

local wanupdatetmr=tmr.create()
wanupdatetmr:register(1800000,tmr.ALARM_AUTO,function(t) UpdateWanIP() end)
wanupdatetmr:start()

local weekprogtmr=tmr.create()
weekprogtmr:register(20000,tmr.ALARM_AUTO,function(t) WeekProgRun() end)
weekprogtmr:start()

print_message("WiFi setup")
