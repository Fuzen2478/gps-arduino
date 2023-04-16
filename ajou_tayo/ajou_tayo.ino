#include "Arduino.h"

#define MQTT_EOF                           0x1A
#define message  "+QGPSLOC: 204801.500,37.28331,127.04524,2.0,66.0,2,0.00,0.0,"

String bg96_rsp = "";
char gps[500];

void setup() {
  bg96_rsp = "";

  LM5_off();
  LM5_on();

  // Serial.begin(115200);

  Serial3.begin(115200); //Arduino Serial
  while(!Serial3);
  Serial3.setTimeout(4);

  delay(6000);
  Serial3.println("ATE0\r");
  delay(500);
  Serial3.println("AT+QGPS=1\r");
  delay(500);
  Serial3.println("AT+QIACT=1\r");
  delay(500);
  Serial3.println("AT+QMTOPEN=0,\"ficons.iptime.org\",1883\r");
  delay(1000);
  Serial3.println("AT+QMTCONN=0,\"fuzen\"\r");
  delay(500);

  delay(1000);
}


void loop() {
  Serial3.println("AT+QGPSLOC=2\r");
  while(Serial3.available()){
    bg96_rsp = Serial3.readString();
    int start = bg96_rsp.indexOf("+QGPSLOC");
    bg96_rsp.toCharArray(gps, bg96_rsp.length());
    // Serial.println(gps);
    if(start > -1){
      Pub(gps);
      break;
    }
    Serial3.flush();
  }
  bg96_rsp = "";
  delay(5000);
}

void LM5_off(){
  analogWrite(11, 0);
  delay(700);

  analogWrite(11, 255);
  delay(2100);

  analogWrite(9, 0);
  //turn off LM5

  delay(10000);
}

void LM5_on(){
  delay(50);

  analogWrite(11, 0);
  analogWrite(9, 0);
  delay(110);

  analogWrite(11, 255);

  delay(4800);

  analogWrite(9, 255);
  //turn on LM5
}

void Pub(char* payload){
  Serial3.print("AT+QMTPUB=0,0,0,0,\"gps\"");
  Serial3.write("\r\n");
  Serial3.print(payload);
  Serial3.write(MQTT_EOF);
}
