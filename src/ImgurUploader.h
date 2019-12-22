/*

  Imgur Image Upload library for ESP32

  This library is inspired from Arduino Imgur library
  https://github.com/tinkerlog/Imgur

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

#ifndef imgur_uploader_h
#define imgur_uploader_h

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>


class ImgurUploader {
  public:
    enum SourceType {
      SOURCE_FILE,
      SOURCE_BYTE_ARRAY,
      SOURCE_STREAM
    };
    ImgurUploader(const char *appKey); // get a **client ID** at https://apidocs.imgur.com/?version=latest#authorization-and-oauth
    int uploadFile( fs::FS &fs, const char* path );
    int uploadBytes( const uint8_t* _byteArray, size_t arrayLen, const char* imageName="pic.jpg", const char* imageMimeType="image/jpeg" );
    int  uploadStream( size_t arrayLen, void (*streamCB)(Stream* client), const char* imageName="pic.jpg", const char* imageMimeType="image/jpeg" );
    void setProgressCallback( void (*progressCB)(byte progress) );
    void setStreamCallback( void (*streamCB)(Stream* client) );
    char* getURL(void) {
      return URL;
    }
  private:
    int upload(const char* imageName, const char* imageMimeType);
    void sendImageData();
    String getMimeType( String fileName );
    int readResponse(void);
    const char *appKey;
    char URL[40];      // http://i.imgur.com/xxxxx.jpg
    uint8_t* byteArray;
    size_t _arrayLen;
    WiFiClientSecure client;
    File sourceFile;
    SourceType source;
    void (*_progressCB)(byte progress);
    void (*_streamCB)(Stream* client);
};

#endif
