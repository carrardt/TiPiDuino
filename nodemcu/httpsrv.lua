print(wifi.sta.getip())
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
			buf=file.read()
			file.close()
			buf=string.format(buf,wifi.sta.getip(),node.heap())
        end
        if(path=="/output.htm")then
			buf="<!DOCTYPE html><html><body>"
			if(_GET.pin ~= nil and _GET.value ~= nil)then
				  pinNumber=tonumber(_GET.pin)
				  value=tonumber(_GET.value)
				  if(pinNumber ~= nil and pinNumber>=0 and value ~= nil)then
					buf=buf.."pin #"..pinNumber.." -> "..value.."<br>"
					gpio.mode(pinNumber, gpio.OUTPUT)
					if(value ~= 0)then
						gpio.write(pinNumber, gpio.HIGH)
					else
						gpio.write(pinNumber, gpio.LOW)
					end
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
			buf=buf.."</body></html>"
        end
        httpsrvreq=nil
		client:send(buf)
		client:close()
        collectgarbage()
    end)
end)
