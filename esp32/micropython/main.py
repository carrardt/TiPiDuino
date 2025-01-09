#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/main.py
#<main.py

print("-= Relay controller web interface =-")

#led=machine.Pin(2,machine.Pin.OUT)
#for i in range(5):
#  machine.sleep(500)
#  led.value(1)
#  machine.sleep(500)
#  led.value(0)

#import json
#home=json.load(open('home.json'))
# print( json.dumps(home) )
#home.heatzone[0].state="E"
#f=open('home.json','w')
#json.dump(home,f)
#f.close()

import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(('', 80))
s.listen(5)

#gpio_state="OFF"

def mini_http_server():
  while True:
    conn, addr = s.accept()
    print('CLI %s' % str(addr[0]))
    request = conn.recv(4096).decode('utf8').split('\n')[0].split(' ')
    print(request)
    if request[0]=="GET":
      freq=request[1].split("?")
      fname=freq[0]
      if fname[0]=="/":
        fname = fname[1:]
      parameters={}
      if len(freq)>1:
        for kv in freq[1].split("&"):
          (k,v) = kv.split("=")
          parameters[k] = v        
      print("file=%s" % fname)
      print(parameters)
      fext=fname.split(".")[-1]
      mimetype="text"
      if fext=="html":
        mimetype="text/html"
      elif fext=="js":
        mimetype="text/javascript"
      elif fext=="json":
        mimetype="application/json"
      print("mime=%s"%mimetype)
      conn.send('HTTP/1.1 200 OK\n')
      conn.send('Content-Type: %s\n'%mimetype)
      conn.send('Cache-Control: public, max-age=604800\n')
      conn.send('Connection: close\n\n')
      try:
        f=open('web/'+fname)
        conn.sendall(f.read())
      except:
        conn.sendall("<html><head><title>page not found</title></head><body>page '%s' not found</body></html>" % fname)
    conn.close()


