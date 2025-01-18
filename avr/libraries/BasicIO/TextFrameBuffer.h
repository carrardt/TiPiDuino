#ifndef __BASICIO_TEXTFRAMEBUFFER_H
#define __BASICIO_TEXTFRAMEBUFFER_H

#include "BasicIO/ByteStream.h"
#include <inttypes.h>

template<uint8_t _cols, uint8_t _rows>
struct TextFrameBuffer : public ByteStream
{
	static constexpr uint8_t cols = _cols;
	static constexpr uint8_t rows = _rows;
	
	inline TextFrameBuffer()
		: m_cx(0)
		, m_cy(0)
	{
		for(uint8_t l=0;l<rows;l++)
		{
			for(uint8_t c=0;c<cols;c++)
			{
				m_buffer[l][c] = ' ';
			}
		}
	}

	virtual void render() {}

	void scroll()
	{
		int s = 1 + (int)m_cy - (int)rows ;
		if( s <= 0 ) return;
		for(int y=0;y<rows;y++)
		{
				for(uint8_t x=0;x<cols;x++)
				{
					uint8_t c = ' ';
					if( (y+s) < rows ) c = m_buffer[y+s][x];
					m_buffer[y][x] = c;
				}
		}
		render();
		m_cy = rows-1;
	}

	bool writeByte(uint8_t c)
	{
		if( c=='\n' )
		{ 
			++m_cy;
			m_cx=0; 
			render(); 
			return true;
		}
		if( c=='\r' || c=='\0' ) { return true; }
		if( m_cx >= cols )
		{
			++m_cy;
			m_cx = 0;
		}
		scroll();
		m_buffer[m_cy][m_cx++] = c;
		return true;
	}

	uint8_t m_buffer[rows][cols];
	uint8_t m_cx, m_cy;
};

#endif
