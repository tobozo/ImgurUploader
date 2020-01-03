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
    // get a **client ID** at https://apidocs.imgur.com/?version=latest#authorization-and-oauth
    ImgurUploader(const char *appKey);

    // upload from filesystem
    int   uploadFile( fs::FS &fs, const char* path );

    // upload from a bytes _array
    int   uploadBytes( const uint8_t* byteArray, size_t arrayLen, const char* imageName="pic.jpg", const char* imageMimeType="image/jpeg" );

    // upload from a stream source
    int   uploadStream( size_t arrayLen, void (*streamCB)( Stream* client ), const char* imageName="pic.jpg", const char* imageMimeType="image/jpeg" );

    // replace the default progress callback by a custom callback
    void  setProgressCallback( void (*progressCB)( byte progress ) );

    // retrieve the last successfully submitted URL
    char* getURL(void) { return URL; }

  private:

    void             sendImageData();
    void             (*_progressCB)( byte progress ); // progress callback pointer
    void             (*_streamCB)( Stream* client ); // stream write callback pointer

    int              upload( const char* imageName, const char* imageMimeType );
    int              readResponse( void );

    const char*      getMimeType( const char* fileName );

    const char*      appKey;
    char             URL[40]; // http://i.imgur.com/xxxxx.jpg
    uint8_t*         _byteArray;
    size_t           _arrayLen;

    WiFiClientSecure client;

    File             _sourceFile;
    SourceType       _source;

};

#endif
