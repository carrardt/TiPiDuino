import time
import network

def wifi_sta(ssid,password,hname):
  dmesg('Wifi ...')
  network.hostname(hname)
  wlan = network.WLAN(network.STA_IF)
  wlan.active(True)
  for wap in [ap[0].decode('utf8') for ap in wlan.scan()[:3]]:
    dmesg(wap)
  wlan.connect(ssid,password)
  retry=5    
  while not wlan.isconnected() and retry>0:
    dmesg("wait %ds"%(retry*5))
    time.sleep(5)
    retry = retry - 1
  return wlan

def wifi_ap():
  dmesg('Connect to')
  wlan = network.WLAN(network.AP_IF)
  ap_id = 0
  for b in wlan.config('mac'): ap_id = (ap_id*7) ^ int(b)
  ap_password="%08X"%ap_id
  ap_ssid="TH%s"%ap_password
  dmesg(ap_ssid)
  dmesg("password is")
  dmesg(ap_password)
  wlan.config(ssid=ap_ssid,password=ap_password,security=network.WLAN.SEC_WPA,max_clients=3)
  wlan.active(True)
  return wlan

def wifi_connect():
  wlan = None
  WIFICON = None
  try:
    WIFICON = [ s.strip() for s in open('config/wifi.txt').readlines() ]
  except:
    WIFICON = None
  if WIFICON:
    wlan = wifi_sta(*WIFICON)
  else:
    wlan = wifi_ap()
  dmesg(wlan.ifconfig()[0])
  return wlan

