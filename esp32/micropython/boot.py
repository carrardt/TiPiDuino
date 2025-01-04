#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/boot.py
#<boot.py

print('-- Import modules --')
import socket
import network
import esp
esp.osdebug(None)
import machine
import requests
import gc
gc.collect()

print('-- Define functions --')
def mkfile():
  l = input()
  if l[0:2] == "#<":
    fname=l[2:]
    print('Write %s from user input (end input with CTRL-D)' % fname)
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
    print('Write %s from URL %s' % (fname,url) )
    f=open(fname,"w")
    f.write(requests.get(url).content.decode('utf8'))
    f.close()

def chkfile(fname):
  ln=1
  for l in open(fname).readlines():
    print("[% 3d] %s" % (ln,l) , end='')
    ln=ln+1

# Wifi connection
print('-- Wifi connection --')
WIFICON = [ s.strip() for s in open('wifi.txt').readlines() ]
ssid = WIFICON[0]
password = WIFICON[1]
hostname = WIFICON[2]
sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
print(sta_if.scan())
print('Configure hostname to %s' % hostname)
sta_if.config(dhcp_hostname=hostname)
print('Connecting to %s with key %s' % (ssid,password) )
sta_if.connect(ssid,password)

# wait for connection established
conwait=0
print('Waiting for connection ',end="")
while not sta_if.isconnected():
  machine.sleep(1000)
  conwait=conwait+1
  print('.',end='')

print('\nConnection successful')
print(sta_if.ifconfig())

# Check / update wan IP address
print('-- Setup wan interface --')
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

