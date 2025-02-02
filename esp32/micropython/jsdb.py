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
    exec( ("jsdb['%s']="%objroot) + open('web/%s.json'%objroot).read() + '\n' , globals() )
  return (objroot,objacc)

def jsdb_get_value(k):
  (objroot,objacc) = jsdb_parse_key(k)
  L={}
  exec('R=jsdb'+objacc,globals(),L)
  return L['R']

def jsdb_set_value(k,v,sync=False):
  (objroot,objacc) = jsdb_parse_key(k)
  L={'V':None}
  exec('V=%s'%v,globals(),L)
  exec('jsdb'+objacc+'=V',globals(),L)
  if sync:
    f=open('web/%s.json'%objroot,'w')
    f.write( str(globals()['jsdb'][objroot])+'\n')
    f.close()
  return True

def http_jsdb_query(conn,fname,mimetype,params):
  fmt='js'
  if 'out' in params:
    fmt=params.pop('out')
  sync=False
  if 'sync' in params:
    sync=(params.pop('sync')=='True')
  repfmt=""
  if fmt=='js':
    http_reply_header(conn,"text/javascript",False)
    repfmt="let %s=%s;\n"
  elif fmt=='json':
    http_reply_header(conn,"application/json",False)
    repfmt="{'%s':%s}\n"
  else:
    dmesg('EFMT:%s'%fmt)
    return False
  jsdb_retcode=0
  for (k,v) in params.items():
    if v=='=':
      conn.sendall(repfmt%(k,jsdb_get_value(k)))
      jsdb_retcode+=100
    else:
      jsdb_set_value(k,v,sync)
      jsdb_retcode+=1
  if fmt=='js':
    conn.sendall('let jsdb_retcode=%d;\n'%jsdb_retcode)
  return True

