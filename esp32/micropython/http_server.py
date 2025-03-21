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
          if len(kvl)==1: parameters[kvl[0]] = ''
          elif len(kvl)>=2: parameters[kvl[0]] = '='.join(kvl[1:])
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

