# install files

import os
import requests

def download_py(fname):
  bname=fname.split('/')[-1]
  print("Download %s ..."%bname)
  f=open(bname,'w')
  f.write(requests.get("https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/%s"%fname).content.decode('utf8'))
  f.close()

for fname in ['boot.py','console.py','http_server.py','main.py','tools.py','wanip.py','wifi.py','jsdb.py','pcd8544/pcd8544.py']:
  download_py(fname)



os.mkdir('web')

def download_web(webfile):
  print("Download %s ..."%webfile)
  f=open("web/%s"%webfile,'w')
  f.write(requests.get("https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/web/%s"%webfile).content.decode('utf8'))
  f.close()

for webfile in ['index.html','setup-wifi.html','setup-wan.html','TiHomeManager.html','TiHomeManager.js']:
  download_web(webfile)



os.mkdir('db')

def download_db(dbfile):
  print("Download %s ..."%dbfile)
  f=open("db/%s"%dbfile,'w')
  f.write(requests.get("https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/db/%s"%webfile).content.decode('utf8'))
  f.close()

for dbfile in ['home.json','wifi.json','wanip.json']:
  download_db(dbfile)

