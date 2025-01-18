#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/main.py
#<main.py

#def http_file_read(conn,fname,parameters):
#  try:
#    f=open('web/'+fname)
#    conn.sendall(f.read())
#  except:
#    conn.sendall("<html><head><title>page not found</title></head><body>page '%s' not found</body></html>" % fname)  

def http_file_read(conn,fname,parameters):
  conn.sendall("<html><head><title>mini debugger</title></head><body>requested '%s'<br>parameters<br>%s</body></html>" % (fname,str(parameters)) )

def setup_wifi_config(conn,fname,parameters):
  print("Setup wifi config: parameters =",parameters)
  conn.sendall("<html><head><title>Wifi setup success</title></head><body>WIFI setup complete</body></html>")

import socket
http_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
http_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
http_sock.bind(('', 80))
http_sock.listen(5)

def mini_http_server():
  while True:
    conn, addr = http_sock.accept()
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
      if fname=="": fname="index.html"
      print("file=%s" % fname)
      print(parameters)
      mimetype="text"
      server_func = http_file_read
      fext=fname.split(".")[-1]
      if fext=="html":
        mimetype="text/html"
      elif fext=="js":
        mimetype="text/javascript"
      elif fext=="json":
        mimetype="application/json"
      elif fext=="py":
        mimetype="text/html"
        server_func=globals()[fname[:-3]]
        print("dyn page",fname[:-3],server_func)
      print("mime=%s"%mimetype)
      conn.send('HTTP/1.1 200 OK\n')
      conn.send('Content-Type: %s\n'%mimetype)
      conn.send('Cache-Control: public, max-age=604800\n')
      conn.send('Connection: close\n\n')
      server_func(conn,fname,parameters)
    conn.close()


