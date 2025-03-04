include('http_server.py')

dmesg("HTTP ready")
time.sleep(3)

SRV_URL = "http://%s/" % wlan.ifconfig()[0]
show_qr_code(SRV_URL)
lcd.text(0,5,SRV_URL)

http_sock = create_http_socket()
http_server( http_sock )

