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

include('jsdb.py')

include('wifi.py')
(wlan,contxt) = wifi_connect()
if contxt!="":
  show_qr_code(contxt)
  print("wait for connection")
  while not wlan.isconnected():
    time.sleep(1)

#wifi_sta = None
#wifi_ap = None
#wifi_connect = None

#include('wanip.py')
#if wlan.isconnected():
#  setup_wanip()
#setup_wanip=None

gc.collect()
dmesg('mem: %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )


