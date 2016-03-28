/*
Arduino library v1.0
LCD12864R
www.DFRobot.com
*/

#ifndef LCD12864_h
#define LCD12864_h
#include <inttypes.h>

#include <BasicIO/ByteStream.h>

class LCD12864 : public ByteStream
{
  public:

#define DEFAULTTIME 80
#define LINESIZE 16
#define LINECOUNT 4


	LCD12864();
	void Initialise(void);
	void setPins(bool tRS, bool tRW, uint8_t tDataBus);
	void delayns(void);
	void VectorConverter(int vector);

	void WriteCommand(int CMD);
	void WriteData(int CMD);

	void Clear(void);
	void DisplayString(int X,int Y,uint8_t *ptr,int dat);
	void DrawFullScreen(uint8_t *p);

	// ByteStream Interface
	virtual bool writeByte( uint8_t x );

	void flush();
	uint8_t ConsoleBuffer[4][16];
	uint8_t cx, cy;
};

#endif
