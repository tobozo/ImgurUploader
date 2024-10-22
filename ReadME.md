 
[![License: MIT](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/tobozo/M5Stack-SD-Updater/blob/master/LICENSE)


[![arduino-library-badge](https://www.ardu-badge.com/badge/ImgurUploader.svg?)](https://www.ardu-badge.com/ImgurUploader)
 

### ImgurUploader

imgur.com image/video upload library for ESP32



❓ What is it?
--------------
This library will let you upload images or videos to imgur.com using WiFi.
It can read from a file (SD, SD_MMC, SPIFFS) or from a byte array.
This library is heavily inspired from the Arduino Imgur library [https://github.com/tinkerlog/Imgur](https://github.com/tinkerlog/Imgur)

It has been totally rewritten to fit the ESP32's WiFiClientSecure requirements, extended to different mime types and sources, and finally adapted to imgur's API v3.

Big thanks to [@tinkerlog](https://github.com/tinkerlog/) for the inspiration.


🏗️ Install
----------

For the lazy: search for "imgurUploader" in the Arduino library manager and click "install".

![image](https://user-images.githubusercontent.com/1893754/71968564-79541400-3205-11ea-83fd-497cf01d1e22.png)


For the brave: download the the [latest release](https://github.com/tobozo/ImgurUploader/releases) from this repository and use the import option from the Arduino IDE sketch menu.

![image](https://user-images.githubusercontent.com/1893754/71968742-c89a4480-3205-11ea-90bd-674bb88cb9a0.png)




🛠️ Usage
--------

1) **[Get an imgur client ID](https://medium.com/@microaeris/getting-started-with-the-imgur-api-4e96c352658a)**

2) Set the client ID in the sketch

    ```C
    #define IMGUR_CLIENT_ID "your-imgur-client-id"
    ```


3) Create an ImgurUploader instance

    ```C
    ImgurUploader imgurUploader( IMGUR_CLIENT_ID );
    ```


4) Connect to the WiFi

    ```C
    WiFi.connect();
    ```


5) Send the data

    ```C
    int ret = imgurUploader.uploadFile( SD, "/pic.jpg" );
    // or
    int ret = imgurUploader.uploadBytes( byteArray, arrayLength, "pic.jpg", "image/jpeg" );
    // or
    int ret = imgurUploader.uploadStream( 12345678, &writeStreamCallback, "pic.jpg", "image/jpeg" );  
    ```


6) Get the resulting imgur.com URL

    ```C
    if( ret > 0 ) {
      Serial.println( imgurUploader.getURL() );
    } else {
      Serial.println( "Upload failed" );
    }
    ```


Callbacks
---------

  - Upload progress: `setProgressCallback( &yourProgressFunction )` where `void yourProgressFunction( byte progress )` prints a value between 0 and 100

  - Stream Write: `imgurUploader.uploadStream( streamSize, &writeStreamCallback )` where `writeStreamCallback( Stream* client )` writes the image data by chunks (total size must be `streamSize` bytes exactly!)
  
  
  
