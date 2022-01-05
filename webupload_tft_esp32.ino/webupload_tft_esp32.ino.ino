#include "wificreds.h"
#include "weatherMapping.h"
#include "weatherApiKey.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>
#include <ESPNexUpload.h>
#include <ArduinoJson.h>
#include <NTPClient.h>

// WARNING:  If you read this code your head will explode and you will embrace the heat death of the universe as a welcome relief.
// I have made zero attempt to make it good.  It's just cobbled together until it sort of works. 
// I will fix it one day, probably.




/*
Create a file called wificreds.h like this:

#define STASSID "your_ssid"
#define STAPSK  "your_wifi_password"
*/

/*
The "ESPNexUpload" library hardcodes the serial GPIOs for the ESP32, but
they are the wrong way round for the NSPanel.  You need to edit ESPNexUpload.cpp
line 47 & 48 and swap the "16" and the "17".

#define NEXT_RX 17 // Default in the library are the wrong way around
#define NEXT_TX 16

I don't like that library and I will replace it with either something I 
cobble together, or use https://github.com/esphome/esphome/blob/dev/esphome/components/nextion/nextion.cpp
and adapt it to my needs.  It looks a lot cleaner.
*/
const char* host = "nextion";
unsigned long previousMillis = 0;
unsigned long threehourmillis = 0;

// NexUploader stuff
int fileSize  = 0;
bool result   = true;
//ESPNexUpload nextion(115200);
WebServer server(80);

// Time stuff
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time"); //, utcOffsetInSeconds);

// HTTP Client Stuff
WiFiClient wificlient;
HTTPClient http;

String getContentType(String filename){
  if(server.hasArg(F("download"))) return F("application/octet-stream");
  else if(filename.endsWith(F(".htm"))) return F("text/html");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(F(".css"))) return F("text/css");
  else if(filename.endsWith(F(".js"))) return F("application/javascript");
  else if(filename.endsWith(F(".png"))) return F("image/png");
  else if(filename.endsWith(F(".gif"))) return F("image/gif");
  else if(filename.endsWith(F(".jpg"))) return F("image/jpeg");
  else if(filename.endsWith(F(".ico"))) return F("image/x-icon");
  else if(filename.endsWith(F(".xml"))) return F("text/xml");
  else if(filename.endsWith(F(".pdf"))) return F("application/x-pdf");
  else if(filename.endsWith(F(".zip"))) return F("application/x-zip");
  else if(filename.endsWith(F(".gz"))) return F("application/x-gzip");
  return F("text/plain");
}


bool handleFileRead(String path) {                          // send the right file to the client (if it exists)
  Serial.print("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";             // If a folder is requested, send the index file
  String contentType = getContentType(path);                // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {   // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                        // Use the compressed verion
    File file = SPIFFS.open(path, "r");                     // Open the file
    size_t sent = server.streamFile(file, contentType);     // Send it to the client
    file.close();                                           // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);      // If the file doesn't exist, return false
  return false;
}


// handle the file uploads
bool handleFileUpload(){
  //Serial2.end();
  ESPNexUpload nextion(115200);
  HTTPUpload& upload = server.upload();

  // Check if file seems valid nextion tft file
  if(!upload.filename.endsWith(F(".tft"))){
    server.send(500, F("text/plain"), F("ONLY TFT FILES ALLOWED\n"));
    return false;
  }
  
  if(!result){
    // Redirect the client to the failure page
    server.sendHeader(F("Location"),"/failure.html?reason=" + nextion.statusMessage);
    server.send(303);
    return false;
  }

  if(upload.status == UPLOAD_FILE_START){

    Serial.println(F("\nFile received. Update Nextion..."));

    // Prepare the Nextion display by seting up serial and telling it the file size to expect
    result = nextion.prepareUpload(fileSize);
    
    if(result){
      Serial.print(F("Start upload. File size is: "));
      Serial.print(fileSize);
      Serial.println(F(" bytes"));
    }else{
      Serial.println(nextion.statusMessage + "\n");
      return false;
    }
    
  }else if(upload.status == UPLOAD_FILE_WRITE){

    // Write the received bytes to the nextion
    result = nextion.upload(upload.buf, upload.currentSize);
    
    if(result){
      Serial.print(F("."));
    }else{
      Serial.println(nextion.statusMessage + "\n");
      return false;
    }
  
  }else if(upload.status == UPLOAD_FILE_END){

    // End the serial connection to the Nextion and softrest it
    
    //  This uploader code is bad.
    //  I moved the ESPNexUpload in to this function instead of global
    //  and it mostly works, but then craps out at the end.  I think
    //  that the upload to the ESP32 has finished, but it hasn't finished
    //  writing over the serial to the screen.  Try and hack this with a
    // delay

    //delay(15000);
    nextion.end();
    
    Serial.println("");
    //Serial.println(nextion.statusMessage);
    //Serial2.begin(115200, SERIAL_8N1, 17,16);
    return true;
  }
return false;
}
// End Uploader stuff #######################

bool writeNxt(std::string data) {
  byte terminator[3] = {255,255,255};
  Serial2.write(data.c_str());
  Serial2.write(terminator,3);
  return true;
}

std::string dayNameShort(unsigned int y, unsigned int m, unsigned int d) {
  std::string day[] = {
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "Sun",
    "Mon",
    "Tue"
  };  
  m = (m + 9) % 12;
  y -= m / 10;
  unsigned long dn = 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d-1);
  return day[dn % 7];
}


void doWeather() {
  // Every three hours we should update ourselves
  // We need a lot of space for the JSON, so do it all in here and then throw it all away
  //std::string weatherLocation = "353363";
  
  // Make the weather man appear
  writeNxt("page 0");
  writeNxt("vis 255,0");
  writeNxt("vis michaelfish,1");
  //writeNxt("ref michaelfish");
  delay(500);

  std::string url = "http://datapoint.metoffice.gov.uk/public/data/val/wxfcs/all/json/";
  url += "353363";
  url += "?res=daily&key=";
  url += metApiKey;

  http.useHTTP10(true);
  http.begin(wificlient, url.c_str());
  http.GET();
  DynamicJsonDocument doc(6144);
  DeserializationError error = deserializeJson(doc, http.getStream());
  http.end();
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  };

  int count = 0;
  
  JsonObject SiteRep_DV = doc["SiteRep"]["DV"];
  JsonObject SiteRep_DV_Location = SiteRep_DV["Location"];
  
  for (JsonObject SiteRep_DV_Location_Period_item : SiteRep_DV_Location["Period"].as<JsonArray>()) {
    if (count > 7) break; // We only have 8 slots, and zero is the big picture.

    const char* date0 = SiteRep_DV_Location_Period_item["value"];
    String date = String(date0); // 2022-01-07Z
    date.remove(date.length()-1,1); // Strip Z
    int yyyy = date.substring(0,date.indexOf("-")).toInt();
    int mm = date.substring(date.indexOf("-")+1,date.lastIndexOf("-")).toInt();
    int dd = date.substring(date.lastIndexOf("-")+1).toInt();
    std::string dayshort = dayNameShort(yyyy,mm,dd);

    JsonObject SiteRep_DV_Location_Period_item_Rep_0 = SiteRep_DV_Location_Period_item["Rep"][0];
    const char* windDirDay = SiteRep_DV_Location_Period_item_Rep_0["D"];
    const char* maxWindSpeedDay = SiteRep_DV_Location_Period_item_Rep_0["Gn"];
    int maxWindSpeedDayInt = atoi(maxWindSpeedDay);
    //const char* humidNoon0 = SiteRep_DV_Location_Period_item_Rep_0["Hn"];
    const char* rainChanceDay = SiteRep_DV_Location_Period_item_Rep_0["PPd"];
    int rainChanceDayInt = atoi(rainChanceDay);
    const char* windSpeedDay = SiteRep_DV_Location_Period_item_Rep_0["S"];
    int windSpeedDayInt = atoi(windSpeedDay);
    //const char* vis0 = SiteRep_DV_Location_Period_item_Rep_0["V"];
    const char* dayMaxTemp = SiteRep_DV_Location_Period_item_Rep_0["Dm"];
    int dayMaxTempInt = atoi(dayMaxTemp);
    //const char* feelsLikeDayMax0 = SiteRep_DV_Location_Period_item_Rep_0["FDm"];
    const char* dayWeatherType = SiteRep_DV_Location_Period_item_Rep_0["W"];
    const char* uvIndexDay = SiteRep_DV_Location_Period_item_Rep_0["U"];
    int uvIndexDayInt = atoi(uvIndexDay);
    const char* timePeriodDay = SiteRep_DV_Location_Period_item_Rep_0["$"];
 
    JsonObject SiteRep_DV_Location_Period_item_Rep_1 = SiteRep_DV_Location_Period_item["Rep"][1];
    const char* windDirNight = SiteRep_DV_Location_Period_item_Rep_1["D"];
    const char* maxWindSpeedNight = SiteRep_DV_Location_Period_item_Rep_1["Gm"];
    int maxWindSpeedNightInt = atoi(maxWindSpeedNight);
    //const char* SiteRep_DV_Location_Period_item_Rep_1_Hm = SiteRep_DV_Location_Period_item_Rep_1["Hm"];
    const char* rainChanceNight = SiteRep_DV_Location_Period_item_Rep_1["PPn"];
    int rainChanceNightInt = atoi(rainChanceNight);
    const char* windSpeedNight = SiteRep_DV_Location_Period_item_Rep_1["S"];
    //const char* SiteRep_DV_Location_Period_item_Rep_1_V = SiteRep_DV_Location_Period_item_Rep_1["V"];
    const char* nightMinTemp = SiteRep_DV_Location_Period_item_Rep_1["Nm"];
    int nightMinTempInt = atoi(nightMinTemp);
    //const char* SiteRep_DV_Location_Period_item_Rep_1_FNm = SiteRep_DV_Location_Period_item_Rep_1["FNm"];
    const char* nightWeatherType = SiteRep_DV_Location_Period_item_Rep_1["W"];
    const char* timePeriodNight = SiteRep_DV_Location_Period_item_Rep_1["$"];

    if (count == 0) {
      // First loop we update the main weather
      if (timeClient.getHours() > 16) {
        // it's already night, so let's skip straight to the night forecast
        byte weatherPic = WEATHER_CODES_LARGE[atoi(nightWeatherType)];
        std::string weatherPicStr = std::to_string(weatherPic);
        std::string command = "p8.pic="+weatherPicStr;
        writeNxt(command.c_str());
        count=1;
        writeNxt("vis p8,1");

        int pos = 38;        
        if (maxWindSpeedNightInt > 20) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(98);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        };
        if (nightMinTempInt < 5) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(96);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
        if (rainChanceNightInt > 60) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(97);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
      } else {
        byte weatherPic = WEATHER_CODES_LARGE[atoi(dayWeatherType)];
        byte weatherPicSm = WEATHER_CODES_SMALL[atoi(nightWeatherType)];
        std::string weatherPicStr = std::to_string(weatherPic);
        std::string command = "p8.pic="+weatherPicStr;
        writeNxt(command);
        int pos = 38;        
        if (maxWindSpeedDayInt > 20) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(98);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        };
        if (dayMaxTempInt < 5) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(96);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
        if (rainChanceDayInt > 60) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(97);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }

        if (uvIndexDayInt > 5) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(95);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }  

        weatherPicStr = std::to_string(weatherPicSm);
        command = "p1.pic="+weatherPicStr;
        writeNxt(command.c_str());
        writeNxt("vis p1,1");
        std::string dayAndTemperatureS = dayshort+": "+nightMinTemp;//+"°";
        dayAndTemperatureS += "\xB0";
        //std::string textId = std::to_string(count+4)
        command = "t4.txt=\""+dayAndTemperatureS+"\"";
        writeNxt(command.c_str());
        pos = 9;
        if (maxWindSpeedNightInt > 20) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(91);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        };
        if (nightMinTempInt < 5) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(89);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
        if (rainChanceNightInt > 60) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(90);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }

        count=2;
        writeNxt("vis p8,1");
        writeNxt("vis t4,1");
      }
      delay(1000);
      continue;
    } else {
      // This should be the next time we come in to the loop.  We will have either
      // updated the big picture and the first little one, or just the big one.
      // `count` should tell us where we got to.
      byte weatherPicDay = WEATHER_CODES_SMALL[atoi(dayWeatherType)];
      byte weatherPicNight = WEATHER_CODES_SMALL[atoi(nightWeatherType)];
      std::string weatherPicDayS    = std::to_string(weatherPicDay);
      std::string weatherPicNightS  = std::to_string(weatherPicNight);
      std::string dayid = std::to_string(count);
      std::string nightid = std::to_string(count+1);
      std::string dayCommand = "p"+dayid+".pic="+weatherPicDayS;
      std::string nightCommand = "p"+nightid+".pic="+weatherPicNightS;
      writeNxt(dayCommand.c_str());
      std::string visDCmd = "vis p" + dayid + ",1";
      std::string visNCmd = "vis p" + nightid + ",1";
      writeNxt(visDCmd);

      // Deal with day temperatures
      int t = atoi(dayMaxTemp);
      std::string dayAndTemperatureS = dayshort+": " + std::to_string(t);
      dayAndTemperatureS += "\xB0"; // °
      std::string textId = std::to_string(count+3);
      std::string command = "t"+textId+".txt=\""+dayAndTemperatureS+"\"";
      writeNxt(command.c_str());
      writeNxt("vis t"+textId+",1");
      int pos = count * 4 + 5;
      if (maxWindSpeedDayInt > 20) {
        std::string pid = "p" + std::to_string(pos);
        std::string pc = pid + ".pic=";
        pc += std::to_string(91);
        writeNxt(pc);
        writeNxt("vis "+pid+",1");
        pos += 1;
      };
      if (dayMaxTempInt < 5) {
        std::string pid = "p" + std::to_string(pos);
        std::string pc = pid + ".pic=";
        pc += std::to_string(89);
        writeNxt(pc);
        writeNxt("vis "+pid+",1");
        pos += 1;
      }
      if (rainChanceDayInt > 60) {
        std::string pid = "p" + std::to_string(pos);
        std::string pc = pid + ".pic=";
        pc += std::to_string(90);
        writeNxt(pc);
        writeNxt("vis "+pid+",1");
        pos += 1;
      }
      if (uvIndexDayInt > 5) {
        std::string pid = "p" + std::to_string(pos);
        std::string pc = pid + ".pic=";
        pc += std::to_string(95);
        writeNxt(pc);
        writeNxt("vis "+pid+",1");
        pos += 1;
      } 
      
      delay(1000);
      pos = count * 4 + 9;

      // Deal with night temperatures
      t = atoi(nightMinTemp);
      dayAndTemperatureS = dayshort+": " + std::to_string(t);
      dayAndTemperatureS += "\xB0";
      textId = std::to_string(count+4);
      command = "t"+textId+".txt=\""+dayAndTemperatureS+"\"";
      if (count < 7 ) {
        writeNxt(nightCommand.c_str()); // Skip the last one if we'd overflow
        writeNxt(visNCmd);
        writeNxt(command.c_str()); // Skip the last one if we'd overflow
        writeNxt("vis t"+textId+",1");
        if (maxWindSpeedNightInt > 20) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(91);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        };
        if (nightMinTempInt < 5) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(89);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
        if (rainChanceNightInt > 60) {
          std::string pid = "p" + std::to_string(pos);
          std::string pc = pid + ".pic=";
          pc += std::to_string(90);
          writeNxt(pc);
          writeNxt("vis "+pid+",1");
          pos += 1;
        }
      };

      delay(1000);
      count+=2;
    }
  } 
  
  // Make the weather man vanish
  writeNxt("vis michaelfish,0");
  writeNxt("vis t1,1");
  writeNxt("vis n0,1");
  writeNxt("vis t3,1");
  writeNxt("vis t2,1");
  writeNxt("vis p37,1");
  writeNxt("vis p0,1");
  writeNxt("vis n1,1");
  writeNxt("vis t0,1");

}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 17,16);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      SPIFFS.end();
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // NS Panel Specifics
  //pinMode(21, OUTPUT); // Buzzer. GPIO21 but which port do I put here?
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);  
  pinMode(4, OUTPUT);  // Screen reset. High = on?
  digitalWrite(4, LOW); // Switch on screen, LOW = on.

  if(!SPIFFS.begin()){
       Serial.println(F("An Error has occurred while mounting SPIFFS"));
       Serial.println(F("Did you upload the data directory that came with this example?"));
       return;
  } 

MDNS.begin(host);
  Serial.print(F("http://"));
  Serial.print(host);
  Serial.println(F(".local"));

  //SERVER INIT
  server.on("/", HTTP_POST, [](){ 

    Serial.println(F("Successfully updated Nextion!\n"));
    // Redirect the client to the success page after handeling the file upload
    server.sendHeader(F("Location"),F("/success.html"));
    server.send(303);
    return true;
  },
    // Receive and save the file
    handleFileUpload
  );

  // receive fileSize once a file is selected (Workaround as the file content-length is of by +/- 200 bytes. Known issue: https://github.com/esp8266/Arduino/issues/3787)
  server.on("/fs", HTTP_POST, [](){
    fileSize = server.arg(F("fileSize")).toInt();
    server.send(200, F("text/plain"), "");
  });

  // called when the url is not defined here
  // use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, F("text/plain"), F("FileNotFound"));
  });


  server.begin();
  Serial.println(F("\nHTTP server started"));    

  // NTP
  timeClient.begin();
  timeClient.update();
  while (!timeClient.isTimeSet()) {
    tone(21,4186,100);
    delay(1000);
  };
  writeNxt("page 0");
  writeNxt("vis michaelfish,0");
  doWeather();
}

byte pic = 2;

void loop() {
  ArduinoOTA.handle();
  timeClient.update();  // Has it's own rate limiter, so call with abandon
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 2500) {
    previousMillis = millis();
    //tone(21,4186, 100);
    // std::string cmd = "page0.p8.pic=";
    // cmd += std::to_string(pic);
    // writeNxt(cmd);
    // if (pic == 2) {
    //   pic = 3;
    // } else {
    //   pic =2;
    // }
  }

  if (currentMillis - threehourmillis >= 10800000){
    threehourmillis = millis();
    doWeather();
  }

  if(Serial2.available()) {
    Serial.write(".");
    Serial.write(Serial2.read());
  }
}
