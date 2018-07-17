// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>
RCSwitch fanSwitch = RCSwitch();
#define FAN_TX_PIN 34

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 32

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
float getTemperature();
unsigned int temperatureFunctionNextRun = 0;
bool fanstate = 0;
/*
 * The setup function. We only start the sensors here
 */
void temperatureSetup(void)
{    
  sensors.begin();
  pinMode(FAN_TX_PIN, OUTPUT);
  fanSwitch.enableTransmit(FAN_TX_PIN);
  fanSwitch.setPulseLength(310);       
}
float getTemperature(){
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
void temperatureCheck(){
  if (temperatureFunctionNextRun <= 60000){    
    temperatureFunctionNextRun++;
    return;  
  }
  temperatureFunctionNextRun = 0;
  float temp = getTemperature();
  Serial.print("Temperature is: ");
  Serial.println(temp);  
  if (temp >= 25.75) {
    // Turn fan on
    fanSwitch.switchOn("11111", "00001");  
    fanstate = 1;
  }
  if (temp < 25.5){
    // Turn fan off
    fanSwitch.switchOff("11111", "00001");    
    fanstate = 0;
  }
}
void fanstateToggle(){
  if (fanstate){
    fanSwitch.switchOff("11111", "00001");    
    fanstate = 0;
  } else {
    fanSwitch.switchOn("11111", "00001");  
    fanstate = 1;
  }
  temperatureFunctionNextRun = 0;
}
/*
 * Main function, get and show the temperature
 */
void temperatureTest(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(sensors.getTempCByIndex(0));  
}

