htmlformat="<!DOCTYPE html><html><body>%s</body></html>"
function MakeWeekHTMLForm(wd,wp)
	local s='<table>'
	for i,k in ipairs(wd) do
		v=wp[k]
		s=s..string.format('<tr><td>%s</td><td><input type="text" name="%s" value="%s"></td></tr>',k,k,v)
	end
	s=s..'</table><br>'
	return s
end
srv=net.createServer(net.TCP)
httpsrvdbg=false
httpsrvreq=nil
srv:listen(80,function(conn)
    conn:on("receive", function(client,request)
        local buf = "";
        local _, _,method,path,vars=string.find(request, "([A-Z]+) (.+)?(.+) HTTP");
        if(method == nil)then
            _, _,method,path=string.find(request, "([A-Z]+) (.+) HTTP");
        end
        local _GET = {}
        if (vars ~= nil)then
            for k, v in string.gmatch(vars,"(%w+)=([^&]+)&*") do
                _GET[k] = string.gsub(v,"(%%[%dA-F][%dA-F])",function(w) n="0x"..string.sub(w,2) return string.char(n) end )
                if(httpsrvdbg)then print(k.."=".._GET[k]) end
            end
        end
        httpsrvreq=request
        if(path=="/index.htm")then
			file.open("index.htm")
			local h,m,s,mo,d,y,wd=getRTCtime(2)
			wd=weekdays[wd]
			dt=string.format("%s %02d/%02d/%02d %02dh%02d",wd,d,mo,y,h,m)
			buf=string.format(file.read(),dt,MakeWeekHTMLForm(weekdays,weekprog))
			file.close()
        end
        if(path=="/sa.htm")then
			if(_GET.SwitchState ~= nil)then
				v=tonumber(_GET.SwitchState)
				gpio.mode(7,gpio.OUTPUT)
				if(v~=0)then
					PwrSwitchState=true
					gpio.write(7,gpio.HIGH)
				else
					PwrSwitchState=false
					gpio.write(7,gpio.LOW)
				end
				PwrSwitchForced=true;
			end
			path="/su.htm"
        end
        if(path=="/sp.htm")then
			local f=file.open("weekprog.txt","w")
			for d,l in pairs(_GET) do
				weekprog[d]=l
				f:write(d.." "..l.."\n")
			end
			f:close()
			path="/su.htm"
        end
        if(path=="/su.htm")then
			local ps="OFF"
			if(PwrSwitchState)then ps="ON" end
			buf=string.format(htmlformat,"<h2>Switch is "..ps.."</h2><br>")
        end
        if(path=="/debug.htm" and httpsrvdbg)then
			file.open("debug.htm")
			buf=file.read()
			file.close()
			buf=string.format(buf,wifi.sta.getip(),node.heap())
        end
        if(path=="/dbgconsole.htm" and httpsrvdbg)then
			buf=""
			if(_GET.pin ~= nil and _GET.value ~= nil)then
				  pinNumber=tonumber(_GET.pin)
				  value=tonumber(_GET.value)
				  if(pinNumber ~= nil and pinNumber>=0 and value ~= nil)then
					buf=buf.."pin #"..pinNumber.." -> "..value.."<br>"
					gpio.mode(pinNumber,gpio.OUTPUT)
					if(value~=0)then gpio.write(pinNumber,gpio.HIGH) else gpio.write(pinNumber,gpio.LOW) end
				  end
			end
			if(_GET.command ~= nil)then
				local cmd=_GET.command
				if(string.sub(cmd,1,1)=="=")then
					cmd="_rval"..cmd
				end
				local f=loadstring(cmd)
				_rval=nil
				f()
				if(_rval ~= nil)then
					buf=buf..tostring(_rval).."<br>"
				end
			end
			buf=string.format(htmlformat,buf)
        end
        httpsrvreq=nil
		client:send(buf)
		client:close()
        collectgarbage()
    end)
end)
