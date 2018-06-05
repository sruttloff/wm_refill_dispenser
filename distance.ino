#define TRIG_PIN 26
#define ECHO_PIN 24
#define DISTANCE_PLUGIN_PIN 28
#define DISTANCE_MEASUREME_EACH 1
#define REFILL_PUMP 5
#define NEXTRUN_AFTER_CONTAINER_OUT 300
#define NEXTRUN_AFTER_ERROR 1
#define NEXTRUN_IF_NOT_CONNECTED 1
void refillPump();
int distanceMeasureTry = 0;
int distanceRoRaw();
uint32_t distanceNextRun = 0;
#define dist_max 35 // reservoir full
#define dist_min 232 // reservoir empty
#define fillStart 90 // start fill
#define fillEnd 75 // fill stop
#define fillTime (3*60)*1000 // time in seconds to fill water // 3minutes - ~250ml
int fillPump = 0; // status of fill pump
int fillBreak = 0; // break after fill for x seconds
String distanceLatestMsg = "";
const String fillFrom = "09:00:01";
const String fillTo = "19:33:00";
uint32_t distanceFunctionNextRun = 0;


void distanceRo() {   
  int distance;
  if (!fillPump){           
    if (distanceFunctionNextRun < currentUnixtime){
      distanceFunctionNextRun = currentUnixtime + 5; // measure each 5 seconds
    } else
      return;
    if (currentTimeStamp < timeStampFromTime(fillFrom) || currentTimeStamp > timeStampFromTime(fillTo)) {   
      Serial.println("Refill Ausserhalb der Zeit " + String(currentDateTime.hour()) + ":" + String(currentDateTime.minute()) + ":" + String(currentDateTime.second()));    
      distanceFunctionNextRun = currentUnixtime + (5*60); // come back after 5 minutes
      return;
    }
  }
  
  refillPump();
  // check if fillPump is on and timer is over
  float tmpT = myTimer(2);        
  if (fillPump && tmpT >= 100.00){
    fillPump = 0;
    refillPump();
    myTimer(2, -1); // reset pump Timer
    Serial.println("Refill timer out of time");    
    fillBreak = 1;   
    myTimer(2, (long) fillTime * 2);
    tmpT = 0;
  }
  if (fillBreak && tmpT >= 100.00){
    fillBreak = 0;    
    myTimer(2, -1); // reset pump Timer
    Serial.println("Refill break over");   
    String tmp = "";     
  }
  // check if device is connected
  if (!digitalRead(DISTANCE_PLUGIN_PIN)){    
    String tmp = "Distance Device not connected";
    if (distanceLatestMsg != tmp){      
      distanceLatestMsg = tmp;      
      Serial.println(distanceLatestMsg);   
    }
    // retry in 7 secs
    distanceNextRun = currentUnixtime + (NEXTRUN_IF_NOT_CONNECTED);
    distanceMeasureTry = 0;
    fillPump = 0;
    refillPump();
    return;
  }
  if (distanceNextRun > getTimeStamp())
    return;    
  if (distanceMeasureTry >=10){    
    String tmp = "Distance error";
    if (distanceLatestMsg != tmp){      
      distanceLatestMsg = tmp;      
      Serial.println(distanceLatestMsg + " " + String(distance) + "mm");   
    }
    buzzHigh();    
    distanceNextRun = currentUnixtime + (NEXTRUN_AFTER_ERROR);
    distanceMeasureTry = 0;
    buzzHigh();
    fillPump = 0;
    refillPump();
    return;
  }
  distance = distanceRoRaw();
  if (!distance){
    String tmp = "Distance read error";
    if (distanceLatestMsg != tmp){      
      distanceLatestMsg = tmp;      
      Serial.println(distanceLatestMsg);   
    }
    distanceMeasureTry++;
    fillPump = 0;
    refillPump();
    return;
  }      
  String tmp = String(distance) + "mm";
  if (distanceLatestMsg != tmp){      
    distanceLatestMsg = tmp;      
    Serial.println(distanceLatestMsg);   
  }
  // check if distance is inside countainer
  if (distance > dist_min || distance < dist_max)
  {
    String tmp = "Out of container";
    if (distanceLatestMsg != tmp){      
      distanceLatestMsg = tmp;      
      Serial.println(distanceLatestMsg + " " + String(distance) + "mm");   
    }
    fillPump = 0;
    refillPump();    
    distanceNextRun = currentUnixtime + (NEXTRUN_AFTER_CONTAINER_OUT);
    return;
  }
  
  distanceMeasureTry = 0;
  
  distanceNextRun = getTimeStamp() + DISTANCE_MEASUREME_EACH;
  if (!fillBreak && !fillPump && distance >= fillStart)
  {
    // start timer to stop fillpump after time 1 minute
    myTimer(2, (long) fillTime);
    fillPump = 1;
    Serial.println("Refill start");   
  }
  if (fillPump && distance <= fillEnd){
    // stop refill        
    fillPump = 0;    
    myTimer(2, -1); // reset pump Timer
    Serial.println("Refill stop");       
  }
  refillPump();
  return;
}
unsigned long distRawMillis = 0;
int tmpLastDistance = 0;
int distanceRoRaw() {  
    int duration;
    int distance;
    unsigned long tmpMillis = millis();
    if (tmpLastDistance && distRawMillis > tmpMillis)
    {
      // return last value
      return tmpLastDistance;
    }
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    if (duration && duration >= 0){
      distRawMillis += 200; // add ms
      tmpLastDistance = distance = (int) ((duration/2.0) / 2.91);      
      return distance;
    } else {
      String tmp = "Distance error of measurement";
      if (distanceLatestMsg != tmp){      
        distanceLatestMsg = tmp;      
        Serial.println(distanceLatestMsg);   
      }
      return false;
    }
}
void distanceRoSetup() {    
  pinMode(REFILL_PUMP, OUTPUT);
  digitalWrite(REFILL_PUMP, LOW);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);    
  pinMode(DISTANCE_PLUGIN_PIN, INPUT);    
  distanceNextRun = currentUnixtime + DISTANCE_MEASUREME_EACH;
}
void refillPump(){
  digitalWrite(REFILL_PUMP, fillPump);
}

