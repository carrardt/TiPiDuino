httpsrvdbg=false
htmlformat="<!doctype html><html><body>%s</body></html>"
htmlrefresh='<!doctype html><html><head><meta http-equiv="refresh" content="15;url=su.htm"></head><body>%s %02d/%02d/%02d %02dh%02d<br>WAN IP %s<br><h2>Switch is %s</h2></body></html>'
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
                if(httpsrvdbg)then print_message(k.."=".._GET[k]) end
            end
        end
        httpsrvreq=request
        if(path=="/son.htm")then
			ForceSwitchState(true)
			path="/su.htm"
        end
        if(path=="/soff.htm")then
			ForceSwitchState(false)
			path="/su.htm"
        end
        if(path=="/sp.htm")then
			UpdateWeekProg(_GET)
			path="/index.htm"
        end
        if(path=="/lcdhi.htm")then
			set_screen_contrast(70)
			path="/su.htm"
        end
        if(path=="/lcdmed.htm")then
			set_screen_contrast(60)
			path="/su.htm"
        end
        if(path=="/lcdlow.htm")then
			set_screen_contrast(50)
			path="/su.htm"
        end
        if(path=="/index.htm")then
			local f=file.open("index.htm","r")
			buf=string.format(f:read(),MakeWeekHTMLForm(weekdays,weekprog))
			f:close()
        end
        if(path=="/su.htm")then
			local ps="OFF"
			if(PwrSwitchState)then ps="ON" end
			local h,m,s,mo,d,y,wd=getRTCtime(2)
			wd=weekdays[wd]
			buf=string.format(htmlrefresh,wd,d,mo,y,h,m,WanIP,ps)
        end
        httpsrvreq=nil
		client:send(buf)
		client:close()
        collectgarbage()
    end)
end)
