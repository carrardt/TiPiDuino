/*
 * AvrTL.h
 * 
 * No Copyright
 * 
 */

#ifndef __AvrTL_H
#define __AvrTL_H

#include <stdint.h>

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP 0
#endif

#ifdef abs
#undef abs // if defined in Arduino.h
#endif

namespace avrtl
{	
	template<typename T>
	inline T abs(T x) { return (x<0) ? (-x) : x ; }

	inline uint8_t checksum8(const uint8_t* buf, int nbytes)
	{
		uint8_t cs = 0;
		for(int i=0;i<nbytes;i++)
		{
			cs = ( (cs<<1) | (cs >>7) ) ^ buf[i];
		}
		return cs;
	}

	template<typename T>
	inline void rotateBufferLeft1(T * ptr, int n)
	{
		T first = ptr[0];
		for(int i=0;i<(n-1);i++)
		{
			ptr[i] = ptr[i+1];
		}
		ptr[n-1] = first;
	}

	template<typename T>
	inline void rotateBufferLeft(T * ptr, int n, int disp)
	{
		for(int i=0;i<disp;i++)
		{
			rotateBufferLeft1(ptr,n);
		}
	}
	
	template<typename T>
	inline int findOccurence(const T* pattern, int psize, const T* buf, int bsize)
	{
		int j = 0;
		int start = 0;
		for(int i=0;i<bsize;i++)
		{
			if( buf[i]==pattern[j] )
			{
				++j;
				if( j == psize ) return i-psize+1;
			}
			else
			{
				i = start++;
				j = 0;
			}
		}
		return -1;
	}

}

// Arduino compatibility layer
#if !defined(ARDUINO_MAIN) && !defined(Arduino_h)
using avrtl::abs;
#endif

#endif
