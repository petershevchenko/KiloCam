# KiloCam
A repository for code with supporting documentation for KiloCam, the low-cost habitat monitoring camera. The motivation for KiloCam is to be a solder-free alternative to my other camera (CoralCam, https://www.sciencedirect.com/science/article/pii/S2468067219300537) that is also more affordable. KiloCam is a low-power camera designed to fit into a standard GoPro Hero4 underwater housing with a 752555 lithium battery. To date, KiloCam has been used to monitor coral reefs, agriculture, streams, plant phenology, tidal flats, marine predators, bird nests and more, all by users of many ages. 

IMPORTANT NOTE: Certain version of the master ESP32 board definitions (arduino-esp32) are known to cause issues with SD card mounting. DO NOT use version 2.0.0. I have found the most recent, stable version to be 2.0.4 when used in combination with Arduino IDE version 1.8.13. This is an older version of the Arduino IDE but I prefer that it allows you to select the correct speed and voltage for Arduino boards (KiloCam is based on the same chip as an Arduino Pro Mini 3.3V).  

For a video build guide of how to assemble and code your KiloCam, check out the video linked below. Be sure to check the video description for additional notes and always use the most up-to-date code files available in this repo. 

Build video link: https://www.youtube.com/watch?v=vf54ca9IuP4&t=11s

## Major Update - March 2024: 
I have continued to revise and improve the code for KiloCam, as well as the boards themselves. The newest code was released today (March 3, 2024) and represents a considerable improvement. Users can now just program the ESP32-Cam once with the supplied code file "KiloCam_V3_LT_ESP32Code_03042024.ino", then program the time on their KiloCam board, and then upload the file "KiloCam_V3_LT_03042024.ino" to their KiloCam board. All user settings, like the timing and number of photos to capture, are now adjusted towards the top of the KiloCam file and are communicated to the ESP32-Cam by the KiloCam board. Upon wake, KiloCam tells the ESP32-Cam how many photos to take, and for each photo the ESP32-Cam requests a new data packet, including timestamp, from the KiloCam board. These changes: 
- Make the code easier to use: Upload the ESP32-Cam code to that board just once, then make all other changes (including number of photos per burst) in the main KiloCam code file. 
- Allow for each image to have its own unique timestamp. This is important when the increment between photos within a single burst increases over time as the number of images on the SD card reaches into the thousands. 
- If controlling an external LED with KiloCam, like the BlueRobotics Lumen, you can easily modify this code to turn the LED on only when a photo is being captured, not when it is being saved. Contact me if you want example code - this is a common operation with KiloCams used to monitor coral spawning. 
- Future KiloCam v3 PCBs (the "LT" version) will come standard with integrated light (L) and temperature sensors (T). This code automatically collects average values from these sensors and adds them to the filename for each photo alongside the timestamp - new values are collected for EACH photo. The code will work with V3 boards that do not have these sensors. On future boards a file that reads "IMG_20240303112829_L297_T17.jpg" will represent a photo collected at a light level of 297 (0 is a dark room, 60 is a dimly lit room), and a temperature of 17 degrees celsius. These data are intended to be supplementary, but could be used to guide intelligent sampling strategies by advanced users. 

## KiloCam V3 Notes:  
Starting in January 2022 KiloCam V3 boards became available. These boards use serial communications between the ESP32-CAM and the KiloCam PCB to 1) allow automatic timestamping of images with the time and date of the KiloCam RTC, and 2) improve overall power efficiency by letting the ESP32-CAM tell KiloCam when an image has been saved. 

KiloCam V3 boards can be used with any ESP32-CAM as long as it is running the appropriate code file. The appropriate code files are: 
- For KiloCam V3 use "KiloCam_V3_KiloCamCode_DATE": To be installed onto the KiloCam PCB using an FTDI programmer set to 3.3V and the Arduino IDE set for an "Arduino Pro Mini 3.3V 8MHz". 
- For ESP32-CAM use "KiloCam_V3_ESP32Code_DATE": To be installed onto the ESP32-Cam board with an FTDI programmer set to 5V and connected to the 5V pin of the ESP32-CAM board. For specific instructions on how to do this, check out the great tutorial at: https://randomnerdtutorials.com/program-upload-code-esp32-cam/

## KiloCam V1 and V2 Notes: 
Ensure you are using the correct code files for your boards as these differ significantly from KiloCam V3 code files. The appropriate code files are: 
- For KiloCam V1 and V2 PCBs use "KiloCam_V2_KiloCamCode_DATE": To be installed onto the KiloCam PCB using an FTDI programmer set to 3.3V and the Arduino IDE set for an "Arduino Pro Mini 3.3V 8MHz". 
- For ESP32-CAM boards used with KiloCam V1 and V2 use "KiloCam_V2_ESP32Code_DATE": To be installed onto the ESP32-Cam board with an FTDI programmer set to 5V and connected to the 5V pin of the ESP32-CAM board. For specific instructions on how to do this, check out the great tutorial at: https://randomnerdtutorials.com/program-upload-code-esp32-cam/

