printToLCD			= true
LCDScreenOn			= false
LCDLineDelay  		= 100000
LCDBitmapDelay		= 5000
LCDScreenSaveDelay	= 60000

function lcdprint(s)
	print(s:sub(1,14))
end

screensavetmr=tmr.create()
screensavetmr:register(LCDScreenSaveDelay,tmr.ALARM_AUTO,
	function(t)
		if(printToLCD and LCDScreenOn)then
			lcdprint("&~p")
			tmr.delay(LCDLineDelay)
			LCDScreenOn=false
		end
	end)
screensavetmr:start()

function clear_console()
	if(printToLCD)then
			lcdprint("             ")
			tmr.delay(LCDLineDelay)
			lcdprint("&~C")
			tmr.delay(LCDLineDelay)
	end
end

function set_screen_contrast(i)
	if(printToLCD)then
		lcdprint("&~c"..i)
		tmr.delay(LCDLineDelay)
	end
end

function wakeUpLCD()
	screensavetmr:stop()
	if(not LCDScreenOn)then
		lcdprint("&~P")
		tmr.delay(LCDLineDelay)
		LCDScreenOn=true
	end
	screensavetmr:start()
end

function print_message(s)
	if(printToLCD)then
		wakeUpLCD()
		lcdprint(s)
		tmr.delay(LCDLineDelay)
	else
		print(s)
	end
end

function print_power_status(d,h,m,s,temp,hum)
	if(printToLCD)then
		wakeUpLCD()
		lcdprint("&~l0 0")
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~D"..d)
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~l0 1")
		tmr.delay(LCDBitmapDelay)
		lcdprint(string.format("&~D%02d",h))
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~l0 2")
		tmr.delay(LCDBitmapDelay)
		lcdprint(string.format("&~D%02d",m))
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~l0 3")
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~D"..s)
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~l0 4")
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~D"..temp)
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~l0 5")
		tmr.delay(LCDBitmapDelay)
		lcdprint("&~D"..hum)
		tmr.delay(LCDBitmapDelay)
	else
		print(s)
	end
end


function drawBitmapFile(fname)
	if(not printToLCD)then
		return
	end
	wakeUpLCD()
	local f = file.open(fname,"rb")
	local data = f:read()
	local a,b,c
	f:close()
	for CI = 1, #data do
		local i = CI-1
		if(i%84==0)then
			tmr.delay(LCDBitmapDelay)
			uart.write(0,string.format("&~l0 %d\n",i/84))
		end
		if(i%4==0)then
			tmr.delay(LCDBitmapDelay)
			uart.write(0,"&~d0x")
			a=data:sub(CI,CI):byte()
		end
		if(i%4==1)then b=data:sub(CI,CI):byte() end
		if(i%4==2)then c=data:sub(CI,CI):byte() end
		if(i%4==3)then
			uart.write(0,string.format("%02X%02X%02X%02X\n",data:sub(CI,CI):byte(),c,b,a))
		end
	end
end
