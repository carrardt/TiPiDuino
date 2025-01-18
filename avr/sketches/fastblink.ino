#define CLK_PIN 3

void setup()
{
	pinMode(CLK_PIN,OUTPUT);
	cli();
}

void loop()
{
	static uint8_t c = 0;
	static uint16_t t = 0;
	while( TCNT0 == t );
	t=TCNT0;
	++c;
	digitalWrite(CLK_PIN,(c>>9)&1);
}
