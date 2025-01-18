import time
import network
import requests
  
def wifi_connect():
  wlan = None
  wificonf = None
  try:
    wificonf = open('config/wifi.txt')
  except:
    wificonf = None
  if wificonf:
    dmesg('Wifi ...')
    WIFICON = [ s.strip() for s in open('config/wifi.txt').readlines() ]
    (ssid,password,hname) = WIFICON
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
  else:
    dmesg('Connect to')
    wlan = network.WLAN(network.AP_IF)
    ap_id = 0
    for b in wlan.config('mac'):
      ap_id = (ap_id*7) ^ int(b)
    ap_password="%08X"%ap_id
    ap_ssid="TH%s"%ap_password
    dmesg(ap_ssid)
    dmesg("password is")
    dmesg(ap_password)
    wlan.config(ssid=ap_ssid,password=ap_password)
    wlan.active(True)
  dmesg(wlan.ifconfig()[0])
  return wlan

# Check / update wan IP address
def setup_wanip():
  try:
    WANIP = [ s.strip() for s in open('config/wanip.txt').readlines() ]
    curwanip=requests.get("https://api.ipify.org").content.decode("utf8")
    if WANIP[1]!=curwanip:
      WANIP[1]=curwanip
      requests.get(WANIP[2]).content.decode('utf8')
      f=open('config/wanip.txt','w')
      f.write('\n'.join(WANIP))
      f.close()
    dmesg("WAN=%s"%curwanip)
  except OSError as e:
    dmesg("Can't setup WAN")

