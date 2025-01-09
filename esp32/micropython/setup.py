# install files

import os

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

f=open('web/index.html','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/web/led_on_off.html').content.decode('utf8'))
f.close()

f=open('web/server.js','w')
f.write(requests.get('https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/web/server.js').content.decode('utf8'))
f.close()

