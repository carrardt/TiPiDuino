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
		msg="off"
	end
	local h,m,s,mo,d,y,wd=getRTCtime(2)
	wd = weekdays[wd]
	wd = wd:sub(1,2)
	print_power_status(wd,h,m,msg)
	-- print_message(string.format("%s %02d:%02d %s",wd,h,m,msg))
end

function pairsByKeys(t, f)
  local a = {}
  for n in pairs(t) do table.insert(a, n) end
  table.sort(a, f)
  local i = 0
  local iter = function ()
	i = i + 1
	if(a[i]==nil)then return nil
	else return a[i], t[a[i]]
	end
  end
  return iter
end

function ParseDaySchedule(s)
	local l={}
	for w in string.gmatch(s,"([^;]+)") do
		local e=w:find('=')
		if(e~=nil)then
			local t=w:sub(1,e-1)
			local s=t:find(':')
			local minutes=nil
			if(s~=nil)then
				local h=tonumber(t:sub(1,s-1))
				local m=tonumber(t:sub(s+1))
				if(h~=nil and m~=nil)then minutes=h*60+m end
			else
				minutes=tonumber(t)
			end
			local a=w:sub(e+1)
			local state=nil
			if(a:upper()=="ON")then state=true end
			if(a:upper()=="OFF")then state=false end
			if(state~=nil and minutes~=nil)then
				l[minutes]=state
			end
		end
	end
	return pairsByKeys(l)
end

function WeekProgRun()
	local hour,minute,second,month,day,year,wday=getRTCtime(2)
	local target=hour*60+minute
	local d=weekdays[wday]
	local progHit=false
	local state=nil
	for m,s in ParseDaySchedule(weekprog[d]) do
		if(m<target and not PwrSwitchForced)then
			state=s
		end
		if(m==target)then
			PwrSwitchForced=false
			progHit=true
			state=s
		end
	end
	if(state~=nil)then
		PwrSwitchState=state
		gpio.mode(7,gpio.OUTPUT)
		if(state)then gpio.write(7,gpio.HIGH)
		else gpio.write(7,gpio.LOW)
		end
	end
	if(progHit)then
		PrintSwitchState(PwrSwitchState)
	end
end

function UpdateWeekProg(wd)
	local f=file.open("weekprog.txt","w")
	for d,l in pairs(wd) do
		local sched=""
		for m,s in ParseDaySchedule(l) do
			local a="OFF"
			if(s)then a="ON" end
			if(sched~="")then sched=sched..";" end
			sched=sched..string.format("%02d:%02d=%s",m/60,m%60,a)
		end
		weekprog[d]=sched
		f:write(d.." "..sched.."\n")
	end
	f:close()
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

