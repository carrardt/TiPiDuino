#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <KeyPad/keypad.h>

KeyPad::KeyPad(int c0, int c1, int c2, int c3, int r0, int r1, int r2, int r3)
{
  m_col_pin[0] = c0;
  m_col_pin[1] = c1;
  m_col_pin[2] = c2;
  m_col_pin[3] = c3;
  m_row_pin[0] = r0;
  m_row_pin[1] = r1;
  m_row_pin[2] = r2;
  m_row_pin[3] = r3;
}

int KeyPad::keyToDigit(int k)
{
  switch(k)
  {
	  case K_0 : return 0;
	  case K_1 : return 1;
	  case K_2 : return 2;
	  case K_3 : return 3;
	  case K_4 : return 4;
	  case K_5 : return 5;
	  case K_6 : return 6;
	  case K_7 : return 7;
	  case K_8 : return 8;
	  case K_9 : return 9;
	  default : return -1;
  }
}

bool KeyPad::isDigit(int k)
{
  return keyToDigit(k)>=0;
}

int KeyPad::scanKeyPad()
{
  int k = -1;
  for(int i=0;i<4 && k==-1;i++)
  {
	  pinMode( m_col_pin[i] , OUTPUT );
	  digitalWrite( m_col_pin[i] , LOW );
	  for(int j=0;j<4 && k==-1;j++)
	  {
		  if( ! digitalRead(m_row_pin[j]) )
		  {
			  k = i*4+j;
		  }
	  }
	  pinMode( m_col_pin[i] , INPUT_PULLUP );
  }
  return k;
}

