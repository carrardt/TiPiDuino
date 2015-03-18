#ifndef __RFSNIFFERCONSTANTS_H
#define __RFSNIFFERCONSTANTS_H

#define PULSE_LVL 			false // = 0 = LOW
#define MIN_PULSE_LEN 		150	// under this threshold noise is too important
#define MAX_PULSE_LEN 		30000 // mainly due to 16-bits limitation
#define PULSE_ERR_RATIO 	8   // 12,5% relative pulse length error
#define MAX_SYMBOLS 		8	// !! MUST VERIFY ( MAX_SYMBOLS < MIN_PULSE_LEN ) !!
#define MAX_LATCH_SEQ_LEN	6
#define MAX_MESSAGE_BITS	128
#define MAX_MESSAGE_BYTES	(MAX_MESSAGE_BITS/8)
#define EEPROM_MAGIC_NUMBER (0x26101976UL)

// Possibly detected encodings
enum MessageEncoding
{
	CODING_UNKNOWN = '?',
	CODING_BINARY = 'B',
	CODING_MANCHESTER = 'M'
};

#endif
