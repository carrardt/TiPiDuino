import requests

def setup_wanip():
  dmesg("wan init...")
  WANIP = jsdb_get_value('wanip')
  if WANIP['url'] != '':
    curwanip=requests.get("https://api.ipify.org").content.decode("utf8")
    if WANIP['ip']!=curwanip:
      WANIP['ip']=curwanip
      requests.get(WANIP['url']).content.decode('utf8')
      jsdb_set_value('wanip',WANIP,True)
    dmesg(curwanip)
  else:
    dmesg('no wan')
