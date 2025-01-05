#$https://raw.githubusercontent.com/carrardt/TiPiDuino/refs/heads/main/esp32/micropython/main.py
#<main.py

print("-= Relay controller web interface =-")

led=machine.Pin(2,machine.Pin.OUT)

for i in range(5):
  machine.sleep(500)
  led.value(1)
  machine.sleep(500)
  led.value(0)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', 80))
s.listen(5)

while True:
  conn, addr = s.accept()
  print('Got a connection from %s' % str(addr))
  request = conn.recv(1024)
  request = str(request)
  print('Content = %s' % request)
  led_on = request.find('/?led=on')
  led_off = request.find('/?led=off')
  if led_on == 6:
    print('LED ON')
    led.value(1)
  if led_off == 6:
    print('LED OFF')
    led.value(0)
  if led.value() == 1:
    gpio_state="ON"
  else:
    gpio_state="OFF"
  conn.send('HTTP/1.1 200 OK\n')
  conn.send('Content-Type: text/html\n')
  conn.send('Connection: close\n\n')
  conn.sendall( open('index.html').read() % gpio_state )
  conn.close()
