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

#include "AvrTL/AvrTLPin.h"
#include "AvrTL/AvrTLSignal.h"
#include "LCD12864/LCD12864.h"
#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"
#include "SoftSerial/SoftSerial.h"

static auto rx = avrtl::StaticPin<3>();
static auto tx = avrtl::StaticPin<2>();
static auto serialIO = make_softserial<57600>(rx,tx);
static LCD12864 LCDA;
static PrintStream cout;
//static InputStream cin;

void setup()
{
  LCDA.Initialise(); // INIT SCREEN
  cout.begin( & LCDA );
  avrtl::DelayMicroseconds(100000);
  LCDA.Clear();
  avrtl::DelayMicroseconds(100000);
  cout<<"LCD12864Console"<<endl;
  serialIO.begin();
}

void loop()
{
  char tmp[16];
  int i=0;
  while(i<15 && (tmp[i]=serialIO.readByte()) != '\n') ++i;
  tmp[i] = '\0';
  cout<<tmp<<endl;
}

