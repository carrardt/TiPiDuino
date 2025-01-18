/*
Arduino library v1.0
LCD12864R
www.DFRobot.com
*/

#ifndef __LCD12864_h
#define __LCD12864_h
#include <inttypes.h>
#include <BasicIO/TextFrameBuffer.h>
#include <TimeScheduler/TimeScheduler.h>

#define LCD12864_BUILTIN_FONT 1


#ifdef LCD12864_BUILTIN_FONT
#define LCD12864_ROWS 4
#else
#define LCD12864_ROWS 8
#endif

// TODO: an option without text frame buffer if not necessary
class LCD12864 : public TextFrameBuffer<16,LCD12864_ROWS>
{
  public:

	static constexpr uint16_t DelayTime = 80;
	static constexpr uint8_t cols = 16;
	static constexpr uint8_t rows = LCD12864_ROWS;

	LCD12864();
	void Initialise();
	void setPins(bool tRS, bool tRW, uint8_t tDataBus);
	void delayns();

	void WriteCommand(uint8_t CMD);
	void WriteData(uint8_t CMD);

	void Clear();
	void DisplayString(int X,int Y,uint8_t *ptr,int dat);
	void DrawFullScreen(uint8_t *p);
#ifndef LCD12864_BUILTIN_FONT
	void RenderText8x8();
#endif
	void render();
};

#endif
