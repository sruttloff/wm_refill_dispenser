// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
uint32_t getTimeStamp();
uint32_t Hour, Minute, Second ;
uint32_t timeStampFromTime(String str);
String timeFromTimeStamp (uint32_t timestamp);
DateTime currentDateTime;
uint32_t currentTimeStamp;
uint32_t currentUnixtime;

void rtcSetup () {  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  readTimeFromRTC();
}
uint32_t getTimeStamp(){
  DateTime now = rtc.now();  
  return now.unixtime();
}
void testTime(){
  Serial.println(getTimeStamp());
}
void test1 () {
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
    
    Serial.println();
    delay(3000);
}
uint32_t timeStampFromTime(String str)
{
  char charBuf[50];
  str.toCharArray(charBuf,10);
  sscanf(charBuf, "%d:%d:%d", &Hour, &Minute, &Second);  
  uint32_t tmp = (Hour * 3600) + (Minute * 60) + Second;
  // Serial.println(tmp);
  return tmp;  
}
String timeFromTimeStamp (uint32_t timestamp){  
  return String(String((timestamp / 3600) % 24) + ":" + String((timestamp / 60) % 60) + ":" + String(timestamp % 60));
}
void readTimeFromRTC(){
  // check each second
  float tmpT = myTimer(3);       
  if (myTimer(3) == 0.0) {
    myTimer(3, 1000);
  } 
  if (tmpT < 100.00)    
    return;
  myTimer(3, -1); // reset pump Timer    
  
  DateTime now = rtc.now();
  uint32_t tmpTime = ((uint32_t) now.hour() * 3600) + ((uint32_t) now.minute() * 60) + (uint32_t) now.second();  
  if (!tmpTime || tmpTime > 86400) 
    return;
  currentTimeStamp = tmpTime;
  currentDateTime = now;
  currentUnixtime = now.unixtime();
  // Serial.println(String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));   
}

