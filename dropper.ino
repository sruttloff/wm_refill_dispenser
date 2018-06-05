
#define DROPPER_PUMP 4
#define dropperTime 110 // time in milliseconds to drop
#define DROP_TIMER_QTY 4
const char* dropTimes[] = {"03:30:00", "09:30:00", "15:30:00", "21:30:00"};
uint32_t dropperNextRun = 0;
int dropperRun = 0;
String dropperLatestMsg = "";

void dropper() {
  // skip if fillpump runs
  if (!dropperRun && fillPump || !currentTimeStamp)
    return;  
  // check if dropper Pump is on and timer is over
  float tmpT = myTimer(4);        
  if (dropperRun && tmpT >= 100.00){
    dropperRun = 0;
    dropperPump(); 
    myTimer(4, -1); // reset dropper Timer
    Serial.println("Dropper timer out of time");    
    dropperNextRun = 0;
  }
  if (dropperNextRun == 0){
    // init    
    for (int i=0;i<DROP_TIMER_QTY; i++){
      // Serial.println(dropTimes[i]);   
      uint32_t tmp = timeStampFromTime(String(dropTimes[i]));
      // Serial.println("Droptime Compare: " + String(tmp) + " :: " + String(currentTimeStamp));   
      if (tmp > currentTimeStamp){
        dropperNextRun = timeStampFromTime(String(dropTimes[i]));        
        break;
      }
    }
    String tmp = "Dropper Next Run: " + String(dropperNextRun) + " = " + timeFromTimeStamp(dropperNextRun);
    if (dropperLatestMsg != tmp){      
      dropperLatestMsg = tmp;      
      Serial.println(dropperLatestMsg);   
    }       
  }  
  if (!dropperRun && dropperNextRun != 0 && dropperNextRun <= currentTimeStamp){
    // dropper should start
    myTimer(4, (long) dropperTime);
    dropperRun = 1;    
  }
  dropperPump();  
}
void dropperPump(){
  digitalWrite(DROPPER_PUMP, dropperRun);
}
void dropperSetup() {    
  pinMode(DROPPER_PUMP, OUTPUT);
  digitalWrite(DROPPER_PUMP, LOW);
}
