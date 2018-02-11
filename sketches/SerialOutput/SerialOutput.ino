
int n = 0;

void setup()
{
  Serial.begin(19200);
}

void loop()
{
	n = ( n + 1 ) % 1024;
  float x = n*0.234f;
  float y = n*0.874f;
  float z = n*0.531f;
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);
}

