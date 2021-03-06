/*
  Jebao SW Wavemaker Controller with Arduino Mega

  Does controll 2 Wavemaker controller (0-10V DC). One is master, the other is slave and runs reversed against No.1
  Food Button can be connected optionally.  

  Free for all!
 */

#include <Arduino.h>
extern HardwareSerial Serial;

const int pump1 = 2; // the PWM pin the right pump is attached to
const int pump2 = 3; // the PWM pin the left pump is attached to
const int pump1Min = 45; //45 ist der kleinste
const int pump1Max = 255;
const int pump2Min = 45;
const int pump2Max = 255;
int pump1Current = 0, pump2Current = 0;
const int t1 = 1; // time in secs. to stay at level min
const int t2 = 1; // time in secs. to stay at max
const int t3 = 35; // time in secs. to go from min to max
const int t4 = 35; // time in secs. to go from max to min
const int relaxPump1 = 180;
const int relaxPump2 = 180;
int pump1MaxTmp = 0;
int pump2MaxTmp = 0;
/*
 * timer 1 = for wavemaker timing
 * timer 2 = fill pump RO water
 * timer 3 = rtc read
 * timer 4 = dropper timer
 * timer 5 = food timer
 */
const int timerElements = 5; // qty of timer's to init
unsigned long timerArray[timerElements + 1];
unsigned long tempTime[timerElements + 1]; // needed to calculate progress
String step = "t1"; // step marker
float myTimer(int timerNo = 0, long time = 0);
void setPump1Speed(float rate);
void setPump2Speed(float rate);
void pumpMaster();
void buzzHigh();
int pumpClean = 0;
unsigned long pumpCleanTimer = 0;
bool foodBreak = 0;
long foodTime = 600; // food time in secs.
const int foodSwitch = 22; // food switch pin
int tmpSwitch = 0;
float tmpFood = 0;
const int buzzer = 30;//der Pins des angesteckten Summers

void setup() {    
    pump1MaxTmp = pump1Max;
    pump2MaxTmp = pump2Max;
    rtcSetup();
    pinMode(pump1, pump1Current);
    pinMode(pump2, pump2Current);
    Serial.begin(115200);
    // init timer array  
    for (int i = 0; i < timerElements; i++) {
        timerArray[i] = 0;
        tempTime[i] = 0;
    }
    pinMode(foodSwitch, INPUT);
    digitalWrite(foodSwitch, HIGH);
    setPump1Speed(50.0);
    wifiSetup();
    distanceRoSetup();
    pinMode(buzzer,OUTPUT);// initialsiert den buzzer als Output
    readTimeFromRTC();
    dropperSetup();
    temperatureSetup();
}
void foodbreakToggle(){
  if (foodBreak == 1) {
      // make timer run out of time      
      myTimer(5, (long) 1);
  } else {
      if (pumpClean)
        return;
      foodBreak = 1;
      analogWrite(pump1, 0);
      analogWrite(pump2, 0);
      // start food timer      
      myTimer(5, (long) foodTime * (long) 1000);
      Serial.println("Web-Trigger Pressed and released " + String((long) foodTime * (long) 1000));
      pump1Current = 0;
      pump2Current = 0;
  }
}
void pumpCleaningToggle(){
  if (foodBreak)
    return;
  if (pumpClean) {
      pumpClean = 0;
      step = "t1";      
      myTimer(1, -1); // reset pump Timer
      setPump1Speed(50.0);
      delay(700);
      Serial.println("Cleaning end");
  } else {
      pumpClean = 1;
      analogWrite(pump1, 255);
      analogWrite(pump2, 0);
      pump1Current = 255;
      pump2Current = 0;     
      pumpCleanTimer = millis() + 1000;       
  }
}
void pumpCleaningAction(){
  if (!pumpClean)
    return;
    if (pumpCleanTimer > millis())
      return;
    pumpClean++;
    if (pumpClean == 100){
      pumpCleaningToggle();
      return;
    }
    Serial.print("Zyklus " +  String(pumpClean) + ": ");
    if (pump1Current){
      analogWrite(pump1, 0);
      analogWrite(pump2, 255);
      pump1Current = 0;
      pump2Current = 255;     
      Serial.println("Cleaning Pump 2");
    } else {
      analogWrite(pump1, 255);
      analogWrite(pump2, 0);
      pump1Current = 255;
      pump2Current = 0;     
      Serial.println("Cleaning Pump 1");
    }    
    pumpCleanTimer = millis() + (750 - (long) (3 * pumpClean));      
}
void foodSwitchAction() {
    int digitalVal = digitalRead(foodSwitch);
    // switch pressed
    if (!tmpSwitch && HIGH == digitalVal)
        tmpSwitch = 1;
    // switch released
    if (tmpSwitch && LOW == digitalVal) {
        tmpSwitch = 0;
        if (foodBreak == 1) {
            // make timer run out of time
            Serial.println("Switch Pressed and released to stop timer");
            myTimer(5, (long) 1);
        } else {
            foodBreak = 1;
            analogWrite(pump1, 0);
            analogWrite(pump2, 0);
            // start food timer            
            Serial.println("Switch Pressed and released " + String((long) foodTime * (long) 1000));
            myTimer(5, (long) foodTime * (long) 1000);
        }
    }
    if (foodBreak == 1) {
        float tmp = myTimer(5);
        if (tmpFood < tmp) {
            tmpFood = ceil(tmp);
            Serial.println("Food Timer at " + String(tmpFood) + "%");
        }
        // timer reached      
        if (tmp >= 100.00) {
            tmpFood = foodBreak = pump1Current = pump2Current = 0;
            step = "t1";
            myTimer(5, -1); // reset Timer
            myTimer(1, -1); // reset pump Timer
            setPump1Speed(50.0);
            delay(700);
            Serial.println("Food end");
        }
    }
}

void loop() {
    readTimeFromRTC();
    if (!foodBreak && !pumpClean)
        pumpMaster();
    foodSwitchAction();
    pumpCleaningAction();
    distanceRo();
    dropper();
    temperatureCheck();
    processWifi();
    //testTime();
    //test();   
}
void setPump1Speed(float rate) {
    // rate is percent
    int tmpRange = pump1MaxTmp - pump1Min;
    if (rate < 0.00)
        rate = 0.00;
    if (rate > 100.00)
        rate = 100.00;
    int speed = (float) pump1Min + (tmpRange * (rate / 100.00));
    // set new speed when not already set
    if ((int) speed != pump1Current) {
        //    if (rate <= 2.00 || rate >= 98.00)
        //      Serial.println("Pump 1 at Rate: " + String(rate) + "% Value: " + String((int)speed));
        pump1Current = (int) speed;
        analogWrite(pump1, pump1Current);
        setPump2Speed((float) 100.00 - rate);
    }
}

void setPump2Speed(float rate) {
    // rate is percent
    int tmpRange = pump2MaxTmp - pump2Min;
    if (rate < 0.00)
        rate = 0.00;
    if (rate > 100.00)
        rate = 100.00;
    int speed = (float) pump2Min + (tmpRange * (rate / 100.00));
    // set new speed when not already set
    if ((int) speed != pump2Current) {
        //    if (rate <= 1.00 || rate >= 99.00)
        //      Serial.println("Pump 1 at Rate: " + String(rate) + " Value: " + String((int)speed));
        pump2Current = (int) speed;
        analogWrite(pump2, pump2Current);
    }
}
uint32_t currentTimeStamp;
uint32_t currentUnixtime;
uint32_t timeStampFromTime(String str);
uint32_t pumpShuffleNextRun = 0;
bool pumpShuffle = 0;
bool pumpRelaxMode = 0;
const String relaxFrom = "22:00:00";
const String relaxTo = "9:00:00";
void pumpMaster() {
  if (currentTimeStamp > timeStampFromTime(relaxFrom) || currentTimeStamp < timeStampFromTime(relaxTo)) {   
    if (pump1MaxTmp != relaxPump1)
      Serial.println("Pumpen Relax-Zeit ein");    
    pump1MaxTmp = relaxPump1;
    pump2MaxTmp = relaxPump2;
    pumpRelaxMode = 1;
  }
  else
  {
    pumpRelaxMode = 0;
    if (!pumpShuffle && pump1MaxTmp != pump1Max)
      Serial.println("Pumpen Relax-Zeit aus");    
    if (pumpShuffleNextRun < currentUnixtime){      
      if (pumpShuffle){
        pumpShuffle = 0;
        pump1MaxTmp = pump1Max;
        pump2MaxTmp = pump2Max;
        Serial.println("Pumpen Zufall aus");  
        pumpShuffleNextRun = currentUnixtime + 60; // each X seconds  
      } else {
        pumpShuffle = 1;
        // set a speed between relax and max
        pump1MaxTmp = random(relaxPump1, pump1Max);
        pump2MaxTmp = random(relaxPump2, pump2Max);;
        Serial.println("Pumpen Zufall ein: Pumpe 1(" + String(pump1MaxTmp) + ") Pumpe 2(" + String(pump2MaxTmp) + ")" );    
        pumpShuffleNextRun = currentUnixtime + 120; // each X seconds
      }
    } 
  }
    // go to min
    if (step == "t4") {
        if (myTimer(1) == 0.0) {
            Serial.println("Power Head: Decrease to min");
            myTimer(1, t4 * 1000L);
        }
        // set pump speed            
        float tmp = myTimer(1);
        float speed = 100.00 - tmp;
        setPump1Speed(speed);
        if (tmp >= 100.00) {
            setPump1Speed(0.00);
            step = "t1";
            Serial.println("Power Head: Decrease to min finished");
            myTimer(1, -1); // reset Timer
        }
    }
    // go to max
    if (step == "t3") {
        if (myTimer(1) == 0.0) {
            Serial.println("Power Head: Init Grow to max");
            myTimer(1, t3 * 1000L);
        }
        // set pump speed
        float speed = myTimer(1);
        setPump1Speed(speed);
        //Serial.println("Speed: " + String(speed) + " - " + String(speed * log10(speed)) + " test = " + String((speed * log10(speed)) - speed));
        if (myTimer(1) >= 100.00) {
            setPump1Speed(100.00);
            step = "t2";
            Serial.println("Power Head: Grow to max finished");
            myTimer(1, -1); // reset Timer
        }
    }
    // keep at min
    if (step == "t1") {
        //Serial.println("myTimer(1)=" + String(myTimer(1)));   
        if (myTimer(1) == 0.0) {
            Serial.println("");
            Serial.println("");
            Serial.println("Power Head: Stay at min");
            myTimer(1, t1 * 1000);
            setPump1Speed(0.00);
        }
        // timer reached      
        if (myTimer(1) >= 100.00) {
            step = "t3"; // t3
            myTimer(1, -1); // reset Timer
        }
    }
    // keep at max
    if (step == "t2") {
        if (myTimer(1) == 0.0) {
            Serial.println("Power Head: Stay at max");
            myTimer(1, t2 * 1000);
            setPump1Speed(100.00);
        }
        // timer reached      
        if (myTimer(1) >= 100.00) {
            step = "t4";
            myTimer(1, -1); // reset Timer
        }
    }
}

// returns a percent value of time thats over or false if timer is run out
float myTimer(int timerNo, long time) {
    unsigned long tmpMillis = millis();
    // Serial.println("Timer " + String(timerNo) + " to time = " + String(time));
    if (time == (long) - 1) {
        timerArray[timerNo] = 0;
        tempTime[timerNo] = 0;
//        Serial.println("Reset Timer " + String(timerNo) + " to time = " + String(time) + " Result in timerArray[timerNo] = " + String(timerArray[timerNo]));
    }
    if (time > (long) 0) {
        // 250+10=5;
        timerArray[timerNo] = tmpMillis + time;
        tempTime[timerNo] = time;
        // Serial.println("Init Timer " + String(timerNo) + " to " + String(time) + " Result in timerArray[timerNo] = " + String(timerArray[timerNo]));
    }
    if (tempTime[timerNo] <= 0){
//      if (timerNo == 1)
//        Serial.println("timerNo.: " + String(timerNo) + "  tempTime[timerNo]=" + String(tempTime[timerNo]));
      return 0.0;
    }
    // 5-251=1;5-252=2;5-253=3;5-4=1
    unsigned long diff = (timerArray[timerNo] - tmpMillis);
    //   Serial.println("diff " + String(diff) + " = " + "(" + String(timerArray[timerNo]) + " - " + String(tmpMillis) + ")");
    // 100/5*1=20;100/5*2=40;100/5*3=60;100/5*1=20
    float progress = float(100) - ((float) 100 / (float) tempTime[timerNo] * (float) diff);
    //Serial.println("Progress " + String(progress) + "%");
    if (timerArray[timerNo] > 0 && timerArray[timerNo] <= tmpMillis) {
        //Serial.println("Timer " + String(timerNo) + " is run out because: " + String(timerArray[timerNo]) + " <= " + String(tmpMillis));        
        return 100.00;
    } else
        return progress == 0.0 ? 0.01 : progress;
}

void buzzHigh(){
  digitalWrite(buzzer,HIGH);
  delay(15);//wartet 1ms
  digitalWrite(buzzer,LOW);
  delay(50);//wartet 1ms
}

