#include "Arduino.h"
#include <UnixTime.h>

#define MQTT_EOF                           0x1A
#define message  "+QGPSLOC: 204801.500,37.28331,127.04524,2.0,66.0,2,0.00,0.0,"

String bg96_rsp = "";
String str1 = "";

UnixTime stamp(9);

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
  Serial3.println("AT+QMTOPEN=0,\"-\",1883\r"); //MQTT 오픈
  // Serial3.println("AT+QIOPEN=1,0,\"TCP\",\"-\",6379\r"); //소켓 오픈
  delay(3000);
  Serial3.println("AT+QMTCONN=0,\"fuzen\"\r");
  delay(1000);

  flush();
  Serial.println("trial : 10");
}


void loop() {
  Serial3.println("AT+QGPSLOC=2\r");
  delay(2000);
  while(Serial3.available()){
    bg96_rsp += Serial3.readString();
  }
  bg96_rsp = "+QGPSLOC: 225722.0,37.37827,127.11290,1.4,88.0,2,143.07,0.0,0.0,230508,08"; //test
  int start_gps = bg96_rsp.indexOf("+QGPSLOC"); 
  int start_cme = bg96_rsp.indexOf("+CME");
  if(start_cme > -1){
    str1 = bg96_rsp.substring(start_cme, start_cme + 15);
  }
  if(start_gps > -1){
    str1 = "01,";
    str1 += get_timestamp();
    //76~81 -> date 22~27 -> utc maybe in real situation...
    str1 += bg96_rsp.substring(start_gps + 19, start_gps + 37); //lat, lng
  }
  Serial.println(str1);
  Pub();
  // send();
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

void send(){
  Serial3.print("AT+QISEND=0,30");
  Serial3.write("\r\n");
  delay(1000);
  Serial3.print("set bus01 ");
  Serial3.print(str1);
  Serial3.write("\r\n");
}

String get_timestamp(){
  flush();

  String clk_rsp = "";
  int year, month, day, hour, minute, second, tmp = 0;

  Serial3.print("AT+CCLK?");
  Serial3.write("\r\n");
  delay(500);
  while(Serial3.available()){
    clk_rsp += Serial3.readString();
  }
  int start_clk = clk_rsp.indexOf("+CCLK");
  if(start_clk > -1){
    clk_rsp = clk_rsp.substring(start_clk+7, clk_rsp.length());
    sscanf(clk_rsp.c_str(),"\"%d/%d/%d,%d:%d:%d+%d\"",&year, &month, &day, &hour, &minute, &second, &tmp);
  }
  year += 2000;
  stamp.setDateTime(year, month, day, hour, minute, second);

  uint32_t unix = stamp.getUnix() + 32400;

  return String(unix) + ",";
}

void flush(){ //flush Serial3 buffer
  while(Serial3.available()){
    Serial3.read();
  }
  delay(500);
}
