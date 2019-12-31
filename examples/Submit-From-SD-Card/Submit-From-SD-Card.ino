#include <WiFi.h>
#include <SD.h>
#include <ImgurUploader.h>
#include "assets.h"

// MANDATORY: put your ID here and uncomment
//#define IMGUR_CLIENT_ID "your-imgur-client-id"

// OPTIONAL (if the ESP32 can reuse a previous connection)
//#define WIFI_SSID "your-wifi-password"
//#define WIFI_PASS "your-wifi-password"

#ifndef IMGUR_CLIENT_ID
  #error "No client ID defined, see how to get one at https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a"
#endif


#define TFCARD_CS_PIN 4

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


// optional progress callback example function will be fired on each write during upload
byte lastprogress = 0;
void progressCallback( byte progress ) {
  if( lastprogress != progress ) {
    lastprogress = progress;
    Serial.printf("Upload progress: %d\n", progress);
  }
}


void setup() {

  Serial.begin( 115200 );
  Serial.println("Started");
  //SD.begin();
  SD.begin( TFCARD_CS_PIN, SPI, 40000000);

  checkWifi();

  // optional, attach an upload progress callback function
  imgurUploader.setProgressCallback( &progressCallback );

  int ret = imgurUploader.uploadFile( SD, "/jpg/Miaou-Goldwyn-Mayer.jpg" );

  if( ret > 0 ) {
    // yay !
    Serial.printf("URL: %s\n", imgurUploader.getURL() );

  } else {
    // oops ?
    Serial.println("imgur uploader failed, check your connection or your client ID");

  }

}




void loop() {

}

