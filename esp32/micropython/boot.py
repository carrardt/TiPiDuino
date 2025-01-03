#<boot.py

try:
  import usocket as socket
except:
  import socket
import network
import esp
esp.osdebug(None)
import gc
gc.collect()

WIFICON = [ s.strip() for s in open('wifi.txt').readlines() ]
ssid = WIFICON[0]
password = WIFICON[1]
sta_if = network.WLAN(network.STA_IF)
sta_if.config(dhcp_hostname="TiHomeManager")
sta_if.active(True)
print(sta_if.scan())
print('Connecting to %s with key %s' % (ssid,password) )
sta_if.connect(ssid,password)

import machine
machine.sleep(2000)

if sta_if.isconnected():
  print('Connection successful')
  print(sta_if.ifconfig())
else:
  print('Connection failed')

try:
  WANIP = [ s.strip() for s in open('wanip.txt').readlines() ]
  print('Current wan IP for %s is %s' % (WANIP[0],WANIP[1]) )
except:
  print('no dynamic wan IP configuration found')

gc.collect()

def mkfile():
  l = input()
  if l[0:2] != "#<":
    print("Bad mkfile input format")
    return
  f=open(l[2:],"w")
  while True:
    try:
      l=input()
    except EOFError:
      break
    f.write(l+"\n")
  f.close()

def chkfile(fname):
  ln=1
  for l in open(fname).readlines():
    print("[% 3d] %s" % (ln,l.strip()) )
    ln=ln+1

