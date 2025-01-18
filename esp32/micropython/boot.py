import esp
esp.osdebug(None)
import machine
import time

exec(open('console.py').read())

clear()
import gc
gc.enable()
gc.collect()

exec(open('wifi.py').read())
wlan = wifi_connect()
wifi_sta=None
wifi_ap=None
wifi_connect=None

exec(open('wanip.py').read())  
if wlan.isconnected():
  setup_wanip()
setup_wanip=None

gc.collect()
dmesg('mem: %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )

