#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/boot.py

import esp
esp.osdebug(None)
import machine
dmesg_out = machine.UART(2,19200)
dmesg_out.init(19200,bits=8,parity=None,stop=1)

def dmesg(s,end="\n"):
  print(s+end)
  dmesg_out.write(s+end)
  machine.sleep(250)

def clear():
  dmesg_out.write("&~C\n")
  machine.sleep(500)

clear()
import gc
gc.enable()

import network
import requests
sta_if = network.WLAN(network.STA_IF)
gc.collect()
dmesg('* %d/%d' % ( gc.mem_alloc()/1024 , gc.mem_free()/1024 ) )

def mkfile():
  l = input()
  if l[0:2] == "#<":
    fname=l[2:]
    dmesg('Write %s from user input (end input with CTRL-D)' % fname)
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
    dmesg('Write %s from URL %s' % (fname,url) )
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
  dmesg('-- Wifi connection --')
  WIFICON = [ s.strip() for s in open('wifi.txt').readlines() ]
  ssid = WIFICON[0]
  password = WIFICON[1]
  hostname = WIFICON[2]
  sta_if.active(True)
  dmesg(sta_if.scan())
  dmesg('Configure hostname to %s' % hostname)
  sta_if.config(dhcp_hostname=hostname)
  dmesg('Connecting to %s with key %s' % (ssid,password) )
  sta_if.connect(ssid,password)
  machine.sleep(5000)
  dmesg(sta_if.ifconfig())
  return sta_if.isconnected()

# Check / update wan IP address
def setup_wanip():
  dmesg('-- Setup wan interface --')
  try:
    WANIP = [ s.strip() for s in open('wanip.txt').readlines() ]
    dmesg('last configured wan IP for %s was %s' % (WANIP[0],WANIP[1]) )
    curwanip = requests.get('https://api.ipify.org').content.decode('utf8')
    dmesg('current wan IP is %s' % curwanip )
    if curwanip==WANIP[1]:
      dmesg('wan IP is unchanged, no update')
    else:
      dmesg('update wan IP to %s' % curwanip)
      WANIP[1]=curwanip
      dmesg(requests.get(WANIP[2]).content.decode('utf8'))
      f=open('wanip.txt','w')
      f.write('\n'.join(WANIP))
      f.close()
  except:
    dmesg('no dynamic wan IP configuration found')

gc.collect()
dmesg('* %d/%d' % (gc.mem_alloc()/1024,gc.mem_free()/1024) )

wifi_connect()
machine.sleep(3000)
setup_wanip()
