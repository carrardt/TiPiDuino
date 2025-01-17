import requests

def mkfile():
  l = input()
  if l[0:2] == "#<":
    fname=l[2:]
    dmesg("W %s"%fname)
    f=open(l[2:],"w")
    while True:
      try:
        l=input()
      except EOFError:
        break
      f.write(l+"\n")
    f.close()
  elif l[0:2] == "#$":
    url=l[2:]
    fname=url[url.rfind('/')+1:]
    dmesg("w %s"%fname)
    f=open(fname,"w")
    f.write(requests.get(url).content.decode('utf8'))
    f.close()

def chkfile(fname):
  ln=1
  for l in open(fname).readlines():
    dmesg("[% 3d] %s" % (ln,l) , end='')
    ln=ln+1


