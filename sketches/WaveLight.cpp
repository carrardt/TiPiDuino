#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"
#include "BasicIO/PrintStream.h"

#include <PCD8544.h>
#include "WS2811/ws2811.h"
#include <avr/interrupt.h>

// PCD8544 pins : SCLK, SDIN, DC, RST, SCE (connect to ground)
#define LCD_PINS     2,    3,  4,   5, PCD8544_UNASSIGNED
static PCD8544 lcd( LCD_PINS );

static ByteStreamAdapter<PCD8544> lcdIO;
static PrintStream cout;

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

static Adafruit_NeoPixel strip(100);
static TrackLights track_lights = {};

void setup()
{
	cli();

  lcdIO.m_rawIO.setPins( LCD_PINS );
  
  // PCD8544-compatible displays may have a different resolution...
  lcdIO.m_rawIO.begin(84, 48);

  lcdIO.m_rawIO.setContrast(63);
  cout.begin(&lcdIO);
  lcdIO.m_rawIO.setCursor(0, 0);
  
  strip.updateLength( track_lights.nb_lights );
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  cout << "WaveLight 1.0\n";
  cout << "TL="<< track_lights.nb_lights <<"\n";
  cout << "BS="<< strip.bufferSize() <<"\n";
  cout << "NL="<< strip.numPixels() <<"\n";

  for(int i=0;i<30;i++) avrtl::delayMicroseconds( 100000 );
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
  avrtl::delayMicroseconds( 10000 );

  ++ counter;
  cout << "dist=" << counter/10 <<"m    \n";

  uint16_t pos[3] = { counter % track_lights.length , (counter+1000) % track_lights.length , (counter+2000) % track_lights.length };

  update_lights( track_lights , pos , 3 );
}

