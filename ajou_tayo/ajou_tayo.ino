#include "Arduino.h"

#define RESP_OK                            "OK\r\n"
#define RET_OK                             1
#define RET_NOK                            -1
#define ON                                 1
#define OFF                                0

#define BG96_PARSER_DELIMITER              "\r\n"

#define MQTT_EOF                           0x1A

String bg96_rsp = "";
char gps[100];
char* a = "";

void setup() {
  LM5_off();
  LM5_on();

  Serial1.begin(115200); //Cat.M1 Serial
  while(!Serial1);

  while(!WaitCatM1()); //Wait Cat.M1 Ready
  
  while(!Setting("ATE0\r", "OK")); //Set Echo Off
  Pub("ATE0 OK");

  while(!Setting("AT+QGPS=2\r", "OK")); //Set GPS On
  Pub("SET GPS OK");

  while(!Setting("AT+QIACT=1\r", "OK")); //Set PDP Context on
  Pub("SET PDP Context OK");

  while(!Setting("AT+QMTOPEN=0,\"ficons.iptime.org\",1883\r", "+QMTOPEN: 0,0"));
  Pub("MQTT Socket Open OK");
  //Set MQTT Socket Open

  while(!Setting("AT+QMTCONN=0,\"fuzen\"\r", "+QMTCONN: 0,0,0"));
  Pub("MQTT Socket Connect OK");
  //Set MQTT Socket Connect
}


void loop() {
  Serial1.println("AT+QGPSLOC=2\r");  //Get Gps Location
  delay(2000);
  while(Serial1.available()){
    bg96_rsp += Serial1.ReadStringUntil("\n");
    bg96_rsp.trim();
    delay(2);
  }
  if(bg96_rsp.indexOf("+C") > -1){
    Pub("Not fixed");
  }else if(bg96_rsp.indexOf("+QGPSLOC") > 0){ //Need to Update
    bg96_rsp.toCharArray(a, bg96_rsp.length());
    Pub(a);
  }else{
    bg96_rsp.toCharArray(a, bg96_rsp.length());
    Pub("Test");
    Pub(a);
  }
  delay(8000);
  bg96_rsp = "";
  *a = "";
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
  Serial1.println("AT+QMTPUB=0,0,0,0,\"gps\"\r");
  delay(100);
  Serial1.println(payload);
  Serial1.write(MQTT_EOF);
  Serial1.write("\r\n");

  Serial1.ReadStringUntil("\n");  //flush OK
  Serial1.ReadStringUntil("\n");  //flush \n
  Serial1.ReadStringUntil("\n");  //flush +QMTPUB: 0,0,0
  Serial1.ReadStringUntil("\n");  //flush \n
}

bool WaitCatM1(){
  while(Serial1.available()){
      bg96_rsp += char(Serial1.read());
      delay(2);
    }
    if(bg96_rsp.indexOf("SMS DONE") > 0){
      Serial.println("Cat.M1 Ready");
      Serial1.flush();
      bg96_rsp = "";
      return true;
    }
}

bool Setting(char* payload, char* condition){
  Serial1.println(payload);
    delay(100);
    while(Serial1.available()){
      bg96_rsp += char(Serial1.read());
      delay(2);
    }
    if(bg96_rsp.indexOf(condition) > 0){
      Serial1.flush();
      bg96_rsp = "";
      return true;
    }
}

