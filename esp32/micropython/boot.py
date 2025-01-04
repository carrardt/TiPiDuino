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

import requests
try:
  WANIP = [ s.strip() for s in open('wanip.txt').readlines() ]
  print('last configured wan IP for %s was %s' % (WANIP[0],WANIP[1]) )
  curwanip = requests.get('https://api.ipify.org').content.decode('utf8')
  print('current wan IP is %s' % curwanip )
  if curwanip==WANIP[1]:
    print('wan IP is unchanged, no update')
  else:
    print('update wan IP to %s' % curwanip)
    WANIP[1]=curwanip
    print(requests.get(WANIP[2]).content.decode('utf8'))
    f=open('wanip.txt','w')
    f.write('\n'.join(WANIP))
    f.close()
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

