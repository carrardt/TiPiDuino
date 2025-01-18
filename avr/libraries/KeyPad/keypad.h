#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

class KeyPad
{
public:
  enum KeyPosition { K_1, K_2, K_3, K_Plus, K_4, K_5, K_6, K_Minus, K_7, K_8, K_9, K_Mult, K_Mode, K_0, K_Equal, K_Div };

  KeyPad(int c0, int c1, int c2, int c3, int r0, int r1, int r2, int r3);
  int keyToDigit(int k);
  bool isDigit(int k)
  int scanKeyPad();

private:
  uint8_t m_col_pin[4];
  uint8_t m_row_pin[4];
};

