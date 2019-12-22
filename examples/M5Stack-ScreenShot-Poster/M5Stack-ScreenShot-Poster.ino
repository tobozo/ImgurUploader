
#include <WiFi.h>
#include <M5Stack.h> // https://github.com/tobozo/ESP32-Chimera-Core
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater
#include <ImgurUploader.h>

//#define tft M5.Lcd

#include "AmigaRulez.h"

//#define IMGUR_CLIENT_ID "your-imgur-client-id"

#ifndef IMGUR_CLIENT_ID
  #error "No client ID defined, see how to get one at https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a"
#endif

ImgurUploader imgurUploader( IMGUR_CLIENT_ID );


static void snapAndPost() {
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
  M5.ScreenShot.snap();
  int ret = imgurUploader.uploadFile( M5STACK_SD, M5.ScreenShot.fileName );
  if( ret > 0 ) {
    M5.Lcd.qrcode( imgurUploader.getURL() );
    delay( 10000 );
  }
}


void setup() {
  M5.begin( true, true, true, false, true ); // don't start Serial
  M5.update();
  if( M5.BtnA.wasPressed() ) {
    Serial.println("Will Load menu binary");
    updateFromFS();
    ESP.restart();
  }

  amigaBallConfig.Width = tft.width();
  amigaBallConfig.Height = tft.height();
  amigaBallConfig.Wires = 8;
  amigaBallConfig.ScaleRatio = 5; // bigger value means smaller ball
  AmigaBall.init( amigaBallConfig );
  WiFi.begin();
}


unsigned long lastcheck = millis();

void loop() {
  if( lastcheck + 100 < millis() ) {
    M5.update();
    if( M5.BtnB.wasPressed() ) {
      snapAndPost();
    }
    lastcheck = millis();
  }
  AmigaBall.animate(1, false);
}
