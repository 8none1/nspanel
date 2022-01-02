#include "wificreds.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Servo.h>
#include <ESPNexUpload.h>

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


// NexUploader stuff #####

int fileSize  = 0;
bool result   = true;
ESPNexUpload nextion(115200);
WebServer server(80);

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
    nextion.end();
    
    Serial.println("");
    //Serial.println(nextion.statusMessage);
    return true;
  }
return false;
}
// End Uploader stuff #######################


void setup() {
  Serial.begin(115200);
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

    Serial.println(F("Succesfully updated Nextion!\n"));
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


    
}

void loop() {
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 2500) {
    previousMillis = millis();
    //tone(21,4186, 100);
  }
    server.handleClient();
}
