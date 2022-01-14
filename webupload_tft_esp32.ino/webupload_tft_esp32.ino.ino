#include "wificreds.h"
#include "weatherApiKey.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include "ESPNexUpload.h"
#include <BluetoothSerial.h>

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

Or I've included a hacked up version in github.  I was going to make changes to it, but haven't done yet.
Once an upload is complete you have to reboot everything for now.
*/

const char* host = "nextion";
unsigned long previousMillis = 0;
unsigned long hourmillis = 0;
unsigned long minutemillis = 0;
unsigned long fiveminsmillis = 0;

const byte key1   = 14;
const byte key2   = 27;
const byte relay1 = 22;
const byte relay2 = 19;
const byte ntc    = 38;

// Forward declation:
void doHWC();
void doDownstairsTemps();


// NexUploader stuff
int fileSize  = 0;
bool result   = true;
ESPNexUpload nextion(115200);
WebServer server(80);

// Time stuff
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time"); //, utcOffsetInSeconds);

// HTTP Client Stuff
WiFiClient wificlient;
HTTPClient http;

// Serial Bluetooth
// This is quite heavy.  Might disable it once everything works.
//BluetoothSerial SerialBT;

void logger(std::string message){
  //SerialBT.println(message.c_str());
}

String getContentType(String filename){
  if(server.hasArg(F("download"))) return F("application/octet-stream");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(F(".gz"))) return F("application/x-gzip");
  return F("text/plain");
}


bool handleFileRead(String path) {                          // send the right file to the client (if it exists)
  if (path.endsWith("/")) path += "index.html";             // If a folder is requested, send the index file
  String contentType = getContentType(path);                // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {   // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                        // Use the compressed verion
    File file = SPIFFS.open(path, "r");                     // Open the file
    size_t sent = server.streamFile(file, contentType);     // Send it to the client
    file.close();                                           // Close the file again
    return true;
  }
  return false;
}

// handle the TFT file uploads
bool handleFileUpload(){
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
    logger("Update Nextion");
    result = nextion.prepareUpload(fileSize);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    nextion.upload(upload.buf, upload.currentSize);  
  }else if(upload.status == UPLOAD_FILE_END){
    delay(5000);
    nextion.end();
    return true;
  }
return false;
}

void handleSerialInput(){
  std::string incomingData= "";
  while (Serial2.available()) {
    incomingData+=(char)Serial2.read();
  }
  //logger(SerialBT.print(F("Received data from Nextion: "));
  logger(incomingData.c_str());
  // for (char const c: incomingData) {
  //   SerialBT.print(c, HEX);
  //   SerialBT.print(":");
  // }
  // Serial.println(F("--"));
  int strSize = incomingData.size();
  char serialBytes[strSize+1];
  strcpy(serialBytes, incomingData.c_str());
  
  byte a = 0;
  for (int i=strSize-1; i > strSize-4; i--) {
    a = serialBytes[i];
    if (a != 255) {
      return;
    }
  };

  int page = 0;
  switch (serialBytes[0]) {
    case 0x66:
      // Page change
      page = serialBytes[1];
      switch (page) {
        case 1:
          // Hot water page
          doHWC();
        case 3:
          doDownstairsTemps();
      };
    case 0x65:
      // Touch event
      page = serialBytes[1];
      int objectId = serialBytes[2];
      switch (page) {
        case 0x03:
          // Ground floor map
          doRoom(page, objectId);
      };
  };
}

bool writeNxt(std::string data) {
  byte terminator[3] = {255,255,255};
  //SerialBT.print("Outgoing NXT command: ");
  //SerialBT.println(data.c_str());
  Serial2.write(data.c_str());
  Serial2.write(terminator,3);
  return true;
}

std::string dateToDayName(unsigned int y, unsigned int m, unsigned int d) {
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

void setPicture(int pictureId, int pictureNumber) {
  std::string picId = "p" + std::to_string(pictureId);
  std::string picCmd = picId + ".pic=";
  picCmd += std::to_string(pictureNumber);
  writeNxt(picCmd);
  writeNxt("vis "+picId + ",1");
}

void setText(int textId, std::string text) {
  std::string txtId = "t" + std::to_string(textId);
  std::string txtCmd = txtId + ".txt=\"" + text + "\"";
  writeNxt(txtCmd);
  writeNxt("vis " + txtId + ",1");
}

void setNumber(int numId, int value) {
  std::string nId = "n" + std::to_string(numId);
  std::string nCmd = nId+".val="+std::to_string(value);
  writeNxt(nCmd);
  writeNxt("vis " + nId + ",1");
};

void setNumber(std::string nId, int value) {
  std::string nCmd = nId+".val="+std::to_string(value);
  writeNxt(nCmd);
  writeNxt("vis " + nId + ",1");
};


void getTemperatureFromWeb(std::string room, int textElement){
  // room is the room name for the url
  // textElement is the text element on the page to set
  // Doing this as a function so that the JSON docs are short lived
  http.useHTTP10(true);
  room = "http://smarthome.whizzy.org:1880/get/temperature/"+room;
  http.begin(wificlient, room.c_str());
  int returnCode = http.GET();
  if (returnCode != 200) {
    //SerialBT.println("Fail get rm tmp");
    return;
  }
  StaticJsonDocument<48> doc;
  DeserializationError error = deserializeJson(doc, http.getStream());
  if (error) {
    //SerialBT.print("Rm tmp deser failed: ");
    //SerialBT.println(error.c_str());
    return;
  }
  float temperature = doc["temperature"]; // 20.52
  int rounded = nearbyint(temperature); // UI only designed to support ints
  std::string t = std::to_string(rounded);
  setText(textElement, t);
}

void doDownstairsTemps(){
  getTemperatureFromWeb("study",7);
  getTemperatureFromWeb("hall",8);
  getTemperatureFromWeb("lounge",6);
  getTemperatureFromWeb("dining_rm",5);
  getTemperatureFromWeb("den", 4);
};

void doWeather() {
  writeNxt("page 0");
  writeNxt("vis 255,0");
  writeNxt("vis michaelfish,1");
  delay(1000);

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
    //SerialBT.print("deserializeJson() failed: ");
    //SerialBT.println(error.c_str());
    return;
  };

  int count = 0;
  std::string dayAndTemperatureS = "";

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
    std::string dayshort = dateToDayName(yyyy,mm,dd);

    JsonObject SiteRep_DV_Location_Period_item_Rep_0 = SiteRep_DV_Location_Period_item["Rep"][0];
    // Day
    //const char* windDirDay = SiteRep_DV_Location_Period_item_Rep_0["D"];
    int maxWindSpeedDayInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["Gn"]);
    int rainChanceDayInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["PPd"]);
    int windSpeedDayInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["S"]);
    int dayMaxTempInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["Dm"]);
    //const char* feelsLikeDayMax0 = SiteRep_DV_Location_Period_item_Rep_0["FDm"];
    int dayWeatherTypeInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["W"]);
    int uvIndexDayInt = atoi(SiteRep_DV_Location_Period_item_Rep_0["U"]);
    const char* timePeriodDay = SiteRep_DV_Location_Period_item_Rep_0["$"];
 
    JsonObject SiteRep_DV_Location_Period_item_Rep_1 = SiteRep_DV_Location_Period_item["Rep"][1];
    // Night
    //const char* windDirNight = SiteRep_DV_Location_Period_item_Rep_1["D"];
    int maxWindSpeedNightInt = atoi(SiteRep_DV_Location_Period_item_Rep_1["Gm"]);
    int rainChanceNightInt = atoi(SiteRep_DV_Location_Period_item_Rep_1["PPn"]);
    int nightMinTempInt = atoi(SiteRep_DV_Location_Period_item_Rep_1["Nm"]);
    //const char* SiteRep_DV_Location_Period_item_Rep_1_FNm = SiteRep_DV_Location_Period_item_Rep_1["FNm"];
    int nightWeatherTypeInt = atoi(SiteRep_DV_Location_Period_item_Rep_1["W"]);
    const char* timePeriodNight = SiteRep_DV_Location_Period_item_Rep_1["$"];

    const int snowflake = 9;
    const int sunglasses = 8;
    const int windy = 11;
    const int umbrella = 10;
    const int sm_snowflake = 2;
    const int sm_sunglasses = 1;
    const int sm_windy = 4;
    const int sm_umbrella = 3;
    const int lg_weather_offset = 18; // The picture ID of the first large weather icon (moon)
    const int sm_weather_offset = 49; // same for small.  This is made easier since the Met Office start their scheme at zero


    if (count == 0) {
      //  First time round the loop, so do the big picture first.
      doOutsideTemperature();
      writeNxt("vis t11,1");

      if (timeClient.getHours() > 16) {
        // it's already night, so let's skip straight to the night forecast
        //setPicture(8, WEATHER_CODES_LARGE_NIGHT[nightWeatherTypeInt]);
        setPicture(8, lg_weather_offset + nightWeatherTypeInt);
        int pos = 38;
        if (maxWindSpeedNightInt > 20) {
          setPicture(pos, windy);
          pos += 1;
        };
        if (nightMinTempInt < 5) {
          setPicture(pos, snowflake);
          pos += 1;
        }
        if (rainChanceNightInt > 60) {
          setPicture(pos, umbrella);
          pos += 1;
        }
        count=1;
      } else {
        // First time round the loop, and it's still daytime, so do the 
        // big picture and then the first small as well.
        //int weatherPicSm = WEATHER_CODES_SMALL_NIGHT[nightWeatherTypeInt];
        //setPicture(8, WEATHER_CODES_LARGE[dayWeatherTypeInt]);
        setPicture(8, lg_weather_offset + dayWeatherTypeInt);
        writeNxt("vis t11,1");
        int pos = 38;        

        if (maxWindSpeedDayInt > 20) {
          setPicture(pos, windy);
          pos += 1;
        };

        if (dayMaxTempInt < 5) {
          setPicture(pos, snowflake);
          pos += 1;
        }
        if (rainChanceDayInt > 60) {
          setPicture(pos, umbrella);
          pos += 1;
        }

        if (uvIndexDayInt > 5) {
          setPicture(pos, sunglasses);
        }

        // Now the first small picture p1
        //setPicture(1, WEATHER_CODES_SMALL_NIGHT[nightWeatherTypeInt]);
        setPicture(1, sm_weather_offset + nightWeatherTypeInt);
        dayAndTemperatureS = dayshort+": "+std::to_string(nightMinTempInt);
        dayAndTemperatureS += "\xB0"; //°
        setText(4, dayAndTemperatureS);
        pos = 9;
        if (maxWindSpeedNightInt > 20) {
          setPicture(pos, sm_windy);
          pos += 1;
        };
        if (nightMinTempInt < 5) {
          setPicture(pos, sm_snowflake);
          pos += 1;
        }
        if (rainChanceNightInt > 60) {
          setPicture(pos, sm_umbrella);
          pos += 1;
        }

        count=2;
      }
      delay(1000);
    } else {
      // Count should be 1 for the first small or 2 for the second small.

      //setPicture(count, WEATHER_CODES_SMALL[dayWeatherTypeInt]);
      setPicture(count, sm_weather_offset + dayWeatherTypeInt);
      
      dayAndTemperatureS = dayshort+": " + std::to_string(dayMaxTempInt);
      dayAndTemperatureS += "\xB0"; // °
      setText(count+3, dayAndTemperatureS);

      int pos = count * 4 + 5;
      if (maxWindSpeedDayInt > 20) {
        setPicture(pos, sm_windy);
        pos += 1;
      };
      if (dayMaxTempInt < 5) {
        setPicture(pos, sm_snowflake);
        pos += 1;
      }
      if (rainChanceDayInt > 60) {
        setPicture(pos, sm_umbrella);
        pos += 1;
      }
      if (uvIndexDayInt > 5) {
        setPicture(pos, sm_sunglasses);
        pos += 1;
      }
      
      pos = count * 4 + 9;
      count +=1;
      if (count > 7) break;

      //setPicture(count, WEATHER_CODES_SMALL_NIGHT[nightWeatherTypeInt]);
      setPicture(count, sm_weather_offset + nightWeatherTypeInt);
      dayAndTemperatureS = dayshort + ": " + std::to_string(nightMinTempInt);
      dayAndTemperatureS += "\xB0"; // °
      setText(count+3, dayAndTemperatureS);

      if (maxWindSpeedNightInt > 20) {
        setPicture(pos, sm_windy);
        pos += 1;
      };
      if (nightMinTempInt < 5) {
        setPicture(pos, sm_snowflake);
        pos += 1;
      }
      if (rainChanceNightInt > 60) {
        setPicture(pos, sm_umbrella);
        pos += 1;
      }

      delay(1000);
      count+=1;
    }
  }
  
  writeNxt("vis michaelfish,0");
  writeNxt("vis t1,1");
  writeNxt("vis n0,1");
  writeNxt("vis t3,1");
  writeNxt("vis t2,1");
  writeNxt("vis p37,1");
  writeNxt("vis p0,1");
}

void updateSwitchStatus(std::string control, std::string picElement) {
    byte relay = (control == "hw") ? relay1 : relay2;
    std::string url = "http://piwarmer.whizzy.org/get/" + control;
    std::string text_label = (control == "hw") ? "t3" : "t2";
    std::string picId = "";
    std::string command = "";
    StaticJsonDocument<64> doc;
    http.useHTTP10(true);
    http.begin(wificlient, url.c_str());
    int returnCode = http.GET();
    if (returnCode != 200) {
      return;
    };
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error) {
      //SerialBT.print("Deserialisation Err: ");
      //SerialBT.println(error.c_str());
      return;
    };
    bool state = doc["state"];
    picId = (state) ? "6" : "7";
    digitalWrite(relay, state);
    // TODO:  Make the Nextion set the state of the icons in an 
    // if statement, and then just set a global variable instead
    // of updating the pictures.
    command = "page0." + picElement + ".pic=" + picId;
    writeNxt(command);
    command = "page1." + picElement + ".pic="+ picId;
    writeNxt(command);
    http.end();
};

void doButtonPress() {  
  bool hwButtPress = !digitalRead(key1);
  bool chButtPress = !digitalRead(key2);
  while (!digitalRead(key1) || !digitalRead(key2) ) { // 0 = pressed, buttons are pulled up.
    delay(20);
  }

  // Using the relay state as the source of truth.  This is far from perfect
  // but I think it's safe and efficient 
  std::string url = "http://piwarmer.whizzy.org/set/";
  if (hwButtPress){
    bool hwState = digitalRead(relay1);
    std::string urlState = (hwState) ? "off" : "on";
    url += "hw/"+urlState; //on|off
    http.begin(wificlient, url.c_str());
    int httpReturn = http.POST("");
    //SerialBT.print("HTTP Return code: ");
    //SerialBT.print(httpReturn);
    http.end();
    digitalWrite(relay1, !hwState);
    updateSwitchStatus("hw", "p37");
  };
  if (chButtPress){
    bool chState = digitalRead(relay2);
    std::string urlState = (chState) ? "off" : "on";
    url += "ch/"+urlState; //on|off
    http.begin(wificlient, url.c_str());
    int httpReturn = http.POST("");
    //SerialBT.print("HTTP Return code: ");
    //SerialBT.print(httpReturn);
    http.end();
    digitalWrite(relay2, !chState);
    updateSwitchStatus("ch", "p0");
  }
};

void doOutsideTemperature(){
  StaticJsonDocument<384> doc;
  http.useHTTP10(true);
  std::string url = "http://smarthome.whizzy.org:8053/utils/latest_outside_temperature.json";
  http.begin(wificlient, url.c_str());
  int returnCode = http.GET();
  if (returnCode == 200) {
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error) {
      //SerialBT.print("Outside temp deserialisation Err: ");
      //SerialBT.println(error.c_str());
      return;
    } else {
      JsonObject fields = doc["fields"];
      int humidity = fields["humidity"]; // 77
      float temperature = fields["temperature"]; // 4.6875
      temperature = temperature * 2;
      temperature = round(temperature);
      temperature = temperature / 2;
      std::string temperature_string = std::to_string(temperature);
      std::size_t point = temperature_string.find('.');
      std::string trimmed_temp = "";
      if (point != std::string::npos) {
        trimmed_temp = temperature_string.substr(0,point+2);
        size_t len = trimmed_temp.length();
        std::string last = trimmed_temp.substr(len-1,1);
        if (last == "0") trimmed_temp = trimmed_temp.substr(0,point);
        std::string command = "page0.t11.txt=\"" + trimmed_temp + "\xB0\"";
        writeNxt(command);
      }
    }
  }
  http.end();
};

void doInternalTemperature(){
  // Read the thermistor.  It's rubbish, don't use it.
  // double Vout = 0;
  //   for (int i=0; i <= 10; i++) {
  //     Vout += analogRead(ntc);
  //     delay(10);
  //   }
  //   Vout = Vout / 10;
  //   Vout = Vout * 3.3/4095.0;
  //   double Rt = 10000.0 * Vout / (3.3 - Vout);
  //   double T = 1/(1/298.15 + log(Rt/10000.0)/3950.0);
  //   double Tc = T - 273.15;
  //   double Cc = Tc / 1.42; // Tweak to get temperature correct.  It should be more or less linear, so a simple divider might do.
  //   int temp = (int) Cc;       
  //   std::string t = std::to_string(temp);
  //   writeNxt("n0.val="+t);
  http.useHTTP10(true);
  std::string url = "http://smarthome.whizzy.org:1880/get/temperature/hall";
  http.begin(wificlient, url.c_str());
  int returnCode = http.GET();
  if (returnCode != 200) {
    //SerialBT.println("Failed to get internal temperature");
    return;
  }
  StaticJsonDocument<48> doc;
  DeserializationError error = deserializeJson(doc, http.getStream());
  if (error) {
    //SerialBT.print("Internal temperature deserializeJson() failed: ");
    //SerialBT.println(error.c_str());
    return;
  }

  float temperature = doc["temperature"]; // 20.52
  int rounded = nearbyint(temperature); // UI only designed to support ints
  std::string t = std::to_string(rounded);
  writeNxt("n0.val="+t);
};

void doHWC(){
 StaticJsonDocument<96> doc;
  http.useHTTP10(true);
  std::string url = "http://piwarmer.whizzy.org/get/hwc";
  std::string command = "";
  http.begin(wificlient, url.c_str());
  int returnCode = http.GET();
  if (returnCode == 200) {
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error) {
      //SerialBT.print("Outside temp deserialisation Err: ");
      //SerialBT.println(error.c_str());
      return;
    } else {
      //bool state = doc["state"]; // false
      //int top = doc["top"]; // 57
      int mid = doc["mid"]; // 50
      //int btm = doc["btm"]; // 51
      mid = mid - 30;
      mid = (mid<0) ? 0 : mid;
      mid = mid * 3 + 2;
      mid = 66 - mid;
      mid = (mid < 0) ? 0 : mid;
      command = "h1.val="+std::to_string(mid);
      writeNxt(command);
      command = "click h1,1"; // Runs the touch event code which switches the images
      writeNxt(command);
      }
    }
  http.end();
};

void doRoom(int page, int roomId) {
  std::string url = "http://smarthome.whizzy.org:1880/get/trv/";  
  std::string room;
  switch (page) {
    case 0x03:  
      switch (roomId) {
        case 0x0A:
          // Den
          room = "den";
        case 0x0B:
          // Dining Rm
          room = "dining_rm";
        case 0x0C:
          // Lounge
          room = "lounge";
        case 0x0D:
          // Study
          room = "study";
        case 0x0E:
          // Hall
          room= "hall";
      }
  }
  StaticJsonDocument<768> doc;
  url += room;
  http.begin(wificlient, url.c_str());
  int returnCode = http.GET();
  if (returnCode != 200) {
    //SerialBT.println("Fail get rm tmp"));
    return;
  };
  DeserializationError error = deserializeJson(doc, http.getStream());
  if (error) {
    //SerialBT.print(F("Room temperature deserializeJson() failed: "));
    //SerialBT.println(error.c_str());
    return;
  }
  //const char* auto_lock = doc["auto_lock"]; // "MANUAL"
  //const char* away_mode = doc["away_mode"]; // "OFF"
  //int away_preset_days = doc["away_preset_days"]; // 0
  //int away_preset_temperature = doc["away_preset_temperature"]; // 15
  bool battery_low = doc["battery_low"]; // false
  //int boost_time = doc["boost_time"]; // 300
  //const char* child_lock = doc["child_lock"]; // "UNLOCK"
  int comfort_temperature = doc["comfort_temperature"]; // 21
  int current_heating_setpoint = doc["current_heating_setpoint"]; // 21
  int eco_temperature = doc["eco_temperature"]; // 20
  const char* force = doc["force"]; // "normal"
  //int linkquality = doc["linkquality"]; // 66
  float local_temperature = doc["local_temperature"]; // 18.5
  //int local_temperature_calibration = doc["local_temperature_calibration"]; // -2
  //int max_temperature = doc["max_temperature"]; // 23
  //int min_temperature = doc["min_temperature"]; // 5
  int position = doc["position"]; // 100
  const std::string preset = doc["preset"]; // "comfort"
  //const char* system_mode = doc["system_mode"]; // "auto"
  //const char* week = doc["week"]; // "5+2"
  //const char* window_detection = doc["window_detection"]; // "OFF"

  writeNxt("page RoomTemp");
  getTemperatureFromWeb(room, 6);
  setText(7, std::to_string(local_temperature));
  setText(8, std::to_string(position));
  setNumber(0, current_heating_setpoint);
  setNumber("h0", current_heating_setpoint);
  bool mode = (preset == "comfort") ? true : false;
  // mode true = comfort
  if (mode){
    writeNxt("click r1,1");
  } else {
    writeNxt("click r0,1");
  };
  

}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 17,16);
  //SerialBT.begin("nspanel");
  //SerialBT.println(F("Booting"));
  WiFi.setHostname("nextion");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //SerialBT.println(F("Connection Failed! Rebooting..."));
    delay(5000);
    ESP.restart();
  }

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
      //SerialBT.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      //SerialBT.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    // .onError([](ota_error_t error) {
    //   SerialBT.printf("Error[%u]: ", error);
    //   if (error == OTA_AUTH_ERROR) SerialBT.println("Auth Failed");
    //   else if (error == OTA_BEGIN_ERROR) SerialBT.println("Begin Failed");
    //   else if (error == OTA_CONNECT_ERROR) SerialBT.println("Connect Failed");
    //   else if (error == OTA_RECEIVE_ERROR) SerialBT.println("Receive Failed");
    //   else if (error == OTA_END_ERROR) SerialBT.println("End Failed");
    // });

  ArduinoOTA.begin();

  //SerialBT.println("Ready");
  //SerialBT.print("IP address: ");
  //SerialBT.println(WiFi.localIP());

  // NS Panel Specifics
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  pinMode(4, OUTPUT);  // Screen reset. High = on?
  digitalWrite(4, LOW); // Switch on screen, LOW = on.
  pinMode(key1, INPUT_PULLUP);
  pinMode(key2, INPUT_PULLUP);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  adcAttachPin(ntc);
  analogSetClockDiv(255);

  if(!SPIFFS.begin()){
       //SerialBT.println(F("An Error has occurred while mounting SPIFFS"));
       //SerialBT.println(F("Did you upload the data directory that came with this?"));
       return;
  } 

  MDNS.begin(host);
  //SerialBT.print(F("http://"));
  //SerialBT.print(host);
  //SerialBT.println(F(".local"));

  //SERVER INIT
  server.on("/", HTTP_POST, [](){ 
    //SerialBT.println(F("Successfully updated Nextion!\n"));
    // Redirect the client to the success page after handeling the file upload
    server.sendHeader(F("Location"),F("/success.html"));
    server.send(303);
    doWeather();
    return true;
    }, 
    handleFileUpload);

  // receive fileSize once a file is selected (Workaround as the file content-length is of by +/- 200 bytes. Known issue: https://github.com/esp8266/Arduino/issues/3787)
  server.on("/fs", HTTP_POST, [](){
    fileSize = server.arg(F("fileSize")).toInt();
    server.send(200, F("text/plain"), "");
  });

  server.on("/reboot", HTTP_POST, [](){
    server.send(200, F("text/plain"), "Rebooting...");
    delay(2000);
    ESP.restart();
  });

  // called when the url is not defined here
  // use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, F("text/plain"), F("FileNotFound"));
  });

  server.begin();
  //SerialBT.println(F("\nHTTP server started"));    

  // NTP
  timeClient.begin();
  timeClient.update();
  while (!timeClient.isTimeSet()) {
    tone(21,4186,100);
    delay(1000);
  };
  writeNxt("page 0");
  writeNxt("vis michaelfish,0");
  doInternalTemperature();
  doWeather();
}

void loop() {
  ArduinoOTA.handle();
  timeClient.update();  // Has it's own rate limiter, so call with abandon
  server.handleClient();
  if (Serial2.available()) {
    handleSerialInput();
  };
  

  // TODO:
  //  A timer which unless reset shows a sad face to indicate the ESP32 has hung
  //  Some kind of flashing alarm looking light
  // 

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    // Every 5 seconds
    previousMillis = millis();
    updateSwitchStatus("hw", "p37");
    updateSwitchStatus("ch", "p0");
  }

  if (millis() - fiveminsmillis > 300000) {
    // Every five minutes
    fiveminsmillis = millis();
    doOutsideTemperature();
    doInternalTemperature();
    //doHWC();
    //doWeather();
  };

  if (millis() - minutemillis > 60000) {
    // Once a minute
    minutemillis = millis();
    //doOutsideTemperature();
    //doInternalTemperature();
  }

  if (currentMillis - hourmillis >= 3600000){
    // Every hour
    hourmillis = millis();
    doWeather();
  };

  doButtonPress();
}
