#include "Arduino.h"
#include <UnixTime.h>

#define MQTT_EOF                           0x1A

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

  flush();

  Serial3.println("ATE0\r");
  while(find_text("OK") == -1);
  Serial3.println("AT+QGPS=1\r");
  while(find_text("OK") == -1);
  Serial3.println("AT+QIACT=1\r");
  while(find_text("OK") == -1);
  // Serial3.println("AT+QMTOPEN=0,\"addr\",1883\r"); //MQTT 오픈
  Serial3.println("AT+QIOPEN=1,0,\"TCP\",\"addr\",2478\r"); //소켓 오픈
  while(find_text("0,0") == -1);
  // Serial3.println("AT+QMTCONN=0,\"fuzen\"\r");

  Serial.println("tcp_socket test");
}


void loop() {
  unsigned long start_loop = millis();
  int start_gps, start_cme = -1;
  str1 = "01,";
  str1 += get_timestamp();
  Serial3.println("AT+QGPSLOC=2\r");
  start_gps = find_text("+QGPSLOC"); 
  start_cme = find_text("+CME");
  if(start_cme > -1){
    str1 += bg96_rsp.substring(start_cme, start_cme + 15);
  }
  else if(start_gps > -1){
    //76~81 -> date 22~27 -> utc maybe in real situation...
    str1 += bg96_rsp.substring(start_gps + 19, start_gps + 37); //lat, lng
  }else {
    str1 += "something strange";
  }
  while(send() == 1);
  flush();
  while(start_loop + 10000 > millis());
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
  
  delay(2000);
  //turn on LM5
}

void Pub(){
  Serial3.print("AT+QMTPUB=0,0,0,0,\"gps\"");
  Serial3.write("\r\n");
  delay(1000);
  Serial3.println(str1);
  Serial3.write(MQTT_EOF);
}

int send(){
  Serial3.print("AT+QISEND=0,"+ String(str1.length()));
  Serial3.write("\r\n");
  delay(1000);
  Serial3.print(str1);
  Serial3.write("\r\n");
  Serial.println(str1);
  if(find_text("SEND OK") == -1){
    Serial3.println("AT+QIOPEN=1,0,\"TCP\",\"addr\",2478\r");
    while(find_text("0,0") == -1);
    return 1;
  }
  return -1;
}

String get_timestamp(){
  int year, month, day, hour, minute, second, tmp = 0;

  Serial3.println("AT+CCLK?\r");
  int start_clk = find_text("+CCLK");
  if(start_clk > -1){
    bg96_rsp = bg96_rsp.substring(start_clk+7, start_clk+29);
    sscanf(bg96_rsp.c_str(),"\"%d/%d/%d,%d:%d:%d+%d\"",&year, &month, &day, &hour, &minute, &second, &tmp);
    year += 2000;
    stamp.setDateTime(year, month, day, hour, minute, second);
    uint32_t unix = stamp.getUnix() + 32400;
    return String(unix) + ",";
  }
  else return "time error";
}

void flush(){ //flush Serial3 buffer
  unsigned long called = millis();
  while(called + 500 > millis()){
    Serial3.read();
  }
  bg96_rsp = "";
}

long find_text(String text){
  unsigned long called = millis();
  while(called + 2000 > millis()){
    bg96_rsp += Serial3.readString();
    int find = bg96_rsp.indexOf(text);
    if(find > -1){
      if((text.indexOf("+")) < 0){
        flush();
      }
      return find;
    }
  }
  return -1;
}
