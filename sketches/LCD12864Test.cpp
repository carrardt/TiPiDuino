/* 
 
 // # Update the library and sketch to compatible with IDE V1.0 and earlier
 
 // # Description:
 // # The sketch for using LCD12864 in parallel mode
 
 LCD  Arduino
 RS = 17; Analog Pin3
 RW = 16; Analog Pin2
 EN = 18; Analog Pin4
 D0  = 8; 
 D1  = 9;
 D2  = 10; 
 D3  = 11; 
 D4  = 4;
 D5  = 5; 
 D6  = 6; 
 D7  = 7;
 PIN15 PSB = 5V;
 */

#include "AvrTL/AvrTLSignal.h"
#include "LCD12864/LCD12864.h"
#include "BasicIO/PrintStream.h"
LCD12864 LCDA;
PrintStream cout;

void setup()
{
  LCDA.Initialise(); // INIT SCREEN
  cout.begin( & LCDA );
  avrtl::DelayMicroseconds(100000);
  LCDA.Clear();
  avrtl::DelayMicroseconds(100000);
}

static int count = 0;
void loop()
{
//  LCDA.Clear();//Clear screen
  cout<<"Count = "<<count<<endl;
  ++ count;
  avrtl::DelayMicroseconds(1000000);
}

