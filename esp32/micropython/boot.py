import esp
esp.osdebug(None)
import machine
import time

exec(open('console.py').read())
exec(open('wifi.py').read())
#exec(open('tools.py').read())

clear()
import gc
gc.enable()
gc.collect()
dmesg('* %d/%d' % ( gc.mem_alloc()/1024 , gc.mem_free()/1024 ) )

wlan = wifi_connect()

gc.collect()
dmesg('* %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )

