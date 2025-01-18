
#define RECEIVE_PIN        9 
#define PULSE_LVL          LOW
#define BAD_BIT            0xFF
#define NOISE_THRESHOLD    100
#define MIN_VALID_SAMPLES  16
#define BUF_SIZE           768
#define RESOLUTION         16

void setup()
{
  pinMode(RECEIVE_PIN, INPUT);
  Serial.begin(9600);
  Serial.println("READY");
}

unsigned long pulseLenHistogram(byte* histogram, unsigned int* pMaxValue, unsigned int bufSize, unsigned int resolution, unsigned int minPulseLen, unsigned int minValidSamples)
{
  unsigned long samples = 0;
  unsigned long t;

  {
    unsigned int seqWarmup[minValidSamples];
    unsigned int warmup = 0;

    // wait for 1st valid pulse
    do {
      t = pulseIn(RECEIVE_PIN, PULSE_LVL);
    } 
    while( t < minPulseLen );
    seqWarmup[warmup++] = min( ( t - minPulseLen ) / resolution , bufSize-1 );

    // check for consecutive valid pulses
    while( warmup < minValidSamples )
    {
      t = pulseIn(RECEIVE_PIN, PULSE_LVL);
      if( t < minPulseLen ) return 0;
      seqWarmup[warmup++] = min( ( t - minPulseLen ) / resolution , bufSize-1 );
    }

    for(warmup=0;warmup<minValidSamples;warmup++)
    {
      t = seqWarmup[warmup];
      ++ histogram[ t ];
      ++samples;
      *pMaxValue = max( *pMaxValue , histogram[t] );
    }
  }

  while( *pMaxValue < 255 )
  {
    t = pulseIn(RECEIVE_PIN, PULSE_LVL);
    if( t < minPulseLen ) return samples;
    t = min( ( t - minPulseLen ) / resolution , bufSize-1 );
    ++ histogram[t]; 
    ++samples;
    *pMaxValue = max( *pMaxValue , histogram[t] );
  }
  
  return samples;
}

void printHistogram(byte* histogram, int bufSize, int resolution, int minPulseLen)
{
  for(int i=0;i<bufSize;i++)
  {
    unsigned long t = minPulseLen + ( i * resolution );
    if( histogram[i] != 0 )
    {
      unsigned long h = 100000;
      while( t<h ) { 
        Serial.print(' '); 
        h/=10; 
      }
      Serial.print(t);
      Serial.print(" : ");
      while( histogram[i] != 0 ) { 
        -- histogram[i]; 
        Serial.print("X"); 
      }
      Serial.println("");
    }
  }
}

void loop()
{
  byte histogram[BUF_SIZE];
  memset(histogram,0,BUF_SIZE); 
  unsigned int maxValue = 0;
  unsigned long samples = 0;
  unsigned long sequences = 0;

  Serial.println("SNIFFING ...");
  while( maxValue < 255 )
  {
    unsigned long seqSamples = pulseLenHistogram( histogram, &maxValue, BUF_SIZE, RESOLUTION, NOISE_THRESHOLD, MIN_VALID_SAMPLES );
    if( seqSamples >= MIN_VALID_SAMPLES )
    {
      samples += seqSamples;
      ++ sequences;
      Serial.print(maxValue);
      Serial.print(" ");
    }
  }
  if( samples >= MIN_VALID_SAMPLES )
  {
    Serial.print("collected ");
    Serial.print(samples);
    Serial.print(" samples in ");
    Serial.print(sequences);
    Serial.println(" sequences");
    printHistogram(histogram,BUF_SIZE,RESOLUTION,NOISE_THRESHOLD);
  }
}







