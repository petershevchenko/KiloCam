/* 
 *  WELCOME TO THE CODE FOR ESP32-CAM USED WITH KILOCAM V3 CAMERAS
 *  
 *  NOTE: Power your board with 5V (to the 5V pin) from an FTDI programmer while uploading this code. Remove the SD card.
 *  
 *  This code is only compatible with KiloCam V3+ boards. Check your PCB for its version.
 *  KiloCam V3 boards are improved with serial communications between the KiloCam PCB
 *  and the ESP32-CAM board attached to them. The code below allows the ESP32-CAM to tell
 *  KiloCam PCBs when it is ready to capture an image, store the image with a timestamp
 *  provided by the KiloCam board, and notifies the KiloCam board when the image is saved
 *  so that power to the camera can be cut. 
 *  
 *  This code will store four images to your SD card for each KiloCam capture interval.
 *  Each image captured about 1 second apart in a "burst" of photos taken by the ESP32-Cam. 
 *  Go to the line commented "CHANGE BURST COUNT HERE" to adjust this. Change the npics value from 1 to 5. 
 *  No other modifications are needed to this code. Simply upload it to your ESP32-CAM board
 *  that you plan to use with a KiloCam V3 or newer board. All timing modifications should be 
 *  made to the KiloCam code file, not this one. 
 *  
 *  NOTE: It has been documented that certain versions of ESP32 board manager cause SD
 *  cards to not properly mount (open) with ESP32 boards. Please use Arduino version 1.8.13
 *  and arduino-esp32 version 2.0.4 or later. This can be installed by following the
 *  instructions at the link below and, in Arduino -> Boards Manager, selecting version 2.0.4. 
 *  
 *  Link: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide
 *  
 *  To test operation of your ESP32-CAM after installing this code, disconnect the GND pin from IO0,
 *  open Serial Monitor, and reset the board. Set baud to 57600. After booting, the ESP32-CAM
 *  will send a "P" signal over serial. Using the serial monitor, send "H120221111120000". 
 *  This will save an image with the timestamp 2022-11-11 12:00:00" and a "Q" will be reported
 *  by the ESP32-CAM when it is ready for power to be cut. 
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

char PAYLOAD[18];
char TIMESTAMP[15];
int npics=4;

void setup() {

  rtc_gpio_hold_en(GPIO_NUM_4);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(57600);
  
  //Serial.println();
  Serial.println("Booting...");
  
  pinMode(33, OUTPUT);   // GPIO for on-board LED flash
  pinMode(4, OUTPUT); //GPIO for LED flash
  
  delay(1000); //startup delay before sending on signal

  // Get timestamp and action flag 
  Serial.println("P"); // Initiate contact to kilocam with by sending "P" via serial
  
  // Wait for response 
  while(Serial.available() == 0) {
    // No actions here 
    }; 

  // Parse message
  int i = 0; 
  if(Serial.available()>0) {    
     while (Serial.available()>0){
       char inByte = Serial.read();
       delay(10);
       
       if (inByte != '\n'){
         PAYLOAD[i] = inByte;
         i++; //incrementing through the bytes in the message 
        }
       else {
        PAYLOAD[i] = 0;
        }
       }                   
    }

  // Generate global variables for use later
  char ACTION = PAYLOAD[1]; // Pull action from payload byte 1
  strcpy(TIMESTAMP, &PAYLOAD[2]); // Pull timestamp the from the rest of the packet
  
  rtc_gpio_hold_dis(GPIO_NUM_4);  //disable pin hold if it was enabled before sleeping

  WiFi.mode(WIFI_MODE_NULL); // Disable onboard wifi antenna to save power

  // Initialize camera 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; //Can also be: YUV422,GRAYSCALE,RGB565,JPEG
  config.grab_mode = CAMERA_GRAB_LATEST; // CRITICAL! If set to CAMERA_GRAB_WHEN_EMPTY you get old images
  
  // Define frame size, image quality, and number of pictures saved in the frame buffer. 

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10; //10-63, lower number is higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set camera settings (note that this can be tricky, best to leave as-is. 
  // Use ESP32CAM examples > Camera > CameraWebServer to find the best settings for your situation
  sensor_t * s = esp_camera_sensor_get();
  
  s->set_brightness(s, 0);     // -2 to 2
  //s->set_contrast(s, 2);       // -2 to 2
  //s->set_saturation(s, 0);     // -2 to 2
  //s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable auto exposure!
  //s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  //s->set_ae_level(s, 0);       // -2 to 2
  //s->set_aec_value(s, 300);    // 0 to 1200
  //s->set_gain_ctrl(s, 0);      // 0 = disable , 1 = enable
  //s->set_agc_gain(s, 0);       // 0 to 30
  //s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 1);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  //s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  // s->set_hmirror(s, 1);        // 0 = disable , 1 = enable
  //s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  //s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  //s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  
  Serial.println("Starting SD Card");
  
  if(!SD_MMC.begin("/sdcard", true)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }  

  // Take picture and obtain camera buffer  
  camera_fb_t * fb = NULL;

  // Delay 1 second or more to improve exposure of camera. Adjust as needed. 
  delay(2000);
  
  if (ACTION == '1') {
    Serial.println("starting LED");
    digitalWrite(4, HIGH);
    }
    else
    {
      Serial.println("no starting LED");
    }
    delay(100);
  // Create Path where new picture will be saved in SD Card
  // Take two extra pics and throw away the first two

    for (int I = 0; I < npics + 2; I++) {
      // Obtain buffer 
      fb = esp_camera_fb_get();  
      if(!fb) {
        Serial.println("Camera capture failed");
        return;
      };
      
      //path_burst=path;
      char path[30] = "/IMG_";
      strcat(path,TIMESTAMP);
      strcat(path,"_");
      //Istr[0]=char(I);
      static char buff[5];
      //strcat(path,Istr[0]);
      strcat(path,itoa(I-2,buff,10));
      strcat(path, ".jpg");
      
      fs::FS &fs = SD_MMC; 
      Serial.printf("Picture %i file name: %s\n", I-2, path);

      // Create new file if on third picture
    if(I>1) {
            
      File file = fs.open(path, FILE_WRITE);
      if(!file){
        Serial.println("Failed to open file in writing mode");
      } 
      else {
        file.write(fb->buf, fb->len); // OLD CODE: payload (image), payload length
        // New code below:
        // uint8_t framebuffer_static[32 * 1024];
        // memcpy(framebuffer_static, fb->buf,  fb->len);
        // file.write(framebuffer_static, fb->len);
      }
      file.close();
      Serial.printf("Picture %i file name written: %s\n", I-2, path);
    }
    else{
      delay(250);
    };
      
        //return camera frame buffer
    esp_camera_fb_return(fb);
    delay(250);
  
    }
   // Turn off LED flash now photo is taken
   Serial.println("stopping LED");
    
  digitalWrite(4, LOW);
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  rtc_gpio_hold_en(GPIO_NUM_4);
  
  delay(1000);
  Serial.println("Q"); // Send Kilocam "Q" character to initiate power down of ESP32-CAM
  delay(50);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  
}
