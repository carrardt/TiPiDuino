# install files

import os
import requests

os.mkdir('config')

f=open('config/wifi.txt','w')
f.write("SSID\nPassWord\nTiHomeManager")
f.close()

f=open('config/wanip.txt','w')
f.write("myserver.org\n10.0.0.1\nhttp://UpdateWanURL")
f.close()

f=open('boot.py','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/boot.py').content.decode('utf8'))
f.close()

f=open('main.py','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/main.py').content.decode('utf8'))
f.close()

os.mkdir('web')

f=open('web/TiHomeManager.html','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/web/TiHomeManager.html').content.decode('utf8'))
f.close()

f=open('web/TiHomeManager.js','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/web/TiHomeManager.js').content.decode('utf8'))
f.close()

