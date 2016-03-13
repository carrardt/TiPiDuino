#include <AvrTL.h>
#include <AvrTLPin.h>
#include <HWSerialIO.h>

//using namespace avrtl;

struct FP16
{
	static constexpr int ibits = 5;
	static constexpr int sbits = 1;
	static constexpr int fbits = 10;
	static constexpr float to_float = 1.0f / (1<<fbits);
	static constexpr float from_float = static_cast<float>( 1<<fbits );

	inline FP16() {}
	inline FP16(float f) { fp = static_cast<int16_t>(f*from_float); }
	inline FP16(int16_t _fp) { setFP(_fp); }
	inline FP16(const FP16& x) : fp(x.fp) {}

	inline int floor() const { return fp>>fbits; }
	inline void setFP(int16_t v) { fp=v; }

	inline FP16& operator = ( FP16 x ) { fp = x.fp; return *this; }
	inline FP16& operator += ( FP16 x ) { fp += x.fp; return *this; }
	inline FP16& operator *= ( FP16 x ) { int32_t t=fp; t*=x.fp; fp=t>>fbits; return *this; }

	inline FP16 operator + ( FP16 x ) const { FP16 r; r.setFP(fp+x.fp); return r; }
	inline FP16 operator - ( FP16 x ) const { FP16 r; r.setFP(fp-x.fp); return r; }
	inline FP16 operator - () const { FP16 r; r.setFP(-fp); return r; }
	inline FP16 operator * ( FP16 x ) const { int32_t t=fp; t*=x.fp; FP16 r; r.setFP(t>>fbits); return r; }

	int16_t fp;
};

/* squared complex magnitude */
static FP16 mag(FP16 re, FP16 im)
{
	return (re*re)+(im*im);
}

#define SIN_2PI_16 FP16(0.38268343236508978f)
#define SIN_4PI_16 FP16(0.707106781186547460f)
#define SIN_6PI_16 FP16(0.923879532511286740f)
#define C_P_S_2PI_16 FP16(1.30656296487637660f)
#define C_M_S_2PI_16 FP16(0.54119610014619690f)
#define C_P_S_6PI_16 FP16(1.3065629648763766f)
#define C_M_S_6PI_16 FP16(-0.54119610014619690f)

/* INPUT: float input[16], float output[16] */
/* OUTPUT: none */
/* EFFECTS:  Places the 16 point fft of input in output in a strange */
/* order using 10 real multiplies and 79 real adds. */
/* Re{F[0]}= out0 */
/* Im{F[0]}= 0 */
/* Re{F[1]}= out8 */
/* Im{F[1]}= out12 */
/* Re{F[2]}= out4 */
/* Im{F[2]}= -out6 */
/* Re{F[3]}= out11 */
/* Im{F[3]}= -out15 */
/* Re{F[4]}= out2 */
/* Im{F[4]}= -out3 */
/* Re{F[5]}= out10 */
/* Im{F[5]}= out14 */
/* Re{F[6]}= out5 */
/* Im{F[6]}= -out7 */
/* Re{F[7]}= out9 */
/* Im{F[7]}= -out13 */
/* Re{F[8]}= out1 */
/* Im{F[8]}=0 */
/* F[9] through F[15] can be found by using the formula */
/* Re{F[n]}=Re{F[(16-n)mod16]} and Im{F[n]}= -Im{F[(16-n)mod16]} */

/* Note using temporary variables to store intermediate computations */
/* in the butterflies might speed things up.  When the current version */
/* needs to compute a=a+b, and b=a-b, I do a=a+b followed by b=a-b-b.  */
/* So practically everything is done in place, but the number of adds */
/* can be reduced by doinc c=a+b followed by b=a-b. */

/* The algorithm behind this program is to find F[2k] and F[4k+1] */
/* seperately.  To find F[2k] we take the 8 point Real FFT of x[n]+x[n+8] */
/* for n from 0 to 7.  To find F[4k+1] we take the 4 point Complex FFT of */
/* exp(-2*pi*j*n/16)*{x[n] - x[n+8] + j(x[n+12]-x[n+4])} for n from 0 to 3.*/

#define BUFSIZE 32
static FP16 buffer[BUFSIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint8_t cursor = 0;
static FP16 spectrum[8];

static FP16 output0, output1, output2, output3, output4, output5, output6, output7, output8, output9, output10, output11, output12, output13, output14, output15;
static FP16 temp, out0, out1, out2, out3, out4, out5, out6, out7, out8;
static FP16 out9,out10,out11,out12,out13,out14,out15;

static inline void R16SRFFT_part1()
{
#define FFT_INPUT(I) buffer[(cursor+I)%BUFSIZE]

  out0=FFT_INPUT(0)+FFT_INPUT(8); /* output[0 through 7] is the data that we */
  out1=FFT_INPUT(1)+FFT_INPUT(9); /* take the 8 point real FFT of. */
  out2=FFT_INPUT(2)+FFT_INPUT(10);
  out3=FFT_INPUT(3)+FFT_INPUT(11);
  out4=FFT_INPUT(4)+FFT_INPUT(12);
  out5=FFT_INPUT(5)+FFT_INPUT(13);
  out6=FFT_INPUT(6)+FFT_INPUT(14);
  out7=FFT_INPUT(7)+FFT_INPUT(15);

  out8=FFT_INPUT(0)-FFT_INPUT(8);   /* inputs 8,9,10,11 are */
  out9=FFT_INPUT(1)-FFT_INPUT(9);   /* the Real part of the */
  out10=FFT_INPUT(2)-FFT_INPUT(10); /* 4 point Complex FFT inputs.*/
  out11=FFT_INPUT(3)-FFT_INPUT(11); 
  out12=FFT_INPUT(12)-FFT_INPUT(4); /* outputs 12,13,14,15 are */
  out13=FFT_INPUT(13)-FFT_INPUT(5); /* the Imaginary pars of  */
  out14=FFT_INPUT(14)-FFT_INPUT(6); /* the 4 point Complex FFT inputs.*/
  out15=FFT_INPUT(15)-FFT_INPUT(7);

#undef FFT_INPUT

  /*First we do the "twiddle factor" multiplies for the 4 point CFFT */
  /*Note that we use the following handy trick for doing a complex */
  /*multiply:  (e+jf)=(a+jb)*(c+jd) */
  /*   e=(a-b)*d + a*(c-d)   and    f=(a-b)*d + b*(c+d)  */

  /* C_M_S_2PI/16=cos(2pi/16)-sin(2pi/16) when replaced by macroexpansion */
  /* C_P_S_2PI/16=cos(2pi/16)+sin(2pi/16) when replaced by macroexpansion */
  /* (SIN_2PI_16)=sin(2pi/16) when replaced by macroexpansion */
  temp=(out13-out9)*(SIN_2PI_16); 
  out9=out9*(C_P_S_2PI_16)+temp; 
  out13=out13*(C_M_S_2PI_16)+temp;
  
  out14*=(SIN_4PI_16);
  out10*=(SIN_4PI_16);
  out14=out14-out10;
  out10=out14+out10+out10;
  
  temp=(out15-out11)*(SIN_6PI_16);
  out11=out11*(C_P_S_6PI_16)+temp;
  out15=out15*(C_M_S_6PI_16)+temp;

  /* The following are the first set of two point butterfiles */
  /* for the 4 point CFFT */

  out8+=out10;
  out10=out8-out10-out10;

  out12+=out14;
  out14=out12-out14-out14;

  out9+=out11;
  out11=out9-out11-out11;

  out13+=out15;
  out15=out13-out15-out15;

  /*The followin are the final set of two point butterflies */
  output1=out8+out9;
  output7=out8-out9;

  output9=out12+out13;
  output15=out13-out12;
  
  output5=out10+out15;        /* implicit multiplies by */
  output13=out14-out11;        /* a twiddle factor of -j */                            
  output3=out10-out15;  /* implicit multiplies by */
  output11=-out14-out11;  /* a twiddle factor of -j */

  /* What follows is the 8-point FFT of points output[0-7] */
  /* This 8-point FFT is basically a Decimation in Frequency FFT */
  /* where we take advantage of the fact that the initial data is real*/

  /* First set of 2-point butterflies */
    
  out0=out0+out4;
  out4=out0-out4-out4;
  out1=out1+out5;
  out5=out1-out5-out5;
  out2+=out6;
  out6=out2-out6-out6;
  out3+=out7;
  out7=out3-out7-out7;

  /* Computations to find X[0], X[4], X[6] */
  
  output0=out0+out2;
  output4=out0-out2;
  out1+=out3;
  output12=out3+out3-out1;

  output0+=out1;  /* Real Part of X[0] */
  output8=output0-out1-out1; /*Real Part of X[4] */
  /* out2 = Real Part of X[6] */
  /* out3 = Imag Part of X[6] */
  
  /* Computations to find X[5], X[7] */

  out5*=SIN_4PI_16;
  out7*=SIN_4PI_16;
  out5=out5-out7;
  out7=out5+out7+out7;

  output14=out6-out7; /* Imag Part of X[5] */
  output2=out5+out4; /* Real Part of X[7] */
  output6=out4-out5; /*Real Part of X[5] */
  output10=-out7-out6; /* Imag Part of X[7] */

  spectrum[6] = mag( output7 , output15 );
  spectrum[7] = mag( output8 , 0.0f );
}

static inline void R16SRFFT_part2()
{
  spectrum[0] = mag( output1 , output9 );
  spectrum[1] = mag( output2 , output10 );
  spectrum[2] = mag( output3 , output11 );
  spectrum[3] = mag( output4 , output12 );
  spectrum[4] = mag( output5 , output13 );
  spectrum[5] = mag( output6 , output14 );
}

static void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0) ;
	ADMUX &= ~(1<<ADLAR); // set ADLAR to 0 => rifght adjust
 
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

static void adc_channel(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
}

static inline void adc_read_start()
{
  // start single convertion
  // write ’1′ to ADSC
	ADCSRA |= (1<<ADSC);
}

static inline uint16_t adc_read_end()
{ 
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
  return (ADC);
}

static uint16_t adc_read10()
{ 
  adc_read_start();
  return adc_read_end();
}

void setup()
{
	DDRD |= 0xFC; // 6 highest bits bits from port D (pins 2,3,4,5,6,7)
	DDRB |= 0X03; // 2 lowest bits from port B (pins 8,9)
	adc_init();
	adc_channel(0);
	cli();
	adc_read_start();
}

static inline void writeLeds( uint8_t x )
{
	uint8_t d = PIND;
	uint8_t b = PINB;
	uint8_t tb = (b&0xFC) | ((x>>6)&0x03);
	uint8_t td = (d&0X03) | ((x<<2)&0xFC);
	PIND = d^td;
	PINB = b^tb;
}

// if defined sample rate is approx 6Khz (on an atmega328@16Mhz), otherwise rate is 3Khz
//#define DOUBLE_SAMPLE_RATE 1

void loop()
{
   static uint8_t LOOP_CLK = 0;
   static uint8_t t0;
   static uint8_t t1;

   buffer[ (cursor+16) % BUFSIZE ].fp = adc_read_end();
   adc_read_start();   
   R16SRFFT_part1();
   
#ifdef DOUBLE_SAMPLE_RATE
   buffer[ (cursor+17) % BUFSIZE ].fp = adc_read_end();
   adc_read_start();
   R16SRFFT_part2();
   cursor = (cursor+2) % BUFSIZE;
#else
   R16SRFFT_part2();
   cursor = (cursor+1) % BUFSIZE;
#endif

   uint8_t LEDS = 0; (LOOP_CLK>>6)&1;
   for(int i=0;i<8;i++)
   {
	  LEDS = LEDS << 1;
	  if( (spectrum[i].fp >> 11) != 0 ) LEDS |= 1;
   }
   writeLeds( LEDS );
}








