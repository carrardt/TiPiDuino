wifi.setmode(wifi.STATION)
file.open("wifi.cfg")
local ssid=string.gsub(file.readline(),"\n","")
local pwd=string.gsub(file.readline(),"\n","")
wifi.sta.config(ssid,pwd)
for k,v in pairs(file.list()) do print(k.." ("..v.." bytes)") end
tmr.register(0,8000,tmr.ALARM_SINGLE,
    function()
        print("IP:"..wifi.sta.getip())
        dofile("httpsrv.lua")
        tmr.unregister(0)
        end)
tmr.start(0)
print("Waiting for connection")
