uart.setup(0,19200,8,0,1,1)
dofile("lcdscreen.lua")

clear_console()
print_message("Smart Switch")

dofile("wifi.lua")
dofile("scheduler.lua")

startWiFi(
	function()
		startWeekScheduler()
		dofile("httpsrv.lua")
	end)

local drawbmtmr=tmr.create()
drawbmtmr:register(23000,tmr.ALARM_SINGLE,function(t) drawBitmapFile("logo.bm") end)
drawbmtmr:start()
