# nspanel

![image](https://user-images.githubusercontent.com/6552931/148619064-476c0c47-9504-4862-8d74-d1ddc394958e.png)

# Colour Picker Tool
![image](https://user-images.githubusercontent.com/6552931/148950031-4a6f1652-7c95-4f68-acee-02f427a1b061.png)

This repo also includes a tool to help you find the right colours.  It offers three sliders for red, green and blue and calculates the 565 decimal colour number that the Nextion devices use.
To use, flash the colour picker tool to your Nextion device, there is no microcontroller code needed, everything is done on the Nextion.

# History

Port my Raspi based smart thermostat to Nextion

My original smart thermostat was built on a Rasp Pi and a 3.5" touchscreen and used a custom UI library written in Python.  I was pretty happy with it, but now I'm going to try and port it over to Nextion and use the Sonoff NSPanel instead. 

The reason for this is that the NSPanel is a much nicer package with PSU, enclosure, etc etc. 

Original code here:  https://github.com/8none1/thermostat

Some notes I've made along the way:

* In the "preinitialize event" section of page0 add:
`bauds=115200`
* Nextion say that software serial is not supported
* The NS Panel has already correctly connected Serial2 of the ESP to the serial ports of the Nextion display.  GPIO16 = Rx Panel, GPIO 17 = Tx panel
* To get PSRAM to work we need to tell the compiler about the GPIOs that are being used.  I don't understand how to do that yet, but this is involved: https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-reference/kconfig.html#psram-clock-and-cs-io-for-esp32-dowd
 * Looks like you would have to rebuild all the arduino libs with the new settings.  The file that needs to be edited is .arduino15/packages/esp32/hardware/esp32/2.0.2/tools/sdk/esp32/sdkconfig but that's static as it comes with the pre-built files for Arduino support from the ESP32 SDK.  To make changes you would have to rebuild arduino support.  Not impossible, but a job for another day.
```
# PSRAM clock and cs IO for ESP32-DOWD
#
CONFIG_D0WD_PSRAM_CLK_IO=5
CONFIG_D0WD_PSRAM_CS_IO=9
# end of PSRAM clock and cs IO for ESP32-DOWD
```
 * 
 
