def jsdb_init():
  if not 'jsdb' in globals():
    globals()['jsdb'] = {}

def jsdb_parse_key(k):
  objpath = k.split('.')
  objroot = objpath[0]
  objacc = ""
  for o in objpath:
    a = o.split('[')
    objacc += "[".join(["['"+a[0]+"']"]+a[1:])
  return (objroot,objacc)

def jsdb_get_value(k):
  jsdb_init()
  if not k in globals()['jsdb']
    exec( ("jsdb['%s']="%k) + open('web/%s.json'%k).read() + '\n' , globals() )
  return globals()['jsdb'][k]

def jsdb_set_value(k,v):
  jsdb_init()
  jsdb_get_value(k)
  globals()['jsdb'][k] = v
  return True

def db_query(conn,fname,mimetype,params):
  fmt=params.pop('output')
  if fmt=='js':
    http_reply_header(conn,"application/json",False)
  else:
    dmesg('EFMT:%s'%fmt)
    return False
  for (k,v) in params.items():
    if v=='=':
      conn.sendall('let %s = %s ;\n'%(k,jsdb_get_value(k)))
    else:
      jsdb_set_value(k,v)
  return True

