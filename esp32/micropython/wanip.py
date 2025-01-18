import requests

def setup_wanip():
  dmesg("wan init...")
  try:
    WANIP = [ s.strip() for s in open('config/wanip.txt').readlines() ]
    curwanip=requests.get("https://api.ipify.org").content.decode("utf8")
    if WANIP[1]!=curwanip:
      WANIP[1]=curwanip
      requests.get(WANIP[2]).content.decode('utf8')
      f=open('config/wanip.txt','w')
      f.write('\n'.join(WANIP))
      f.close()
    dmesg(curwanip)
  except OSError as e:
    dmesg("WAN failed")

