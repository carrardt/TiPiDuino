import pcd8544
import machine
spi = machine.SPI(1) # CLK = Pin(14) , DIN (aka MOSI) = Pin(13)
cs = machine.Pin(12) # to CE
dc = machine.Pin(15) # to DC
rst = machine.Pin(4) # to RST
#bl = machine.Pin(12, machine.Pin.OUT, value=1) # to BL

#lcd = pcd8544.PCD8544(spi, cs, dc, rst)
lcd = pcd8544.PCD8544_FRAMEBUF(spi, cs, dc, rst)
lcd.reset()
lcd.init()
lcd.contrast(0x3f, pcd8544.BIAS_1_48, pcd8544.TEMP_COEFF_0) # displays best on ESP32 WROOM with VCC on 3v3
lcd.clear()

lcd_line = 0
def lcd_print_line(l):
  lcd_line=globals()['lcd_line']
  while lcd_line>5:
    for i in range(8):
      lcd.scroll(0,-1)
      lcd.show()
    lcd_line = lcd_line - 1
  lcd.text(l,0,lcd_line*8,1)
  lcd.show()
  lcd_line = lcd_line + 1

