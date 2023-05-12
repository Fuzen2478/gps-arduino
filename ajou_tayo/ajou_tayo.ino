#include <TinyGPS.h>
#include <UnixTime.h>
#include "TYPE1SC.h"
#if !defined(__AVR_ATmega4809__)
#elif !defined(__AVR_ATmega2560__)
#else
#include <avr/dtostrf.h>
#endif

#define DebugSerial Serial
#define M1Serial Serial1
#define mySerial Serial2

TYPE1SC TYPE1SC(M1Serial, DebugSerial);
TinyGPS gps;
UnixTime stamp(9);

char IPAddr[32];
int _PORT = 2478;
char sckInfo[128];
char sendBuffer[1024] = "";
char* debugBuffer = "";

void setup() {
  // put your setup code here, to run once:
  M1Serial.begin(115200);
  DebugSerial.begin(115200);
  mySerial.begin(9600);

  DebugSerial.println("TYPE1SC Module Start!!!");
  /* Board Reset */
  TYPE1SC.reset();

  /* TYPE1SC Module Initialization */
  if (TYPE1SC.init()) {
    DebugSerial.println("TYPE1SC Module Error!!!");
  }

  /* Network Regsistraiton Check */
  while (TYPE1SC.canConnect() != 0) {
    DebugSerial.println("Network not Ready !!!");
    delay(2000);
  }

  DebugSerial.println("TYPE1SC Module Ready!!!");

  /* Enter a DNS address to get an IP address */
  while (1) {

    if (TYPE1SC.getIPAddr("ficons.iptime.org", IPAddr,
                          sizeof(IPAddr)) == 0) {
      DebugSerial.print("IP Address : ");
      DebugSerial.println(IPAddr);
      break;
    } else {
      DebugSerial.println("IP Address Error!!!");
    }
    delay(2000);
  }

  /* 1 :TCP Socket Create ( 0:UDP, 1:TCP ) */
  if (TYPE1SC.socketCreate(1, IPAddr, _PORT) == 0)
    DebugSerial.println("TCP Socket Create!!!");

INFO:

  /* 2 :TCP Socket Activation */
  if (TYPE1SC.socketActivate() == 0)
    DebugSerial.println("TCP Socket Activation!!!");

  if (TYPE1SC.socketInfo(sckInfo, sizeof(sckInfo)) == 0) {
    DebugSerial.print("Socket Info : ");
    DebugSerial.println(sckInfo);

    if (strcmp(sckInfo, "ACTIVATED")) {
      delay(3000);
      goto INFO;
    }
  }

  // if (TYPE1SC.setGPS(1, debugBuffer) == 0)
  //   DebugSerial.println("GPS Set Complete");
  // else DebugSerial.println(debugBuffer);
}

void loop() {
  delay(5000);
  String gpsStr = getgps(gps);
  gpsStr.toCharArray(sendBuffer, gpsStr.length());
  
  if (TYPE1SC.socketSend(sendBuffer) == 0) {
    DebugSerial.print("[Send] >>  ");
    DebugSerial.println(sendBuffer);
  } else
    DebugSerial.println("Send Fail!!!");
}

String getgps(TinyGPS &gps){
  float lat, lon;
  int _year, _month, _day, _hour, _minute, _second, _tmp;

  lat = 0.0;
  lon = 0.0;

  if (mySerial.available()) {
    int c = mySerial.read();
    if (gps.encode(c)){
      gps.f_get_position(&lat, &lon);
    }
  }

  char szTime[32];
  if (TYPE1SC.getCCLK(szTime, sizeof(szTime)) == 0) {
    sscanf(szTime,"\"%d/%d/%d,%d:%d:%d+%d\"",&_year, &_month, &_day, &_hour, &_minute, &_second, &_tmp);
  }    
  //Set Date Time
  _year += 2000;
  stamp.setDateTime(_year, _month, _day, _hour, _minute, _second);   // year/month/day,hour:minute:second

  //Get Unix Time 
  uint32_t unix = stamp.getUnix();
  return "04," + String(unix) + "," + String(lat) + "," + String(lon);
}
