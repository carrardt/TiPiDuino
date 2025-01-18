import machine

# console output to PCD8544 LCD screen
import pcd8544
lcd = pcd8544.PCD8544_BM8x5(machine.SPI(1), machine.Pin(12), machine.Pin(15), machine.Pin(4))
lcd.reset()
lcd.init()
lcd.contrast(0x3f, pcd8544.BIAS_1_48, pcd8544.TEMP_COEFF_0)
lcd.clear()

def dmesg(s,mend="\n"):
  print(s,end=mend)
  lcd.println(s)

def clear():
  lcd.fill(0)
  lcd.show()

