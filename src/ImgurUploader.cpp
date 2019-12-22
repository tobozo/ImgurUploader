/*

  Imgur Image Upload library for ESP32

  This library is inspired from Arduino Imgur library
  https://github.com/tinkerlog/Imgur
  It has been totally rewritten to fit the WiFiClientSecure requirements,
  extended to different mime types and sources, and adapted to imgur's API v3.

  MIT License

  Copyright (c) 2019 tobozo

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include "ImgurUploader.h"

#define IMGUR_UPLOAD_API_URL    "/3/image"
#define IMGUR_UPLOAD_API_DOMAIN "api.imgur.com"
#define IMGUR_URL_MASK          "https://imgur.com/%s"
#define BOUNDARY                "blah-blah-oz"
#define HEADER                  "--" BOUNDARY
#define FOOTER                  "--" BOUNDARY "--"

ImgurUploader::ImgurUploader(const char *appKey) : client(), appKey(appKey) { ; }


int ImgurUploader::uploadFile( fs::FS &fs, const char* path ) {
  sourceFile = fs.open( path );
  if( !sourceFile ) {
    log_e("Could not open path %s", path );
  }
  source = SOURCE_FILE;
  String fileName = sourceFile.name();
  size_t fileSize = sourceFile.size();
  String mimeType;
  if( fileName.endsWith(".jpg") ) {
    mimeType = "image/jpeg";
  } else if( fileName.endsWith(".png") ) {
    mimeType = "image/png";
  } else if( fileName.endsWith(".bmp") ) {
    mimeType = "image/x-windows-bmp";
  } else if( fileName.endsWith(".gif") ) {
    mimeType = "image/gif";
  } else {
    mimeType = "application/octet-stream";
  }
  return upload( fileSize, fileName.c_str(), mimeType.c_str() ); 
}


int ImgurUploader::uploadBytes( const uint8_t* _byteArray, size_t _arrayLen, const char* imageName, const char* imageMimeType  ) {
  source = SOURCE_BYTE_ARRAY;
  byteArray = (uint8_t*)_byteArray;
  arrayLen = _arrayLen;
  String mimeType;
  String fileName = String( imageName );
  if( fileName.endsWith(".jpg") ) {
    mimeType = "image/jpeg";
  } else if( fileName.endsWith(".png") ) {
    mimeType = "image/png";
  } else if( fileName.endsWith(".bmp") ) {
    mimeType = "image/x-windows-bmp";
  } else if( fileName.endsWith(".gif") ) {
    mimeType = "image/gif";
  } else {
    mimeType = "application/octet-stream";
  }
  return upload( arrayLen, fileName.c_str(), mimeType.c_str() ); 
}


int ImgurUploader::upload( size_t imageLen, const char* imageName, const char* imageMimeType ) {
  int ret = -1;
  int mimeTypeLen = strlen( imageMimeType );
  int nameLen = strlen( imageName );
  if (WiFi.status() != WL_CONNECTED) {
    log_e("WiFi Not connected!");
    return ret;
  }
  log_d("connecting ...");
  client.setCACert( NULL ); // YOLO Mode enabled
  if (!client.connect("api.imgur.com", 443)) {
    log_e("Connection failed!");
    return ret;
  }
  log_d("posting image ...");
  uint32_t length = 16 + 56 + nameLen + 3 + 16 + mimeTypeLen + 2 + imageLen + 2 + 18 + 2;
  client.println( "POST " IMGUR_UPLOAD_API_URL " HTTP/1.0" );
  client.print( "Authorization: Client-ID " );
  client.println( appKey );
  client.println( "Host: " IMGUR_UPLOAD_API_DOMAIN );
  client.println( "Content-Type: multipart/form-data; boundary=" BOUNDARY );
  client.print( "Content-Length: " );
  client.println( length );
  client.println();
  client.println( HEADER ); // 14+2
  client.print( "Content-Disposition: form-data; name=\"image\"; filename=\""); // 56
  client.print( imageName ); // nameLen
  client.println("\"" ); // 1+2
  client.print( "Content-Type: " ); // 14
  client.println( imageMimeType ); // mimeTypeLen
  client.println(); // +2
  sendImageData( &client );
  client.println(); // +2
  client.println( FOOTER ); // 16+2
  client.println(); // +2
  ret = readResponse();
  client.stop();
  log_d("connection closed");
  return ret;
}

#define IMGUR_BUFFSIZE 512

void ImgurUploader::sendImageData( WiFiClientSecure *client ) {
  uint8_t buf[IMGUR_BUFFSIZE];
  size_t packets = 0;
  switch( source ) {
    case SOURCE_FILE:
      {
        log_d("Using filesystem");
        while( (packets = sourceFile.read( buf, sizeof(buf))) > 0 ) {
          client->write( buf, packets );
          log_w("Sent %d bytes", packets);
        }
        sourceFile.close();
      }
    break;
    case SOURCE_BYTE_ARRAY:
      log_d("Using memory");
      packets = arrayLen;
      log_d("Byte array size: %d", packets );
      size_t index = 0;
      size_t packetSize = IMGUR_BUFFSIZE;
      while( packets > 0 ) {
        // copy array chunk into buffer
        for( size_t i=0; i<packetSize; i++ ) {
          buf[i] = byteArray[(index*IMGUR_BUFFSIZE)+i];
        }
        // send buffer
        client->write( buf, packetSize );
        log_w("Sent %d bytes", packetSize);
        if( packets > IMGUR_BUFFSIZE ) {
          packets -= IMGUR_BUFFSIZE;
          if( packets < IMGUR_BUFFSIZE ) {
            packetSize = packets;
          }
          index++;
        } else {
          break;
        }
      }
    break;
  }
}


int ImgurUploader::readResponse(void) {
  int ret = -1;
  while (client.connected()) {
    if(client.available()) {
      String response = client.readStringUntil('\n');
      log_v(response);
      DynamicJsonDocument jsonImgurBuffer( 8192 );
      DeserializationError error = deserializeJson(jsonImgurBuffer, response );
      if (!error) {
        JsonObject root = jsonImgurBuffer.as<JsonObject>();
        if( root["success"].as<String>() == "true" ) {
          String link = root["data"]["link"].as<String>();
          String id = root["data"]["id"].as<String>();
          sprintf( URL, IMGUR_URL_MASK, id.c_str() );
          Serial.printf("Link: %s, id: %s\n", link.c_str(), id.c_str() );
          ret = 1;
        } else {
          // upload failed
          log_e("upload failed (file too big, data corrupted?");
          log_e("Response: %s", response.c_str() );           //Print request answer
        }
      }
    }
  }
  return ret;
}



