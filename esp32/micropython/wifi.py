import time
import network

def wifi_sta(ssid,password,hname):
  dmesg('Wifi ...')
  network.hostname(hname)
  wlan = network.WLAN(network.STA_IF)
  wlan.active(True)
  apreach=False
  for wap in [ap[0].decode('utf8') for ap in wlan.scan()[:3]]:
    if wap==ssid:
      dmesg("%s*"%wap)
      apreach=True
    else:
      dmesg(wap)
  if not apreach:
    dmesg("%s!"%ssid)
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
  ap_password="PW%08X"%ap_id
  ap_ssid="AP%s"%ap_id
  wlan.config(ssid=ap_ssid,password=ap_password,security=wlan.SEC_WPA2,max_clients=3)
  wlan.active(True)
  return (wlan,"WIFI:T:WPA2;S:%s;P:%s;"%(ap_ssid,ap_password))

def wifi_connect():
  wlan = None
  WIFICON = None
  WIFIQRTXT = None
  try:
    WIFICON = jsdb_get_value('wifi')
  except:
    WIFICON = { 'ssid':'' , 'key':'' , 'host':'' }
  if WIFICON['ssid'] != '' :
    wlan = wifi_sta(WIFICON['ssid'],WIFICON['key'],WIFICON['host'])
  else:
    (wlan,WIFIQRTXT) = wifi_ap()
  dmesg(wlan.ifconfig()[0])
  return (wlan,WIFIQRTXT)

