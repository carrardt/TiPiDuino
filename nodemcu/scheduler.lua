-- power switch state --
PwrSwitchState=false
PwrSwitchForced=false

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

function PrintSwitchState(state)
	local msg=""
	if(state)then
		msg="ON"
	else
		msg="OFF"
	end
	local h,m,s,mo,d,y,wd=getRTCtime(2)
	wd = weekdays[wd]
	wd = wd:sub(1,2)
	print_message(string.format("%s %02d:%02d %s",wd,h,m,msg))
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
	if(progHit)then
		PrintSwitchState(PwrSwitchState)
	end
end

weekdays = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"}
weekprog = readWeekProgram("weekprog.txt")
for k,v in pairs(weekdays) do
	if(weekprog[v]==nil)then weekprog[v]="0:0=off" end
end

function ForceSwitchState(state)
	PwrSwitchState = state
	PwrSwitchForced = true;
	gpio.mode(7,gpio.OUTPUT)
	if(state)then
		gpio.write(7,gpio.HIGH)
	else
		gpio.write(7,gpio.LOW)
	end
	PrintSwitchState(state)
end

function startWeekScheduler()
	local weekprogtmr=tmr.create()
	weekprogtmr:register(20000,tmr.ALARM_AUTO,function(t) WeekProgRun() end)
	weekprogtmr:start()
end

