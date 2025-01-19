include('http_server.py')
dmesg('http ready')

http_sock = create_http_socket()
http_server( http_sock )

