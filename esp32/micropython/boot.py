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
(wlan,wifi_qrtxt) = wifi_connect()
if wifi_qrtxt!=None:
  show_qr_code(wifi_qrtxt)
  i=0
  while not wlan.isconnected():
    c = i%4
    lcd.text(0,5,"Connect " + ("."*c) + (" "*(4-c)) )
    i = i + 1
    time.sleep(1)

wifi_sta = None
wifi_ap = None
wifi_connect = None

include('wanip.py')
if wifi_qrtxt=="":
  setup_wanip()

setup_wanip=None
wifi_qrtxt=None

gc.collect()
dmesg('mem: %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )


