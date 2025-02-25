import machine

# console output to PCD8544 LCD screen
import pcd8544
# SPI #1 means pins 13 (MOSI) and 14 (SCLK) for ESP32 WROOM 32
# 3 other pins are CS, DC and RST respectively
lcd = pcd8544.PCD8544_BM8x5(machine.SPI(1), machine.Pin(12), machine.Pin(15), machine.Pin(4))
lcd.reset()
lcd.init()
lcd.contrast(0x3f, pcd8544.BIAS_1_48, pcd8544.TEMP_COEFF_0)
lcd.clear()

def dmesg(s,mend="\n"):
  lcd.println(s)

def clear():
  lcd.clear()

import qrcodegen

def show_qr_code(txt,EccLvl=qrcodegen.QRCode.Ecc_LOW):
  SCALE = 1
  qr = qrcodegen.QRCode()
  qr.setEcc(EccLvl)
  qr.fromText(txt)
  sz = qr.size()
  if sz <= 24:
    SCALE = 2
  sz = sz * SCALE
  startx=(84-sz)//2
  startcol=0
  lcd.clear()
  for col in range((sz+7)//8):
      lcd.position(startx,startcol+col)
      for i in range(sz):
          c=0
          for b in range(8):
              c = c*2 + int( qr.module( i//SCALE , (col*8+7-b)//SCALE ) )
          lcd.data( bytearray([c]) )
  qr = None
  gc.collect()

