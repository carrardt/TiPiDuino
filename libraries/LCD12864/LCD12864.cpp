/*

Arduino library v1.0
LCD12864R
www.DFRobot.com
*/
#include "LCD12864.h"

#include <AvrTL/AvrTLPin.h>
#include <AvrTL/AvrTLSignal.h>

static avrtl::StaticPin<17> RS;
static avrtl::StaticPin<16> RW;
static avrtl::StaticPin<18> EN;
static avrtl::StaticPin<8> D0;
static avrtl::StaticPin<9> D1;
static avrtl::StaticPin<10> D2;
static avrtl::StaticPin<11> D3;
static avrtl::StaticPin<4> D4;
static avrtl::StaticPin<5> D5;
static avrtl::StaticPin<6> D6;
static avrtl::StaticPin<7> D7;

/*
static const int RS = 17; 
static const int RW = 16;  
static const int EN = 18;  
static const int D0  = 8;  
static const int D1  = 9; 
static const int D2  = 10;  
static const int D3  = 11;  
static const int D4  = 4; 
static const int D5  = 5;  
static const int D6  = 6;  
static const int D7  = 7; 
*/

LCD12864::LCD12864() 
	: cx(0), cy(0)
{
	for(uint8_t y=0;y<LINECOUNT;y++)
	{
		for(uint8_t x=0;x<LINESIZE;x++) ConsoleBuffer[y][x]=' ';
	}
} 

bool LCD12864::writeByte( uint8_t x )
{
	bool newline = false;
	bool newchar = false;

	if( x=='\n' ) { ++cy; cx=0; newline=true; }
	else if( x=='\r' || x=='\0' ) { return true; }
	else
	{
		++ cx;
		newchar = true;		
		if(cx==LINESIZE)
		{
			cx = 0; 
			++ cy;
			newline = true;
		}
	}
	
	if( newchar && cy == LINECOUNT )
	{
		for(uint8_t y=0;y<(LINECOUNT-1);y++)
		{
			for(uint8_t x=0;x<LINESIZE;x++) ConsoleBuffer[y][x]=ConsoleBuffer[y+1][x];
		}
		for(uint8_t x=0;x<LINESIZE;x++) ConsoleBuffer[LINECOUNT-1][x] = ' ';
		cy = LINECOUNT-1;
	}
	if( newchar ) { ConsoleBuffer[cy][cx] = x; }
	if( newline ) { flush(); }
	return true;
}

void LCD12864::flush()
{
	//Clear();
	for(uint8_t y=0;y<(LINECOUNT-1);y++)
	{
		DisplayString(y,0,ConsoleBuffer[y],16);
	}	
}

void LCD12864::setPins(bool tRS, bool tRW, uint8_t vector) 
{
	RS = tRS;
	RW = tRW;
	D0 = vector&1 ; vector=vector>>1;
	D1 = vector&1 ; vector=vector>>1;
	D2 = vector&1 ; vector=vector>>1;
	D3 = vector&1 ; vector=vector>>1;
	D4 = vector&1 ; vector=vector>>1;
	D5 = vector&1 ; vector=vector>>1;
	D6 = vector&1 ; vector=vector>>1;
	D7 = vector&1 ;

	EN = HIGH;
	delayns();
	EN = LOW;
	delayns();
}

//*********************延时函数************************//
void LCD12864::delayns(void)
{   
	avrtl::DelayMicroseconds(DEFAULTTIME);
}

void LCD12864::WriteCommand(int CMD)
{ 
   delayns();
   delayns();
   setPins(0,0,CMD); //WriteCommand

}
void LCD12864::WriteData(int CMD)
{  
   delayns();
   delayns();
   setPins(1,0,CMD); //WriteData
}

void LCD12864::Initialise(void) 
{
	RS.SetOutput();
	RW.SetOutput();
	EN.SetOutput();
	D0.SetOutput();
	D1.SetOutput();
	D2.SetOutput();
	D3.SetOutput();
	D4.SetOutput();
	D5.SetOutput();
	D6.SetOutput();
	D7.SetOutput();

	delayns();
	WriteCommand(0x30);        //功能设定控制字
	WriteCommand(0x0c);        //显示开关控制字
	WriteCommand(0x01);        //清除屏幕控制字
	WriteCommand(0x06);        //进入设定点控制字
}
void LCD12864::Clear(void)
{  
    WriteCommand(0x30);//
    WriteCommand(0x01);//清除显示
}
void LCD12864::DisplayString(int X,int Y,uint8_t *ptr,int dat)
{
  int i;
  switch(X)
   {
     case 0:  Y|=0x80;break;

     case 1:  Y|=0x90;break;

     case 2:  Y|=0x88;break;

     case 3:  Y|=0x98;break;

     default: break;
   }
  WriteCommand(Y);// 定位显示起始地址
  for(i=0;i<dat;i++)
    { 
      WriteData(ptr[i]);//显示汉字时注意码值，连续两个码表示一个汉字
    }
}

void LCD12864::DrawFullScreen(uint8_t *p)
{
	int ygroup,x,y,i;
	int temp;
	int tmp;

	for(ygroup=0;ygroup<64;ygroup++) //写入液晶上半图象部分
	{                           //写入坐标
		if(ygroup<32)
		{
			x=0x80;
			y=ygroup+0x80;
		}
		else 
		{
			x=0x88;
			y=ygroup-32+0x80;    
		}         
		WriteCommand(0x34);        //写入扩充指令命令
		WriteCommand(y);           //写入y轴坐标
		WriteCommand(x);           //写入x轴坐标
		WriteCommand(0x30);        //写入基本指令命令
		WriteCommand(0x0c);
		tmp=ygroup*16;
		for(i=0;i<16;i++)
		{
			temp=p[tmp++];
			WriteData(temp);
		}
	}
	WriteCommand(0x34);        //写入扩充指令命令
	WriteCommand(0x36);        //显示图象
}
