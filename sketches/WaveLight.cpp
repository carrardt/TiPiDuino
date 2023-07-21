#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"
#include "BasicIO/PrintStream.h"

#include <PCD8544.h>
#include "WS2811/ws2811.h"
#include "DS1302/Ds1302.h"
#include <avr/interrupt.h>

// PCD8544 pins : SCLK, SDIN, DC, RST, SCE (connect to ground)
#define LCD_PINS     2,    3,  4,   6, PCD8544_UNASSIGNED
// WS2811 pin = 8 a.k.a PB0
#define DATA_FEEDBACK_PIN 5 // a.k.a T1 pin, external clock signal for timer 1

static PCD8544 lcd( LCD_PINS );
static ByteStreamAdapter<PCD8544> lcdIO;
static PrintStream cout;

#define MAX_TRACK_LIGHTS 1024

// A custom glyph (a smiley)...
// static const uint8_t glyph[] = { 0b00010000, 0b00110100, 0b00110000, 0b00110100, 0b00010000 };
struct TrackLights
{
  uint16_t nb_lights = 100;
  uint16_t length = 4000; // in decimeters
  uint16_t run_distance = 0; // in decimeters
  uint16_t run_time = 6*60*10; // in 1/10 seconds (default is a 6 minutes run)
};

static constexpr uint8_t MAX_PARTNERS = 4;
static const uint8_t PROGMEM partner_color[MAX_PARTNERS*3] = { 0x00,0xFF,0x00 , 0x7F,0x3F,0x00 , 0xFF,0x00,0x00 , 0xFF,0x00,0xFF };

static Adafruit_NeoPixel strip;
static TrackLights track_lights = {};
static avrtl::AvrTimer<avrtl::AvrTimer0HW,256> delayTimer;
static avrtl::AvrTimer<avrtl::AvrTimer1HW,avrtl::AvrTimer1HW::ExternalClockRising> loopBackSignalCounter;
static avrtl::StaticPin<5> loopBackPin;

#define RTC_PIN_CLK 7
#define RTC_PIN_DAT 9
#define RTC_PIN_RST 10

// DS1302 RTC instance
static Ds1302 rtc(RTC_PIN_RST, RTC_PIN_CLK, RTC_PIN_DAT);

void setup()
{
	cli();

  lcdIO.m_rawIO.setPins( LCD_PINS );
  
  // PCD8544-compatible displays may have a different resolution...
  lcdIO.m_rawIO.begin(84, 48);

  lcdIO.m_rawIO.setContrast(63);
  cout.begin(&lcdIO);
  lcdIO.m_rawIO.setCursor(0, 0);

  delayTimer.start();
  loopBackSignalCounter.start();
  loopBackPin.SetInput();
  loopBackSignalCounter.resetCounter();

  strip.updateLength( MAX_TRACK_LIGHTS );
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // 1-255 or 0 to disable  

  cout << "WaveLight 1.0\n";
  cout << "BS="<< LED_STRIP_BUFFER_SIZE <<"\n";
  const int maxLights = strip.numPixels();
  cout << "ML="<<maxLights<<"\n";
  cout << "RTC="<<!rtc.isHalted()<<"\n";
  cout << "Test\r";
  
  track_lights.nb_lights = 0;
  int ticks = 0;
  for(int i=1;i<maxLights && track_lights.nb_lights==0;i++)
  {
    cout << "Test "<<i<<"\r";
    strip.updateLength(i);
    strip.setBrightness(50);
    strip.clear();
    strip.setPixelColor( i-1 , 255 , 255 , 255 );
    loopBackSignalCounter.resetCounter();
    strip.show();
    delayTimer.delayMicroseconds( 100000 );
    ticks = loopBackSignalCounter.counter();
    if( ticks > 6 && ticks < 32 ) { track_lights.nb_lights = i; }
  }
  lcdIO.m_rawIO.clear();
  lcdIO.m_rawIO.setCursor(0, 0);

  if( track_lights.nb_lights > 0 ) -- track_lights.nb_lights;
  if( track_lights.nb_lights <= 0 ) track_lights.nb_lights = maxLights;
  if( ticks != 24 ) cout << "bad lb sig ("<<ticks<<")\n";
  else cout << "lb Ok :-)\n";
  cout << "Lights="<<track_lights.nb_lights<<"\n";

  delayTimer.delayMicroseconds( 5000000 );

  strip.updateLength( track_lights.nb_lights );
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(50); // 1-255 or 0 to disable
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP

/*  
  cout << "WaveLight 1.0\n";
  cout << "BUF="<< LED_STRIP_BUFFER_SIZE <<"\n";
  cout << "TRK="<< track_lights.nb_lights <<"\n";
  cout << "NL="<< strip.numPixels() <<"\n";
*/
}

// positions given in decimeters, up to 6.5Km
void update_lights( const TrackLights& track, const uint16_t* pos, int n )
{
  strip.clear();
  
  for(int i=0;i<n;i++)
  {
    const uint32_t p = pos[i];
    const uint32_t lx16 = ( (p*track.nb_lights*16) / track.length ) % (track.nb_lights*16);
    const uint16_t l = lx16 / 16;
    uint8_t frac = lx16 % 16; // fraction between the light and the light after in [0;15]
    const uint16_t l2 = (l+1) % track.nb_lights;
    
    const uint16_t R = pgm_read_byte( & partner_color[(i%MAX_PARTNERS)*3+0] );
    const uint16_t B = pgm_read_byte( & partner_color[(i%MAX_PARTNERS)*3+1] );
    const uint16_t G = pgm_read_byte( & partner_color[(i%MAX_PARTNERS)*3+2] );
    strip.setPixelColor( l2 , (R*frac)/16 , (G*frac)/16 , (B*frac)/16 );
    frac = 15 - frac;
    strip.setPixelColor( l , (R*frac)/16 , (G*frac)/16 , (B*frac)/16 );
  }
  
  strip.show();
}

void loop()
{
  static uint16_t counter = 0;
  delayTimer.delayMicroseconds( 100000 );

  ++ counter;

  cout << "Dist " << counter/10 <<'.' << (counter%10) <<"m    \r";

  uint16_t pos[3] = { counter % track_lights.length , (counter+1000) % track_lights.length , (counter+2000) % track_lights.length };

  update_lights( track_lights , pos , 3 );
}

