#include <stdio.h>
#include <cmath>
#include <cstdint>

//#define FAKE_FP16 1

/*
 * FLOAT RANGE = approximately [-8;8] ( with input signal in [0;1] )
 * so 1 bit for sign, 5 bits for integer and 10 bits for fraction is enough.
 * could be 4 bits for integer and 11 bits for fraction
 */

struct FP16
{
	static constexpr int ibits = 5;
	static constexpr int sbits = 1;
	static constexpr int fbits = 10;
	static constexpr float to_float = 1.0f / (1<<fbits);
	static constexpr float from_float = static_cast<float>( 1<<fbits );

	inline FP16() {}
	inline FP16(float x) { fromFloat(x); }
	inline FP16(const FP16& x) : fp(x.fp) {}

	inline operator float () const { return toFloat(); }

#ifdef FAKE_FP16
	inline void _fromFloat(float x) { fp = x; }
	inline float toFloat() const { return fp; }
	inline int floor() const { return std::floor(fp); }
#else
	inline void _fromFloat(float x) { fp = static_cast<int>(x*from_float); }
	inline float toFloat() const { return fp * to_float; }
	inline int floor() const { return fp>>fbits; }
	inline void setFP(int16_t v) {fp=v;}
#endif

	inline void fromFloat(float x)
	{
		_fromFloat(x);
		updateRange();
	}

	inline void updateRange()
	{
		float x = toFloat();
		if( x < rmin ) rmin = x;
		if( x > rmax ) rmax = x;
	}

	inline FP16& operator = ( FP16 x ) { fp = x.fp; return *this; }

	inline FP16& operator += ( FP16 x ) { fp += x.fp; return *this; }
	inline FP16& operator += ( float x ) { fromFloat( toFloat() + x ); return *this; }

	inline FP16& operator *= ( FP16 x ) { int32_t t=fp; t*=x.fp; fp=t>>fbits; return *this; }
	inline FP16& operator *= ( float x ) { fromFloat( toFloat() * x ); return *this; }

	inline FP16 operator + ( FP16 x ) const { FP16 r; r.setFP(fp+x.fp); return r; }
	inline FP16 operator + ( float x ) const { return FP16(toFloat()+x); }

	inline FP16 operator - ( FP16 x ) const { FP16 r; r.setFP(fp-x.fp); return r; }
	inline FP16 operator - ( float x ) const { return FP16(toFloat()-x); }

	inline FP16 operator * ( FP16 x ) const { int32_t t=fp; t*=x.fp; FP16 r; r.setFP(t>>fbits); return r; }
	inline FP16 operator * ( float x ) const { return FP16(toFloat()*x); }

#ifdef FAKE_FP16
	float fp;
#else
	int16_t fp;
#endif

	static float rmin, rmax;
};

float FP16::rmin=0.0f;
float FP16::rmax=0.0f;

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

static float mag(FP16 re, FP16 im)
{
	return	sqrtf(re*re+im*im);
}

void R16SRFFT(float input[16],float spectrum[8] ) {
  FP16 temp, out0, out1, out2, out3, out4, out5, out6, out7, out8;
  FP16 out9,out10,out11,out12,out13,out14,out15;
  FP16 output[16];

  out0=input[0]+input[8]; /* output[0 through 7] is the data that we */
  out1=input[1]+input[9]; /* take the 8 point real FFT of. */
  out2=input[2]+input[10];
  out3=input[3]+input[11];
  out4=input[4]+input[12];
  out5=input[5]+input[13];
  out6=input[6]+input[14];
  out7=input[7]+input[15];



  out8=input[0]-input[8];   /* inputs 8,9,10,11 are */
  out9=input[1]-input[9];   /* the Real part of the */
  out10=input[2]-input[10]; /* 4 point Complex FFT inputs.*/
  out11=input[3]-input[11]; 
  out12=input[12]-input[4]; /* outputs 12,13,14,15 are */
  out13=input[13]-input[5]; /* the Imaginary pars of  */
  out14=input[14]-input[6]; /* the 4 point Complex FFT inputs.*/
  out15=input[15]-input[7];

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
  output[1]=out8+out9;
  output[7]=out8-out9;

  output[9]=out12+out13;
  output[15]=out13-out12;
  
  output[5]=out10+out15;        /* implicit multiplies by */
  output[13]=out14-out11;        /* a twiddle factor of -j */                            
  output[3]=out10-out15;  /* implicit multiplies by */
  output[11]=-out14-out11;  /* a twiddle factor of -j */

  
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
  
  output[0]=out0+out2;
  output[4]=out0-out2;
  out1+=out3;
  output[12]=out3+out3-out1;

  output[0]+=out1;  /* Real Part of X[0] */
  output[8]=output[0]-out1-out1; /*Real Part of X[4] */
  /* out2 = Real Part of X[6] */
  /* out3 = Imag Part of X[6] */
  
  /* Computations to find X[5], X[7] */

  out5*=SIN_4PI_16;
  out7*=SIN_4PI_16;
  out5=out5-out7;
  out7=out5+out7+out7;

  output[14]=out6-out7; /* Imag Part of X[5] */
  output[2]=out5+out4; /* Real Part of X[7] */
  output[6]=out4-out5; /*Real Part of X[5] */
  output[10]=-out7-out6; /* Imag Part of X[7] */

  spectrum[0] = mag( output[1] , output[9] );
  spectrum[1] = mag( output[2] , output[10] );
  spectrum[2] = mag( output[3] , output[11] );
  spectrum[3] = mag( output[4] , output[12] );
  spectrum[4] = mag( output[5] , output[13] );
  spectrum[5] = mag( output[6] , output[14] );
  spectrum[6] = mag( output[7] , output[15] );
  spectrum[7] = mag( output[8] , 0.0f );	
}


int main() {
  float data[16];
  float output[16];
  float zero=0;

  scanf("%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",&data[0],&data[1],&data[2],&data[3],&data[4],&data[5],&data[6],&data[7],&data[8],&data[9],&data[10],&data[11],&data[12],&data[13],&data[14],&data[15]);
  printf("input: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);

#ifdef FAKE_FP16
  FP16::rmin = FP16::rmax = data[0];
#endif

  R16SRFFT(data,output);
  printf("\nresult is:\n");
  printf("k,\t\tReal Part\t\tImaginary Part\t\tPower\n");
  //print_result(0,output[0],zero);
  print_result(1,output[1],output[9]);
  print_result(2,output[2],output[10]);
  print_result(3,output[3],output[11]);
  print_result(4,output[4],output[12]);
  print_result(5,output[5],output[13]);
  print_result(6,output[6],output[14]);
  print_result(7,output[7],output[15]);
  print_result(8,output[8],zero);

#ifdef FAKE_FP16
  printf("float range = [%g;%g]\n",FP16::rmin,FP16::rmax);
#endif

	return 0;
}








