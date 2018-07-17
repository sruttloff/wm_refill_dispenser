/*
 WiFiEsp example: WebServerLed
 
 A simple web server that lets you turn on and of an LED via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.
 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/
#define _ESPLOGLEVEL_ 0 // edit in file directly
#include "WiFiEsp.h"

void sendHttpResponse(WiFiEspClient client);
void send404Response(WiFiEspClient client);
void sendXhrResponse(WiFiEspClient client, String txtT);

char ssid[] = "zyx";            // your network SSID (name)
char pass[] = "xyz";        // your network password
int status = WL_IDLE_STATUS;
WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void wifiSetup()
{  
  Serial1.begin(115200);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module
  

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }


  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  
  // WiFi.hostname("meinTest");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void processWifi()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    // Serial.println("New client");             // print a message out the serial port
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        String response = "";
        while (client.available())     {
          char c = client.read();
          response += c;
        }
        String req = response; //client.readStringUntil('\r\n\r\n');
        Serial.println(req);

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
         // Serial.println(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
                
        if (req.indexOf(" /index.html") != -1)
          sendHttpResponse(client);
        else if (req.indexOf("GET / ") != -1)
          sendHttpResponse(client);        
        else if (req.indexOf("GET /xhr.html") != -1){          
          String txtT = req.substring(req.indexOf("GET /xhr.html?")+14, req.indexOf("HTTP/1.1"));
          sendXhrResponse(client, txtT);                  
        } else {
          // wrong way
          send404Response(client);
        }
        break;
        // ----
              
      }
    }
    
    // close the connection
    client.stop();
    //Serial.println("Client disconnected");
  }
}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  String txt = "<html><head><title>Aqua Computer</title>";
  txt += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  txt += "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\">";
  txt += "<link rel=\"stylesheet\" href=\"https://code.getmdl.io/1.3.0/material.pink-cyan.min.css\" />";
  txt += "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\" />";
  txt += "<link rel=\"stylesheet\" href=\"https://cdn.saas-secure.com/css/styles.min.css\" />";
  txt += "<script defer src=\"https://code.getmdl.io/1.3.0/material.min.js\"></script>";
  txt += "<script defer src=\"https://code.jquery.com/jquery-3.3.1.min.js\"></script>";
  txt += "<script defer src=\"https://cdn.saas-secure.com/js/aqua-computer.js\"></script>";
  txt += "</head><body><div class=\"mdl-grid\">";
  
  txt += "<div class=\"mdl-cell mdl-cell--12-col\">";
  txt += "<h3>Aqua Computer v1.0</h3>";
  txt += "</div>";    
  client.print(txt);

  // wave controller
  txt = "<div class=\"mdl-cell mdl-cell--6-col mdl-cell--4-col-phone mdl-cell--8-col-tablet\">";
  client.print(txt);  

  txt = "<h4>Wavecontroller</h4>";
  txt += "<table><thead><tr><th>Pump</th><th>Min.Value</th><th>Max.Value</th><th>&nbsp;</th></tr></thead>";
  txt += "<tbody>";
  txt += "<tr><td>1.</td><td><input type='text' name='pump1min' value='" + String(pump1Min) + "' size='3' /></td><td><input type='text' name='pump1max' value='" + String(pump1Max) + "' size='3' /></td><td rowspan='2'><button class='mdl-button mdl-js-button mdl-button--raised mdl-button--colored' onclick='sendPumpValues();'>Send</button></td></tr>";
  txt += "<tr><td>2.</td><td><input type='text' name='pump2min' value='" + String(pump2Min) + "' size='3' /></td><td><input type='text' name='pump2max' value='" + String(pump2Max) + "' size='3' /></td></tr>";  
  txt += "</tbody></table>";
  client.print(txt);
  int speed1 =  (100.00 / float (pump1Max - pump1Min)) * (pump1Current - pump1Min);
  int speed2 =  (100.00 / float (pump2Max - pump2Min)) * (pump2Current - pump2Min);
  
  txt = "Controller 1: <span id='speed1'>" + String(speed1) + "</span>% Controller 2: <span id='speed2'>" + String(speed2) + "</span>% <i style='vertical-align:bottom' class=\"material-icons hover\" onclick=\"refreshWavecontroller();\">autorenew</i><br />";  
  txt += "Night Mode is: <span id='nightmode'><span class=\"mdl-color-text--green " + String(pumpRelaxMode ? "" : "no-display") + "\">On</span><span class=\"mdl-color-text--red " + String(!pumpRelaxMode ? "" : "no-display") + "\">Off</span></span><br />";  
  txt += "Shuffle Mode is: <span id='shufflemode'><span class=\"mdl-color-text--green " + String(pumpShuffle ? "" : "no-display") + "\">On</span><span class=\"mdl-color-text--red " + String(!pumpShuffle ? "" : "no-display") + "\">Off</span> Pump 1(" + String(pump1MaxTmp) + ") Pump 2(" + String(pump2MaxTmp) + ")</span><br />";  
  client.print(txt);
  txt = "Foodbreak is: <span id='foodbreak' class='hover' onclick='toggleFoodbreak();'><span class=\"mdl-color-text--green " + String(foodBreak == 1 ? "" : "no-display") + "\">On</span><span class=\"mdl-color-text--red " + String(foodBreak == 0 ? "" : "no-display") + "\">Off</span></span><br />";  
  txt += "Cleaning is: <span id='cleaningbreak' class='hover' onclick='toggleCleaningbreak();'><span class=\"mdl-color-text--green " + String(pumpClean ? "" : "no-display") + "\">On</span><span class=\"mdl-color-text--red " + String(!pumpClean ? "" : "no-display") + "\">Off</span></span><br />";  
  txt += "</div>";  
  client.print(txt);
  


  // temperature  
  txt = "<div class=\"mdl-cell mdl-cell--6-col mdl-cell--4-col-phone mdl-cell--8-col-tablet\">";
  txt += "<h4>Temperature</h4>";
  client.print(txt);
  float temp = getTemperature();
  if (temp < 24.0 || temp > 28.0){
    txt = "<span class=\"mdl-color-text--red\">";
  } else if (temp < 24.50 || temp > 26.5){
    txt = "<span class=\"mdl-color-text--yellow\">";
  } else {
    txt = "<span class=\"mdl-color-text--green\">";
  }
  client.println("<br />Current Temperature: <span id=\"temperature\">" + txt + String(getTemperature()) + "&deg;C</span></span> <i style='vertical-align:bottom' class=\"material-icons hover\" onclick=\"refreshTemperature();\">autorenew</i>");   
  txt = "<br />Fan is: <span id='fanstate' class='hover' onclick='toggleFanstate();'><span class=\"mdl-color-text--green " + String(fanstate == 1 ? "" : "no-display") + "\">On</span><span class=\"mdl-color-text--red " + String(fanstate == 0 ? "" : "no-display") + "\">Off</span></span>";  
  txt += "</div>";  
  txt += "</div>";  
  client.print(txt);
  
  txt = "<div class=\"mdl-grid\">";
  txt += "<div class=\"mdl-cell mdl-cell--12-col mdl-cell--4-col-phone mdl-cell--8-col-tablet\">";
  client.print(txt);
  client.println("<small>Page rendered: " + String(currentDateTime.hour()) + ":" + String(currentDateTime.minute()) + ":" + String(currentDateTime.second()) + "</small>");   
  txt = "</div>";    
  txt += "</div></body></html>";
  client.print(txt);
  client.println();
  client.println();
}
void sendXhrResponse(WiFiEspClient client, String txtT)
{
  char charBuf[txtT.length() + 1];
  char *params[5][2];
  // Serial.println("Zeile1: " + txtT);
  txtT.toCharArray(charBuf,txtT.length());
  int resultsCt = parseUrlParams(charBuf, params, 5, true);
          
//  Serial.print("produced ");
//  Serial.print(resultsCt);
//  Serial.print(" parameters:");
//  Serial.println();
//
//  for (int i = 0; i < resultsCt; i++) {
//    Serial.print("param ");
//    Serial.print(i);
//    Serial.print(" name \"");
//    Serial.print( params[i][0]);
//    Serial.print("\", value \"");
//    Serial.print( params[i][1]);
//    Serial.print("\".");
//    Serial.println();
//  }
  
  Serial.println("Aufruf: " +  String(params[0][0]) + " - " + String(params[0][1]));
  String evt = String(params[0][1]);

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  String txt = "";
  txt += "{\"Results\":{\"MSGHead\":\"\",\"MSGFoot\":\"\",\"MSGBody\":\"\"";
  if (evt == "xhrGetTemperature" || evt == "xhrFanstateToggle"){
    if (evt == "xhrFanstateToggle")  
      fanstateToggle();
    txt += ",\"js\":\"updateTemperature(\\\"";
    float temp = getTemperature();
    if (temp < 24.0 || temp > 28.0){
      txt += "<span class='mdl-color-text--red'>";
    } else if (temp < 24.50 || temp > 26.5){
      txt += "<span class='mdl-color-text--yellow'>";
    } else {
      txt += "<span class='mdl-color-text--green'>";
    }
    txt += String(getTemperature()) + "&deg;C</span>";   
    txt += "\\\", " + String(fanstate) + ");\"";
  } 
  else if (evt == "xhrGetSpeed" || evt == "xhrFoodbreakToggle" || evt == "xhrCleaningbreakToggle"){        
    if (evt == "xhrFoodbreakToggle")
      foodbreakToggle();
    if (evt == "xhrCleaningbreakToggle")
      pumpCleaningToggle();
    int speed1 =  (100.00 / float (pump1Max - pump1Min)) * (pump1Current - pump1Min);
    int speed2 =  (100.00 / float (pump2Max - pump2Min)) * (pump2Current - pump2Min);
    txt += ",\"js\":\"updateSpeed(" +  String(speed1) + "," + String(speed2) + "," + String(foodBreak) + "," + String(pumpClean) + ");\"";
  }  
  else if (evt == "xhrAny"){
    
  } else
    txt += ",\"js\":\"alert(\\\"Command Unknown!\\\");\"";
  txt += "}}";  
  client.print(txt);
  
  client.println();
  client.println();
}

void send404Response(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 404 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  client.print("Page not found"); 
  client.println("<br>");
  client.println("<br>");
  
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
/**
 * queryString: the string with is to be parsed.
 * WARNING! This function overwrites the content of this string. Pass this function a copy
 * if you need the value preserved.
 * results: place to put the pairs of param name/value.
 * resultsMaxCt: maximum number of results, = sizeof(results)/sizeof(*results)
 * decodeUrl: if this is true, then url escapes will be decoded as per RFC 2616
 */

int parseUrlParams(char *queryString, char *results[][2], int resultsMaxCt, boolean decodeUrl) {
  int ct = 0;

  while (queryString && *queryString && ct < resultsMaxCt) {
    results[ct][0] = strsep(&queryString, "&");
    results[ct][1] = strchrnul(results[ct][0], '=');
    if (*results[ct][1]) *results[ct][1]++ = '\0';

    if (decodeUrl) {
      percentDecode(results[ct][0]);
      percentDecode(results[ct][1]);
    }

    ct++;
  }

  return ct;
}

/**
 * Perform URL percent decoding.
 * Decoding is done in-place and will modify the parameter.
 */

void percentDecode(char *src) {
  char *dst = src;

  while (*src) {
    if (*src == '+') {
      src++;
      *dst++ = ' ';
    }
    else if (*src == '%') {
      // handle percent escape

      *dst = '\0';
      src++;

      if (*src >= '0' && *src <= '9') {
        *dst = *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst = 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst = 10 + *src++ - 'a';
      }

      // this will cause %4 to be decoded to ascii @, but %4 is invalid
      // and we can't be expected to decode it properly anyway

      *dst <<= 4;

      if (*src >= '0' && *src <= '9') {
        *dst |= *src++ - '0';
      }
      else if (*src >= 'A' && *src <= 'F') {
        *dst |= 10 + *src++ - 'A';
      }
      else if (*src >= 'a' && *src <= 'f') {
        *dst |= 10 + *src++ - 'a';
      }

      dst++;
    }
    else {
      *dst++ = *src++;
    }

  }
  *dst = '\0';
}
