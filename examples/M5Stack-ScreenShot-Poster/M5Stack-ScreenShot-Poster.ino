
#include <WiFi.h>
#include <M5Stack.h> // https://github.com/tobozo/ESP32-Chimera-Core
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater
#include <ImgurUploader.h>

#include "AmigaRulez.h"
#include "assets.h"

//#define IMGUR_CLIENT_ID "your-imgur-client-id"

#ifndef IMGUR_CLIENT_ID
  #error "No client ID defined, see how to get one at https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a"
#endif

ImgurUploader imgurUploader( IMGUR_CLIENT_ID );


// animaton for the ScreenShot
void AmigaBallInit() {
  amigaBallConfig.Width = tft.width();
  amigaBallConfig.Height = tft.height();
  amigaBallConfig.Wires = 8;
  amigaBallConfig.ScaleRatio = 5; // bigger value means smaller ball
  AmigaBall.init( amigaBallConfig );
}


void checkWifi() {
  if(WiFi.status() != WL_CONNECTED ) {
    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.macAddress());
    WiFi.begin();
    while(WiFi.status() != WL_CONNECTED) {
      //log_e("Not connected");
      delay(1000);
    }
    log_w("Connected!");
    Serial.print("Connected to "); Serial.println(WiFi.SSID());
    Serial.print("IP address: "); Serial.println(WiFi.localIP()); 
    Serial.println("");
  }
}


// example for posting image stored in a byte array (see assets.h)
void postBytesArray() {
  checkWifi();
  int ret = imgurUploader.uploadBytes( Miaou_Goldwyn_Mayer_jpg, Miaou_Goldwyn_Mayer_jpg_len, "Miaou_Goldwyn_Mayer.jpg", "image/jpeg" );
  if( ret > 0 ) {
    M5.Lcd.qrcode( imgurUploader.getURL() );
    delay( 10000 );
    AmigaBallInit();
  }
}

// example for taking a screenshot to the SD + uploading from filesystem
void snapAndPost() {
  checkWifi();
  M5.ScreenShot.snap();
  int ret = imgurUploader.uploadFile( M5STACK_SD, M5.ScreenShot.fileName );
  if( ret > 0 ) {
    M5.Lcd.qrcode( imgurUploader.getURL() );
    delay( 10000 );
    AmigaBallInit();
  }
}


void setup() {
  M5.begin( true, true, true, false, true ); // don't start I2C, init ScreenShot service
  M5.update();
  if( M5.BtnA.wasPressed() ) {
    Serial.println("Will Load menu binary");
    updateFromFS();
    ESP.restart();
  }
  WiFi.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor( GREEN );
  M5.Lcd.println("imgur.com Uploader");
  M5.Lcd.println();
  M5.Lcd.println();
  M5.Lcd.setTextColor( GREENYELLOW );
  M5.Lcd.println("Buttons upload type:");
  M5.Lcd.println();
  M5.Lcd.println();
  M5.Lcd.setTextColor( YELLOW );
  M5.Lcd.println("B = Filesystem/ScreenShot");
  M5.Lcd.println();
  M5.Lcd.println("C = Flash rom/Byte Array");
  delay(10000);
  AmigaBallInit();
}


unsigned long lastcheck = millis();

void loop() {
  if( lastcheck + 100 < millis() ) {
    M5.update();
    if( M5.BtnB.wasPressed() ) {
      // screenshot to filesystem, then upload from filesystem
      snapAndPost();
    }
    if( M5.BtnC.wasPressed() ) {
      // upload from bytes array stored in flash rom
      postBytesArray();
    }
    lastcheck = millis();
  }
  AmigaBall.animate(1, false);
}
