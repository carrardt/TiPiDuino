import esp
esp.osdebug(None)
import machine
import time

def include(fname):
  bc=compile(open(fname).read(),fname,'exec')
  exec(bc)

include('console.py')

clear()
import gc
gc.enable()
gc.collect()

include('wifi.py')
wlan = wifi_connect()
wifi_sta=None
wifi_ap=None
wifi_connect=None

include('wanip.py')
if wlan.isconnected():
  setup_wanip()
setup_wanip=None

gc.collect()
dmesg('mem: %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )

