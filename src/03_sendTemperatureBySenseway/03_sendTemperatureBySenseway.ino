// ADT7410温度送信用スケッチ for Kiwi LoRa Module TLM922S
//            for Arduino / Kiwi LoRa module TLM922S
//            Ver 1.2
//
// Copyright(c) 2018 SenseWay, All rights reserved.
//
#include <SoftwareSerial.h>
#include <Wire.h>

#define LoRa_RX_PIN 11 // Arduino D11 to LoRa module TX
#define LoRa_TX_PIN 12 // Arduino D12 to LoRa module RX

#define SERIAL_WAIT_TIME 6000
#define LoRa_SEND_INTERVAL 30000 // LoRa送信間隔 (ミリ秒)

#define LoRa_fPort_TEMP 12 // port 12 = 温度湿度気圧等

SoftwareSerial LoRa(LoRa_RX_PIN, LoRa_TX_PIN);

unsigned long beforetime = 0L;

void setup() {
  // for console serial
  Serial.begin(9600);
  Serial.println("LoRa ADT7410 Temperature Send for KiwiTech");

  // for LoRa module serial
  LoRa.begin(9600);
  LoRa.setTimeout(SERIAL_WAIT_TIME);

  // for ADT7410 I2C initialize
  Wire.begin();

  // send initialize command to LoRa
  initLoRa(SERIAL_WAIT_TIME);

  // beforetime = -(LoRa_SEND_INTERVAL);
  beforetime = millis();
}

void loop() {
  if (millis() - beforetime > LoRa_SEND_INTERVAL) {
    Serial.println("\nAuto mode - send");

    sendTemp();
    beforetime = millis();
  }
}

void rxFlushLoRa(void) {
  delay(SERIAL_WAIT_TIME);
  // print any response data before send
  while (LoRa.available() > 0) {
    char ch = LoRa.read();
    Serial.print(ch);
  }
}

//
// void initLoRa(int waitTime)
//    send command strings to initialize/ready LoRa module
//        waitTime - waiting timeout for command result
//
void initLoRa(int waitTime) {
  // activate LoRa serial
  LoRa.listen();

  // send dummy enter
  sendCmd("\n", false, "", waitTime);
  rxFlushLoRa();

  //
  // LoRa module status clear
  //
  if (!sendCmd("mod reset", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  rxFlushLoRa();
  if (!sendCmd("mod set_echo off", false, "Ok", waitTime)) {
    Serial.println("Request Failed");
  }
  //
  // LoRa module various value get
  //
  if (!sendCmd("mod get_hw_model", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("mod get_ver", true, "", waitTime)) {
    Serial.println("Request Failed");
  }
  if (!sendCmd("lorawan get_deveui", true, "", waitTime)) {
    Serial.println("Request Failed");
  }

  //
  // LoRa module join to Network Server by OTAA
  //
  while (!sendCmd("lorawan join otaa", true, "accepted", waitTime * 2)) {
    Serial.println("Request Failed");
    // forever loop until join success
  }
}

//
// bool sendCmd(String cmd, bool echo, String waitStr, int waitTime)
//    send command string to LoRa module and wait result
//        cmd - command string to send LoRa module
//        echo - command local echo back on/off
//        waitStr - waiting string expected for command result
//        waitTime - waiting timeout for command result
//
bool sendCmd(String cmd, bool echo, String waitStr, int waitTime) {
  unsigned long tim;
  String str;

  if (echo) {
    Serial.print(cmd);
  }

  LoRa.listen();

  LoRa.print(cmd);
  LoRa.print('\r');
  LoRa.flush();

  tim = millis() + waitTime;

  while (millis() < tim) {
    if (LoRa.available() > 0) {
      char ch = LoRa.read();
      Serial.print(ch);
      str += String(ch);
      if (str.indexOf("\r> ") >= 0)
        break;
    }
  }
  if (waitStr == NULL)
    return true;
  if (str.indexOf(waitStr) >= 0)
    return true;

  return false;
}

//
// bool sendCmd2(String cmd, bool echo, String waitStr, String waitStr2, int
// waitTime)
//    send command string to LoRa module and wait result
//        cmd - command string to send LoRa module
//        echo - command local echo back on/off
//        waitStr, waitStr2 - waiting either strings expected for command result
//        waitTime - waiting timeout for command result
//
bool sendCmd2(String cmd, bool echo, String waitStr, String waitStr2,
              int waitTime) {
  unsigned long tim;
  String str;
  int in_byte;

  if (echo) {
    Serial.print(cmd);
  }

  LoRa.listen();

  LoRa.print(cmd);
  LoRa.print('\r');
  LoRa.flush();

  tim = millis() + waitTime;

  while (millis() < tim) {
    if (LoRa.available() > 0) {
      char ch = LoRa.read();
      Serial.print(ch);
      str += String(ch);
      if (str.indexOf("\r> ") >= 0)
        break;
    }
  }
  if (waitStr == NULL)
    return true;
  if (str.indexOf(waitStr) >= 0 || str.indexOf(waitStr2) >= 0)
    return true;

  return false;
}

//
// 温度を LoRa送信する
//
bool sendTemp() {
  char cmdline[64];
  short port = LoRa_fPort_TEMP; // port 12 = Temperature

  Wire.requestFrom(0x48, 2);
  uint16_t val = Wire.read() << 8;
  val |= Wire.read();
  val >>= 3; // convert to 13bit format
  int ival = (int)val;
  if (val & (0x8000 >> 3)) {
    ival -= 8192;
  }
  float temp = (float)ival / 16.0;
  char buf[6];
  Serial.print(F("Temp = "));
  Serial.print(dtostrf(temp, 3, 2, buf));
  Serial.println(F(" degrees C"));

  // (小数点以下2桁まで有効にするため)100倍して整数と見なし、16進数変換して送信する。
  sprintf(cmdline, "lorawan tx ucnf %d %04x", port, (int)(temp * 100));
  if (!sendCmd2(cmdline, true, "tx_ok", "rx", SERIAL_WAIT_TIME)) {
    Serial.println("Request Failed");
    return false;
  }
  return true;
}
