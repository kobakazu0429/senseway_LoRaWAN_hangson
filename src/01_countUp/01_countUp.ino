#include <SoftwareSerial.h>

#define INTERVAL 2000
int i = 0;

void setup()
{
  Serial.begin(57600);
  Serial.println("Count Up Start!");
}

void loop()
{
  i++;
  Serial.println(i);
  delay(INTERVAL);
}
