import socket

def create_http_socket():
  http_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  http_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  http_sock.bind(('', 80))
  http_sock.listen(5)
  return http_sock

def http_reply_header(conn,mimetype,cache=True):
  conn.send('HTTP/1.1 200 OK\n')
  conn.send('Content-Type: %s\n'%mimetype)
  if cache:
    conn.send('Cache-Control: public, max-age=604800\n')
  else:
    conn.send('Cache-Control: public, no-cache\n')
  conn.send('Connection: close\n\n')  

def http_file_read(conn,fname,mimetype,parameters):
  http_reply_header(conn,mimetype)
  try:
    f=open('web/'+fname)
    conn.sendall(f.read())
  except:
    conn.sendall("<html><head><title>page not found</title></head><body>page '%s' not found</body></html>" % fname)  

def setup_wifi_config(conn,fname,mimetype,params):
  http_reply_header(conn,mimetype)
  if not 'ssid' in params.keys():
    dmesg('missing ssid')
    conn.sendall("<html><head><title>Wifi setup failed</title></head><body>WIFI setup failed</body></html>")
    return False
  ssid=params['ssid']
  seckey=''
  if 'key' in params.keys():
    seckey=params['key']
  host='TiHome'
  if 'host' in params.keys():
    host=params['host']
  f=open('config/wifi.txt','w')
  f.write("%s\n%s\n%s\n"%(ssid,seckey,host))
  f.close()
  conn.sendall("<html><head><title>Wifi setup success</title></head><body>WIFI setup complete<br>SSID=%s<br>key=%s<br>host=%s</body></html>"%(ssid,seckey,host) )
  dmesg("Wifi setup")
  return True


def jsdb_init():
  if not 'jsdb' in globals():
    globals()['jsdb'] = {}

def jsdb_get_value(k):
  jsdb_init()
  if not k in globals()['jsdb']
    globals()['jsdb'][k] = open('web/%s.json'%k).read()
  return globals()['jsdb'][k]

def jsdb_set_value(k,v):
  jsdb_init()
  jsdb_get_value(k)
  globals()['jsdb'][k] = v
  return True

def db_query(conn,fname,mimetype,params):
  fmt=params.pop('output')
  if fmt=='js':
    http_reply_header(conn,"application/json")
  else:
    dmesg('EFMT:%s'%fmt)
    return False
  for (k,v) in params.items():
    if v=='=':
      conn.sendall('let %s = %s ;\n'%(k,jsdb_get_value(k)))
    else:
      
  return True

def http_server(http_sock):
  while True:
    conn, addr = http_sock.accept()
    print('<%s' % str(addr[0]))
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
          kvl=kv.split("=")
          parameters[kvl[0]] = kvl[1:]
      if fname=="": fname="index.html"
      mimetype="text"
      server_func = http_file_read
      fext=fname.split(".")[-1]
      mimetype="text/html"
      if fext=="js":
        mimetype="text/javascript"
      elif fext=="json":
        mimetype="application/json"
      elif fext=="py":
        server_func=globals()[fname[:-3]]
      server_func(conn,fname,mimetype,parameters)
    conn.close()

