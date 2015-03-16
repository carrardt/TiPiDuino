#ifndef __RFSNIFFERCONSTANTS_H
#define __RFSNIFFERCONSTANTS_H

#define PULSE_LVL 			false // = 0 = LOW
#define MAX_PULSES 			512
#define MIN_PULSE_LEN 		150	// under this threshold noise is too important
#define MAX_PULSE_LEN 		30000 // mainly due to 16-bits limitation
#define MIN_LATCH_LEN 		2000
#define MIN_PROLOG_LATCHES 	1
#define MIN_MESSAGE_PULSES 	64
#define PULSE_ERR_RATIO 	8   // 4(25%) would speedup execution ...
#define MAX_SYMBOLS 		8	// !! MUST VERIFY ( MAX_SYMBOLS < MIN_PULSE_LEN ) !!
#define MAX_LATCH_SEQ_LEN	8
#define MAX_MESSAGE_BITS	128
#define MAX_MESSAGE_BYTES	(MAX_MESSAGE_BITS/8)
#define EEPROM_MAGIC_NUMBER (0x26101977UL)

// Possibly detected encodings
enum MessageEncoding
{
	CODING_UNKNOWN = 0,
	CODING_BINARY = 1,
	CODING_MANCHESTER = 2
};
static constexpr char codingChar[] = {'?','B','M'};

#endif
