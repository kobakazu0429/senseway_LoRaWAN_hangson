#include <SoftwareSerial.h>
#include <Wire.h>

#define PORT_TEMP 12

#define INTERVAL 2000
int i = 0;

void setup()
{
  Wire.begin(); // I2C initialize

  Serial.begin(57600);
  Serial.println("Count Up Start!");
}

void loop()
{
  i++;
  Serial.println(i);
  sendTemp();
  delay(INTERVAL);
}

//温度センサADT7410で温度を取得し送信する関数
void sendTemp()
{
  char cmdline[64];
  short port = PORT_TEMP;

  Wire.requestFrom(0x48, 2);
  uint16_t val = Wire.read() << 8;
  val |= Wire.read();
  val >>= 3; // convert to 13bit format
  int i_val = (int)val;
  if (val & (0x8000 >> 3))
  {
    i_val -= 8192;
  }
  float temp = (float)i_val / 16.0;
  char buf[6];
  Serial.print(F("Temp = "));
  dtostrf(temp, 3, 2, buf);
  Serial.print(buf);
  Serial.println(F(" degrees C"));
}
