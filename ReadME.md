 

### ImgurUploader

imgur.com image upload library for ESP32


What is it?
-----------
This library will let you upload images to imgur.com using WiFi.
It can read from a file (SD, SD_MMC, SPIFFS) or from a byte array.
This library is heavily inspired from the Arduino Imgur library [https://github.com/tinkerlog/Imgur](https://github.com/tinkerlog/Imgur)

It has been totally rewritten to fit the ESP32's WiFiClientSecure requirements, extended to different mime types and sources, and finally adapted to imgur's API v3.

Big thanks to [@tinkerlog](https://github.com/tinkerlog/) for the inspiration.


Install
-------
Download the zip file and install from the menu Sketch / Include Library / Add .ZIP Library


Usage
-----

1) [Get an imgur client ID](https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a)

2) Set the client ID in the sketch

    #define IMGUR_CLIENT_ID "your-imgur-client-id"


3) Create an ImgurUploader instance

    ImgurUploader imgurUploader( IMGUR_CLIENT_ID );


4) Connect to the WiFi

    WiFi.connect()


5) Send the data

    int ret = imgurUploader.uploadFile( SD, "/pic.jpg" );
    // or
    int ret = imgurUploader.uploadBytes( byteArray, "pic.jpg", "image/jpeg" );


6) Get the URL

    if( ret > 0 ) {
      Serial.println( imgurUploader.getURL() );
    } else {
      Serial.println( "Upload failed" );
    }

