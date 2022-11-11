/*
 * WELCOME TO THE ESP32-CAM CODE TO BE USED WITH KILOCAM V1 and V2 boards
 * 
 * You do not need to make modifications to this code. Simply follow the instructions at the link
 * below and upload this code to an ESP32-CAM board that you wish to use with your KiloCam V1 or V2 boards.
 * 
 * Please note that this code does not allow for automatic timestamping of images. That functionality
 * was added in KiloCam V3 and newer boards. Images saved using this code will be assigned a number, and 
 * timestamps can be added manually after data collection using the deployment date and capture interval. 
 * 
 * NOTE: This code can still be used on KiloCam V3 boards, but is less power efficient and timestamping
 * functionality will be lost. 
 * 
 * TROUBLESHOOTING NOTE: If your images appear to have grey lines, or if part of an image is missing, 
 * increase the delay in the "RunCamera" function in your KiloCam V1 V2 code file to allow more time before
 * power to the ESP32-CAM is cut. Alternatively, try using a microSD card with faster write speeds. 
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

int NewImageCounter = 0; 
int PreviousPictureNumber; 
int NowPictureNumber; 

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  
  Serial.println();
  Serial.println("Booting...");
  
  pinMode(4, INPUT);              //GPIO for LED flash
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);  //disable pin hold if it was enabled before sleeping

  WiFi.mode(WIFI_MODE_NULL); // Disable onboard wifi antenna

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
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
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
  sensor_t * s = esp_camera_sensor_get();
  
  //s->set_brightness(s, -2);     // -2 to 2
  //s->set_contrast(s, 0);       // -2 to 2
  //s->set_saturation(s, 0);     // -2 to 2
  //s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 0);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 0);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  //s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  //s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  //s->set_ae_level(s, 0);       // -2 to 2
  //s->set_aec_value(s, 300);    // 0 to 1200
  //s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  //s->set_agc_gain(s, 0);       // 0 to 30
  //s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  //s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  //s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  //s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  //s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  //s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
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

  // First read the SD to see if the ImageCounter file exists... 
  File counterread = SD_MMC.open("/ImageCounter.txt", FILE_READ); 
  // If the file cannot be opened or doesn't exist, write a new one with the value 1
  if (!counterread) { 
    counterread.close();
    Serial.println("ImageCounter file does not exist, writing a new one.");
    File counterwrite = SD_MMC.open("/ImageCounter.txt", FILE_WRITE); // write a new ImageCounter file
    if (!counterwrite) {
      Serial.println("Cannot write new ImageCounter file on SD."); 
    return;
     }
    if (counterwrite.print(NewImageCounter)) {
    Serial.println("ImageCounter was restarted.");
  } else {
    Serial.println("Failed writing new ImageCounter to SD");
  }
    counterwrite.close(); 
  }

  if (counterread.available()) {
    Serial.println("ImageCounter file exists and was opened.");
    PreviousPictureNumber = counterread.parseInt(); 
    Serial.print("Previous picture number read from card is: "); 
    Serial.println(PreviousPictureNumber); 
  }
  counterread.close(); 

  // At this point we should have a value for PreviousPictureNumber either way. 
  // This is the value of either the previous picture, or of our restarted counter. 
  // Now we increment it by 1 to represent the number of the current picture, we use it, then later we write it to memory. 

  NowPictureNumber = PreviousPictureNumber += 1;
  Serial.print("After incrementing, current picture number will be: "); 
  Serial.println(NowPictureNumber);  

  // Take picture and obtain camera buffer  
  camera_fb_t * fb = NULL;

  // Delay 1 second or more to improve exposure of camera
  delay(2000);
  
  // Obtain buffer 
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  delay(250);

  // Path where new picture will be saved in SD Card
  String path = "/IMG" + String(NowPictureNumber) +".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());

  // Create new file
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
  }
  file.close();
  
  //return camera frame buffer
  esp_camera_fb_return(fb);
  Serial.printf("Picture file saved to SD as: %s\n", path.c_str());

  // After saving our image file, write the existing picture number to our file on the SD card 
 File counterwrite = SD_MMC.open("/ImageCounter.txt", FILE_WRITE); 
  if (!counterwrite) {
    Serial.println("Opening ImageCounter file on SD failed"); 
    return;
  }

  // Now write the current picture number to the ImageCounter file on the SD
  if (counterwrite.print(NowPictureNumber)) {
    Serial.print("ImageCounter write success. ImageCounter number saved to SD was: ");
    Serial.println(NowPictureNumber); 
  } else {
    Serial.println("Failed writing ImageCounter to SD");
  }
  counterwrite.close(); 
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  
  delay(500);
  Serial.println("Going to sleep now");
  delay(500);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  
}
