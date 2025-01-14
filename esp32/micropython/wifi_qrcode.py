# The information is encoded in a string format, usually structured like this:
# WIFI:T:<encryption_type>;S:<SSID>;P:<password>;H:<hidden>;
# T: Type of encryption (e.g., WPA, WPA2).
# S: SSID of the network.
# P: Password for the network.
# H: Indicates if the network is hidden (optional).
# exemples from Wikipedia :
# Common format: WIFI:S:<SSID>;T:<WEP|WPA|nopass>;P:<PASSWORD>;H:<true|false|blank>;;
# Sample: WIFI:S:MySSID;T:WPA;P:MyPassW0rd;;

import network
import uQR

ap_ssid="MyWifiAP"
ap_password="123456789"

ap_if = network.WLAN(network.AP_IF)
ap_if.active(True)
ap_if.config(ssid=SSID,password=ap_password)

ap_qr_data = "WIFI:S:%s;T:WPA;P:%s;;" % (ap_ssid,ap_password)
qr = uQR.QRCode()
qr.add_data(ap_qr_data)
mat = qr.get_matrix()
qr = None
ap_qr_data = None

# framebuf display ...

