def jsdb_parse_key(k):
  objpath = k.split('.')
  objroot = objpath[0]
  objacc = ""
  for o in objpath:
    a = o.split('[')
    objacc += "[".join(["['"+a[0]+"']"]+a[1:])
  if not 'jsdb' in globals():
    globals()['jsdb'] = {}
  if not objroot in globals()['jsdb']:
    exec( ("jsdb['%s']="%k) + open('web/%s.json'%k).read() + '\n' , globals() )
  return (objroot,objacc)

def jsdb_get_value(k):
  (objroot,objacc) = jsdb_parse_key(k)
  L={}
  exec('R=jsdb'+objacc,globals(),L)
  return L['R']

def jsdb_set_value(k,v):
  (objroot,objacc) = jsdb_parse_key(k)
  L={'V':v}
  exec('jsdb'+objacc+'=V',globals(),L)
  return True

def db_query(conn,fname,mimetype,params):
  fmt=params.pop('output')
  if fmt=='js':
    http_reply_header(conn,"application/json",False)
  else:
    dmesg('EFMT:%s'%fmt)
    conn.sendall('EFMT:%s\n'%fmt)
    return False
  for (k,v) in params.items():
    if v=='=':
      conn.sendall('let %s = %s ;\n'%(k,jsdb_get_value(k)))
    else:
      jsdb_set_value(k,v)
  conn.sendall('let jsdb_errcode = 0;\n')
  return True

