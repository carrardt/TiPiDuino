#pragma once

#define MIN_PULSE_LEN 		150		// under this threshold noise is too important
#define MAX_PULSE_LEN 		32000 	// mainly due to 16-bits limitation
#define PULSE_ERR_RATIO 	8   	// 1/8 = 12,5% relative pulse length error
#define MAX_LATCH_SEQ_LEN	2
#define MAX_SYMBOLS 		(MAX_LATCH_SEQ_LEN+2)		// !! MUST SATISFY ( MAX_SYMBOLS < MIN_PULSE_LEN ) == TRUE !!
#define MAX_MESSAGE_BITS	128
#define MAX_MESSAGE_BYTES	(MAX_MESSAGE_BITS/8)

// Possibly detected encodings
enum MessageEncoding
{
	CODING_UNKNOWN = '?',	// unknown encoding
	CODING_BINARY = 'B',	// raw binary coding
	CODING_MANCHESTER = 'M'	// manchester coding (bipolar)
};

