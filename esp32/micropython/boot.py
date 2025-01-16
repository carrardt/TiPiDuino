#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/boot.py

import esp
esp.osdebug(None)
import machine
import time
import os

SERIAL_CONSOLE=None
#SERIAL_CONSOLE=(2,19200)

dmesg_out = None
if SERIAL_CONSOLE:
  dmesg_out = machine.UART(SERIAL_CONSOLE[0],SERIAL_CONSOLE[1])
  dmesg_out.init(SERIAL_CONSOLE[1],bits=8,parity=None,stop=1)

def dmesg(s,mend="\n"):
  print(s,end=mend)
  if dmesg_out:
    dmesg_out.write(s+mend)
    time.sleep_ms(250)

def clear():
  print("---")
  if dmesg_out:  
    dmesg_out.write("&~C\n")
    time.sleep_ms(250)

clear()
import gc
gc.enable()

import network
import requests
gc.collect()
dmesg('* %d/%d' % ( gc.mem_alloc()/1024 , gc.mem_free()/1024 ) )

def mkfile():
  l = input()
  if l[0:2] == "#<":
    fname=l[2:]
    dmesg("W %s"%fname)
    f=open(l[2:],"w")
    while True:
      try:
        l=input()
      except EOFError:
        break
      f.write(l+"\n")
    f.close()
  elif l[0:2] == "#$":
    url=l[2:]
    fname=url[url.rfind('/')+1:]
    dmesg("w %s"%fname)
    f=open(fname,"w")
    f.write(requests.get(url).content.decode('utf8'))
    f.close()

def chkfile(fname):
  ln=1
  for l in open(fname).readlines():
    dmesg("[% 3d] %s" % (ln,l) , end='')
    ln=ln+1

# Wifi connection
def wifi_connect():
  wlan = None
  wificonf = None
  try:
    wificonf = open('config/wifi.txt')
  except:
    wificonf = None
  if wificonf:
    dmesg('Wifi STA')
    WIFICON = [ s.strip() for s in open('config/wifi.txt').readlines() ]
    (ssid,password,hname) = WIFICON
    dmesg("hname=%s"%name)
    network.hostname(hostname)
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    for wap in [ap[0].decode('utf8') for ap in wlan.scan()[:3]]:
      dmesg(wap)
    dmesg("%s,p=%s"%(ssid,password))
    wlan.connect(ssid,password)
    retry=5    
    while not wlan.isconnected() and retry>0:
      dmesg("Wait con. %ds"%(retry*5))
      time.sleep(5)
      retry = retry - 1
  else:
    dmesg('Wifi AP')
    wlan = network.WLAN(network.AP_IF)
    ap_id = 0
    for b in wlan.config('mac'):
      ap_id = (ap_id*7) ^ int(b)
    ap_password="%08X"%ap_id
    ap_ssid="TH%s"%ap_password
    dmesg("AP=%s"%ap_ssid)
    dmesg("PW=%s"%ap_password)
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

wlan = wifi_connect()

gc.collect()
dmesg('* %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )

#machine.sleep(3000)
#setup_wanip()

