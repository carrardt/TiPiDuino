#ifndef _Template_LiquidCrystal_h
#define _Template_LiquidCrystal_h

#include <inttypes.h>
#include "AvrTL.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

// handles Read/Write mode with optional rw pin
// if no pin is connected (pin = -1) nothing happens

#include <ByteStream.h>

template<int _rs, int _en, int _d0, int _d1, int _d2, int _d3,int _cols=16, int _lines=2, int _dotSize=LCD_5x8DOTS>
struct LCD : public ByteStream
{
	static constexpr int cols = _cols;
	static constexpr int lines = _lines;
	static constexpr int dotSize = _dotSize;
	avrtl::AvrPin<_rs> rs;
	avrtl::AvrPin<_en> en;
	avrtl::AvrPin<_d0> d0;
	avrtl::AvrPin<_d1> d1;
	avrtl::AvrPin<_d2> d2;
	avrtl::AvrPin<_d3> d3;

	void pulseEnable() 
	{
	  en = 0;
	  avrtl::DelayMicroseconds(1);    
	  en = 1;
	  avrtl::DelayMicroseconds(1);    // enable pulse must be >450ns
	  en = 0;
	  avrtl::DelayMicroseconds(100);   // commands need > 37us to settle
	}

	void write4bits(uint8_t value) 
	{
		d0 = value & 1; value >>= 1;
		d1 = value & 1; value >>= 1;
		d2 = value & 1; value >>= 1;
		d3 = value & 1;
		pulseEnable();
	}
	
	void write(uint8_t value) 
	{
		write4bits(value>>4);
		write4bits(value);
	}

	void command(uint8_t value) 
	{
		rs = 0;
		write(value);
	}

	void init() 
	{
		rs.SetOutput();
		rs = 0;
		en.SetOutput();
		en = 0;
		d0.SetOutput();
		d1.SetOutput();
		d2.SetOutput();
		d3.SetOutput();
		write4bits(0x03);
		avrtl::DelayMicroseconds(4500);
		write4bits(0x03);
		avrtl::DelayMicroseconds(4500);
		write4bits(0x03);
		avrtl::DelayMicroseconds(150);
		write4bits(0x02);
		command( LCD_FUNCTIONSET | LCD_4BITMODE | ((lines<=1)?LCD_1LINE:LCD_2LINE) | dotSize );
	}
	
	void sendByte(uint8_t value) 
	{
		rs = 1;
		write(value);
	}	
	
	LCD()
		: m_writeMode(true)
		, m_lineFeed(false)
		, m_displayControlFlags(0)
		, m_displayModeFlags(0)
		, m_bufSize(0)
	{
	}

	void begin()
	{
		init();
		m_writeMode = true;
		setDisplayFlags( LCD_DISPLAYON );
		clear();
  		setDisplayModeFlags( LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
	}

	void clear()
	{
	  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	  avrtl::DelayMicroseconds(2000);  // this command takes a long time!
	}
	
	void home()
	{
	  command(LCD_RETURNHOME);  // set cursor position to zero
	  avrtl::DelayMicroseconds(2000);  // this command takes a long time!
	}
	
	void setCursor(uint8_t col, uint8_t row)
	{
	  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	  if ( row >= lines ) {
	    row = lines-1;    // we count rows starting w/0
	  }
 	 command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
	}

	void setDisplayFlags(uint8_t flags)
	{
		if( (m_displayControlFlags&flags) == flags ) return;
		m_displayControlFlags |= flags;
	  	command(LCD_DISPLAYCONTROL | m_displayControlFlags);
	}
	void clearDisplayFlags(uint8_t flags)
	{
		if( (m_displayControlFlags&flags) == 0 ) return;
		m_displayControlFlags &= ~flags;
	  	command(LCD_DISPLAYCONTROL | m_displayControlFlags);
	}

	void noDisplay() { clearDisplayFlags(LCD_DISPLAYON); }
	void display() { setDisplayFlags(LCD_DISPLAYON); }
	void noCursor() { clearDisplayFlags(LCD_CURSORON); }
	void cursor() { setDisplayFlags(LCD_CURSORON); }
	void noBlink() { clearDisplayFlags(LCD_BLINKON); }
	void blink() { setDisplayFlags(LCD_BLINKON); }

	void scrollDisplayLeft() const
	{
	  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
	}
	void scrollDisplayRight() const
	{
	  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
	}
	
	void setDisplayModeFlags(uint8_t flags)
	{
	  if( (m_displayModeFlags&flags) == flags ) return;
	  m_displayModeFlags |= flags;
	  command(LCD_ENTRYMODESET | m_displayModeFlags);
	}
	void clearDisplayModeFlags(uint8_t flags)
	{
	  if( (m_displayModeFlags&flags) == 0 ) return;
	  m_displayModeFlags &= ~flags;
	  command(LCD_ENTRYMODESET | m_displayModeFlags);
	}
	
	void leftToRight() { setDisplayModeFlags(LCD_ENTRYLEFT); }
	void rightToLeft() { clearDisplayModeFlags(LCD_ENTRYLEFT); }
	void autoscroll() { setDisplayModeFlags(LCD_ENTRYSHIFTINCREMENT); }
	void noAutoscroll() { clearDisplayModeFlags(LCD_ENTRYSHIFTINCREMENT); }

	void createChar(uint8_t location, uint8_t charmap[])
	{
	  location &= 0x7; // we only have 8 locations 0-7
	  command(LCD_SETCGRAMADDR | (location << 3));
	  for (int i=0; i<8; i++) {
	    sendByte(charmap[i]);
	  }
	}

	virtual bool writeChar(char value)
	{
		if(m_lineFeed)
		{
			if( lines == 1 )
			{
				clear();
				setCursor(0,0);
			}
			else
			{
				int i;
				setCursor(0,0);
				for(i=0;i<m_bufSize;i++) sendByte(m_buffer[i]);
				for(;i<cols;i++) sendByte(' ');
				setCursor(0,1);
				for(i=0;i<cols;i++) sendByte(' ');
				setCursor(0,1);
			}
			m_bufSize = 0;
			m_lineFeed = false;
		}
		
		if( value == '\n' )
		{
			m_lineFeed = true;
		}
		else if( value == '\r' )
		{
			// ignore it
		}
		else
		{
			if( m_bufSize < cols )
			{
				m_buffer[ m_bufSize++ ] = value;
			}
			sendByte(value);
		}
		return true;
	}

private:
	bool m_writeMode;
	bool m_lineFeed;
	uint8_t m_displayControlFlags;
	uint8_t m_displayModeFlags;
	uint8_t m_bufSize;
	uint8_t m_buffer[cols];
	
};

#endif

