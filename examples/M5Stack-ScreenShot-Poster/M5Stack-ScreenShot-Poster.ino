#include <WiFi.h>
#include <M5Stack.h> // https://github.com/tobozo/ESP32-Chimera-Core
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater
#include <ImgurUploader.h>
#include "AmigaRulez.h"
#include "assets.h"

// MANDATORY: put your ID here and uncomment
//#define IMGUR_CLIENT_ID "your-imgur-client-id"

// OPTIONAL (if the ESP32 can reuse a previous connection)
//#define WIFI_SSID "your-wifi-password"
//#define WIFI_PASS "your-wifi-password"

#ifndef IMGUR_CLIENT_ID
  #error "No client ID defined, see how to get one at https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a"
#endif


ImgurUploader imgurUploader( IMGUR_CLIENT_ID );


void checkWifi() {
  // some routers do MAC filtering
  Serial.printf("Current MAC Address: %s (must be allowed by your router)\n", String(WiFi.macAddress()).c_str());
  if(WiFi.status() != WL_CONNECTED ) {
    WiFi.mode(WIFI_STA);
    #if defined(WIFI_SSID) && defined(WIFI_PASS)
      WiFi.begin( WIFI_SSID, WIFI_PASS );
    #else
      WiFi.begin();
    #endif

    while(WiFi.status() != WL_CONNECTED) {
      Serial.println("Attempting to connect...");
      delay(1000);
    }
  }
  Serial.printf("Connected to %s\n", String(WiFi.SSID()).c_str());
  Serial.printf("IP address: %s\n", String(WiFi.localIP()).c_str()); 
}

// animaton for the ScreenShot
void AmigaBallInit() {
  amigaBallConfig.Width = tft.width();
  amigaBallConfig.Height = tft.height();
  amigaBallConfig.Wires = 8;
  amigaBallConfig.ScaleRatio = 5; // bigger value means smaller ball
  AmigaBall.init( amigaBallConfig );
}




// progress callback example function will be fired on each write during upload
byte lastprogress = 0;
void progressCallback( byte progress ) {
  if( lastprogress != progress ) {
    lastprogress = progress;
    Serial.printf("Upload progress: %d\n", progress);
  }
}

/*
// Outsider function to write data, must send exactly the amount of bytes given 
// as first arg to uploadStream()
void writeStreamCallback( Stream* client ) {
  size_t buffLen = 512;
  char buffer[buffLen];
  while( cameraReadBytes( buffer, &buffLen) ) {
    client->write( buffer, buffLen );
  }
}

// Example for posting image by chunks from the outside of the main class.
// tradeoff: the size of the written bytes must be know before write starts
void postStream() {
  checkWifi();
  ret = imgurUploader.uploadStream( 12345678, &writeStreamCallback, "blah.jpg", "image/jpeg" );  
  if( ret > 0 ) {
    M5.Lcd.qrcode( imgurUploader.getURL() );
    delay( 10000 );
    AmigaBallInit();
  }
}
*/

// example for posting image stored in a byte array (what's in assets.h)
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
    M5.Lcd.qrcode( imgurUploader.getURL(), 50, 10, 220, 2);
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
  WiFi.begin(); // pre-connect
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
  M5.Lcd.println("A = Show last QR Code");
  M5.Lcd.println();
  M5.Lcd.println("B = Filesystem/ScreenShot");
  M5.Lcd.println();
  M5.Lcd.println("C = Flash rom/Byte Array");
  delay(10000);
  AmigaBallInit();
  // optional, attach a progress callback function
  imgurUploader.setProgressCallback( &progressCallback );
}


unsigned long lastcheck = millis();

void loop() {
  if( lastcheck + 100 < millis() ) {
    M5.update();
    if( M5.BtnA.wasPressed() ) {
      if( String( imgurUploader.getURL() ) != "" ) {
        Serial.printf("URL: %s\n", imgurUploader.getURL() );
        M5.Lcd.qrcode( imgurUploader.getURL(), 50, 10, 220, 2);
        delay(10000);
        AmigaBallInit();
      }
    }
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
