#include "Arduino.h"

#define MQTT_EOF                           0x1A
#define message  "+QGPSLOC: 204801.500,37.28331,127.04524,2.0,66.0,2,0.00,0.0,"

String bg96_rsp = "";
String str1 = "";

void setup() {
  bg96_rsp = "";

  LM5_off();
  LM5_on();

  Serial.begin(115200);

  Serial3.begin(115200); //Arduino Serial
  while(!Serial3);

  delay(6000);
  Serial3.println("ATE0\r");
  delay(500);
  Serial3.println("AT+QGPS=1\r");
  delay(500);
  Serial3.println("AT+QIACT=1\r");
  delay(500);
  Serial3.println("AT+QMTOPEN=0,\"ficons.iptime.org\",1883\r");
  delay(3000);
  Serial3.println("AT+QMTCONN=0,\"fuzen\"\r");
  delay(1000);

  flush();
  Serial.println("trial : 9");
}


void loop() {
  Serial3.println("AT+QGPSLOC=2\r");
  delay(2000);
  while(Serial3.available()){
    bg96_rsp += Serial3.readString();
  }
  int start_gps = bg96_rsp.indexOf("+QGPSLOC"); 
  int start_cme = bg96_rsp.indexOf("+CME");
  if(start_cme > -1){
    str1 = bg96_rsp.substring(start_cme, start_cme + 15);
  }
  if(start_gps > -1){
    str1 = bg96_rsp.substring(start_gps, start_gps + 48);
  }
  Serial.println(str1);
  Pub();
  str1, bg96_rsp = "";
  flush();
  delay(3000);
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

void Pub(){
  Serial3.print("AT+QMTPUB=0,0,0,0,\"gps\"");
  Serial3.write("\r\n");
  delay(1000);
  Serial3.println(str1);
  Serial3.write(MQTT_EOF);
}

void flush(){ //flush Serial3 buffer
  while(Serial3.available()){
    Serial3.read();
  }
  delay(500);
}
