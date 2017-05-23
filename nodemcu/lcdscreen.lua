printToLCD=true
LCDScreenOn=false
LCDLineDelay=100000
LCDBitmapDelay=10000
LCDScreenSaveDelay=120000

screensavetmr=tmr.create()
screensavetmr:register(LCDScreenSaveDelay,tmr.ALARM_AUTO,
	function(t)
		if(printToLCD and LCDScreenOn)then
			print("&~p")
			tmr.delay(LCDLineDelay)
			LCDScreenOn=false
		end
	end)
screensavetmr:start()

function clear_console()
	if(printToLCD)then
			print("--")
			tmr.delay(LCDLineDelay)
			print("&~C")
			tmr.delay(LCDLineDelay)
	end
end

function set_screen_contrast(i)
	if(printToLCD)then
		print("&~c"..i)
		tmr.delay(LCDLineDelay)
	end
end

function wakeUpLCD()
	screensavetmr:stop()
	if(not LCDScreenOn)then
		print("&~P")
		tmr.delay(LCDLineDelay)
		LCDScreenOn=true
	end
	screensavetmr:start()
end

function print_message(s)
		if(printToLCD)then
			wakeUpLCD()
			if(#s>14)then s=s:sub(1,14) end
		end
		print(s)
		if(printToLCD)then
			tmr.delay(LCDLineDelay)
		end
end

function drawBitmapFile(fname)
	if(not printToLCD)then
		return
	end
	wakeUpLCD()
	local f = file.open(fname,"rb")
	local data = f:read()
	f:close()
	for CI = 1, #data/4 do
		local i = CI-1
		if( i%21 == 0 )then
			print(string.format("&~l0 %d",i/21))
			tmr.delay(LCDBitmapDelay)
		end
		local b0 = data:sub(i*4+1,i*4+1):byte()
		local b1 = data:sub(i*4+2,i*4+2):byte()
		local b2 = data:sub(i*4+3,i*4+3):byte()
		local b3 = data:sub(i*4+4,i*4+4):byte()
		print( string.format("&~d0x%02X%02X%02X%02X",b3,b2,b1,b0) )
		tmr.delay(LCDBitmapDelay)
	end
end
