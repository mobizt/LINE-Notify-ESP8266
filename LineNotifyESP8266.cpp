/*
 * LINE Notify Client Arduino Library for ESP8266, version 1.0
 * 
 * This library provides ESP8266 to perform REST API call to LINE Notify service to post the several message types.
 * 
 * The library was test and work well with ESP8266s based module.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#ifndef LineNotifyESP8266_CPP
#define LineNotifyESP8266_CPP

#include "LineNotifyESP8266.h"


void LineNotifyESP8266::init(const char* token) {
  SPIFFS.begin();
  memset(LINE_TOKEN, 0, sizeof LINE_TOKEN);
  strcpy(LINE_TOKEN, token);
}

void LineNotifyESP8266::init(const String token) {
  init(token.c_str());
}

void LineNotifyESP8266::setToken(const char* token) {
  memset(LINE_TOKEN, 0, sizeof LINE_TOKEN);
  strcpy(LINE_TOKEN, token);
}

void LineNotifyESP8266::setToken(const String token) {
  setToken(token.c_str());
}

void LineNotifyESP8266::send_request_header(WiFiClientSecure &client, const char* token, size_t contentLength) {

  char lenVal[10];
  char data[400];
  memset(data, 0, sizeof data);
  memset(lenVal, 0, sizeof lenVal);

  strcpy_P(data, (char*)F("POST /api/notify HTTP/1.1\r\n"));
  strcat_P(data, (char*)F("cache-control: no-cache\r\n"));
  strcat_P(data, (char*)F("Authorization: Bearer "));
  strcat(data, token);
  strcat(data, newline);
  strcat_P(data, (char*)F("Content-Type: multipart/form-data; boundary="));
  strcat(data, HEADER_BOUNDARY);
  strcat(data, newline);
  strcat_P(data, (char*)F("User-Agent: "));
  strcat(data, USER_AGENT);
  strcat(data, newline);
  strcat_P(data, (char*)F("Accept: */*\r\n"));
  strcat_P(data, (char*)F("Host: "));
  strcat(data, LINE_HOST);
  strcat(data, newline);
  strcat_P(data, (char*)F("accept-encoding: gzip, deflate\r\n"));
  strcat_P(data, (char*)F("Connection: close\r\n"));
  strcat_P(data, (char*)F("content-length: "));
  itoa(contentLength, lenVal, 10);
  strcat(data, lenVal);
  strcat(data, newline);
  strcat(data, newline);
  client.print(data);


}

void LineNotifyESP8266::set_multipart_header(char* data, const char* arg)
{

  strcat(data, "--");
  strcat(data, HEADER_BOUNDARY);
  strcat(data, newline);

  if (strcmp(arg, "message") == 0 || strcmp(arg, "stickerPackageId") == 0 || strcmp(arg, "stickerId") == 0  || strcmp(arg, "imageThumbnail") == 0 || strcmp(arg, "imageFullsize") == 0) {

    strcat_P(data, (char*)F("Content-Disposition: form-data; name=\""));
    strcat(data, arg);
    strcat_P(data, (char*)F("\"\r\n"));
    strcat(data, newline);

  }  else {
    char ext[10];
    memset(ext, 0, sizeof ext);

    strcat_P(data, (char*)F("Content-Disposition: form-data; name=\"imageFile\"; filename=\""));
    strcat(data, arg);
    strcat_P(data, (char*)F("\"\r\n"));

    char contentType[30];
    memset(contentType, 0, sizeof contentType);
    strcpy(contentType, getContentType(arg));

    strcat_P(data, (char*)F("Content-Type: "));
    strcat(data, contentType);
    strcat(data, newline);
    strcat(data, newline);

  }

}

void LineNotifyESP8266::set_multipart_boundary(char* data) {
  strcat(data, "--");
  strcat(data, HEADER_BOUNDARY);
  strcat(data, "--");
  strcat(data, newline);
}

uint8_t LineNotifyESP8266::sendLineMessage(WiFiClientSecure &client, const char* msg) {

  if (!client.connect(LINE_HOST, LINE_PORT))
    return LineNotifyESP8266::LineStatus::CONNECTION_FAILED;

  char textHeader[200];
  memset(textHeader, 0, sizeof textHeader);
  set_multipart_header(textHeader, "message");
  strcat(textHeader, msg);
  strcat(textHeader, newline);
  set_multipart_boundary(textHeader);


  send_request_header( client, LINE_TOKEN, strlen(textHeader));
  client.print(textHeader);

  if (waitLineResponse(client))
    return LineNotifyESP8266::LineStatus::SENT_COMPLETED;
  else
    return LineNotifyESP8266::LineStatus::SENT_FAILED;

}

uint8_t LineNotifyESP8266::sendLineMessage(WiFiClientSecure &client, const String msg) {
  return LineNotifyESP8266::sendLineMessage(client, msg.c_str());
}

uint8_t LineNotifyESP8266::sendLineSticker(WiFiClientSecure &client, const char* msg, uint16_t  stickerPackageId, uint16_t stickerId) {


  if (!client.connect(LINE_HOST, LINE_PORT))
    return LineNotifyESP8266::LineStatus::CONNECTION_FAILED;


  char val[20];
  char textHeader[400];
  memset(textHeader, 0, sizeof textHeader);

  set_multipart_header(textHeader, "message");
  strcat(textHeader, msg);
  strcat(textHeader, newline);

  set_multipart_header(textHeader, "stickerPackageId");
  memset(val, 0, sizeof val);
  itoa(stickerPackageId, val, 10);
  strcat(textHeader, val);
  strcat(textHeader, newline);

  set_multipart_header(textHeader, "stickerId");
  memset(val, 0, sizeof val);
  itoa(stickerId, val, 10);
  strcat(textHeader, val);
  strcat(textHeader, newline);

  set_multipart_boundary(textHeader);

  send_request_header(client, LINE_TOKEN, strlen(textHeader));

  client.print(textHeader);

  if (waitLineResponse(client))
    return LineNotifyESP8266::LineStatus::SENT_COMPLETED;
  else
    return LineNotifyESP8266::LineStatus::SENT_FAILED;


}

uint8_t LineNotifyESP8266::sendLineSticker(WiFiClientSecure &client, const String msg, uint16_t  stickerPackageId, uint16_t stickerId) {
  return sendLineSticker(client, msg.c_str(), stickerPackageId, stickerId);
}


uint8_t LineNotifyESP8266::sendLineImageData(WiFiClientSecure &client, const char* msg, const char* fileName, uint8_t* imageData, size_t imageLength) {


  if (!client.connect(LINE_HOST, LINE_PORT))
    return LineNotifyESP8266::LineStatus::CONNECTION_FAILED;


  char textHeader[200];
  memset(textHeader, 0, sizeof textHeader);
  set_multipart_header(textHeader, "message");
  strcat(textHeader, msg);
  strcat(textHeader, newline);

  char imageHeader[200];
  memset(imageHeader, 0, sizeof imageHeader);
  set_multipart_header(imageHeader, fileName);

  char boundary[100];
  memset(boundary, 0, sizeof boundary);
  set_multipart_boundary(boundary);

  uint32_t contentLength = strlen(textHeader) +  strlen(imageHeader) + imageLength + strlen(boundary);


  send_request_header(client, LINE_TOKEN, contentLength);

  client.print(textHeader);

  client.print(imageHeader);

  size_t chunkSize = 512;

  size_t byteRead = 0;

  while (byteRead < imageLength) {
    if (byteRead + chunkSize < imageLength) {
      client.write(imageData, chunkSize);
      imageData += chunkSize;
      byteRead += chunkSize;
    } else {
      client.write(imageData, imageLength - byteRead);
      imageData += chunkSize;
      byteRead = imageLength;
    }
    yield();
  }

  client.print(newline);

  client.print(boundary);

  if (waitLineResponse(client))
    return LineNotifyESP8266::LineStatus::SENT_COMPLETED;
  else
    return LineNotifyESP8266::LineStatus::SENT_FAILED;


}

uint8_t LineNotifyESP8266::sendLineImageData(WiFiClientSecure &client, const String msg, const String fileName, uint8_t* imageData, size_t imageLength)
{
  return sendLineImageData (client, msg.c_str(), fileName.c_str(), imageData, imageLength);
}

uint8_t LineNotifyESP8266::sendLineImageURL(WiFiClientSecure &client, const char* msg, const char* imageURL) {


  if (!client.connect(LINE_HOST, LINE_PORT))
    return LineNotifyESP8266::LineStatus::CONNECTION_FAILED;


  char val[20];

  char textHeader[400];
  memset(textHeader, 0, sizeof textHeader);

  set_multipart_header(textHeader, "message");
  strcat(textHeader, msg);
  strcat(textHeader, newline);

  set_multipart_header(textHeader, "imageThumbnail");
  strcat(textHeader, imageURL);
  strcat(textHeader, newline);

  set_multipart_header(textHeader, "imageFullsize");
  strcat(textHeader, imageURL);
  strcat(textHeader, newline);

  set_multipart_boundary(textHeader);

  send_request_header(client, LINE_TOKEN, strlen(textHeader));

  client.print(textHeader);

  if (waitLineResponse(client))
    return LineNotifyESP8266::LineStatus::SENT_COMPLETED;
  else
    return LineNotifyESP8266::LineStatus::SENT_FAILED;


}

uint8_t LineNotifyESP8266::sendLineImageURL(WiFiClientSecure &client, const String msg, const String imageURL) {
  return sendLineImageURL (client, msg.c_str(), imageURL.c_str());
}


uint8_t LineNotifyESP8266::sendLineImageSPIF(WiFiClientSecure &client, const char* msg, const char* filePath) {


  if (SPIFFS.exists(filePath)) {
    File file = SPIFFS.open(filePath, "r");
    uint32_t imageLength = file.size();



    if (!client.connect(LINE_HOST, LINE_PORT))
      return LineNotifyESP8266::LineStatus::CONNECTION_FAILED;


    char textHeader[200];
    memset(textHeader, 0, sizeof textHeader);
    set_multipart_header(textHeader, "message");
    strcat(textHeader, msg);
    strcat(textHeader, newline);

    char imageHeader[200];
    memset(imageHeader, 0, sizeof imageHeader);
    set_multipart_header(imageHeader, filePath);

    char boundary[100];
    memset(boundary, 0, sizeof boundary);
    set_multipart_boundary(boundary);

    uint32_t contentLength = strlen(textHeader) +  strlen(imageHeader) + imageLength + strlen(boundary);


    send_request_header(client, LINE_TOKEN, contentLength);


    client.print(textHeader);

    client.print(imageHeader);

    size_t chunkSize = 512;
    uint8_t chunk[chunkSize];
    size_t byteRead = 0;

    while (byteRead < imageLength ) {
      memset(chunk, 0, sizeof chunk);
      if (byteRead + chunkSize < imageLength) {
        file.read(chunk, chunkSize);
        client.write(chunk, chunkSize);
        byteRead += chunkSize;
      } else {
        file.read(chunk, imageLength - byteRead);
        client.write(chunk, imageLength - byteRead);
        byteRead = imageLength;
        break;
      }
      yield();
    }

    file.close();

    client.print(newline);

    client.print(boundary);

    if (waitLineResponse(client))
      return LineNotifyESP8266::LineStatus::SENT_COMPLETED;
    else
      return LineNotifyESP8266::LineStatus::SENT_FAILED;


  }


}

uint8_t LineNotifyESP8266::sendLineImageSPIF(WiFiClientSecure &client, const String msg, const String filePath) {
  return sendLineImageSPIF(client, msg, filePath);
}

bool LineNotifyESP8266::waitLineResponse(WiFiClientSecure &client) {

  char response[200];
   char res[200];
  memset(response, 0, sizeof response);
  long dataTime = millis();

  uint16_t count = 0;
  char c;
  int p1, p2, len;

  int httpCode = -1000;
  _textLimit = 0;
  _textRemaining = 0;
  _imageLimit = 0;
  _imageRemaining = 0;

  while (client.connected() && !client.available() && millis() - dataTime < 5000) delay(1);

  dataTime = millis();
  if (client.connected() && client.available()) {
    while (client.available() &&  millis() - dataTime < 5000) {
      c = client.read();
      if (count < sizeof(response) - 1 && c != '\n') {
        strcat_c(response, c);
        count++;
      } else {
		
        dataTime = millis();
        if (strlen(response) > 0) {
          if (strpos(response, "HTTP/1.1", 0) != -1) {
            p1 = strpos(response, " ", 0);
            len = rstrpos(response, " ", 0) - p1 - 1;
            memset(res, 0, sizeof res);
            strncpy(res, response + p1, len);
            httpCode = atoi(res);
          } else if (strpos(response, ":", 0) != -1) {
            p1 = strpos(response, ":", 0);
            if (p1 < strlen(response) - 1) {
              len = strlen(response) - p1;
              memset(res, 0, sizeof res);
              strncpy(res, response + p1, len);

              //Parses for headers and payload
              if (strpos(response, "X-RateLimit-Limit", 0) != -1) _textLimit = atoi(res);
              else if (strpos(response, "X-RateLimit-ImageLimit", 0) != -1) _imageLimit = atoi(res);
			        else if (strpos(response, "X-RateLimit-Remaining", 0) != -1) _textRemaining = atoi(res);
			        else if (strpos(response, "X-RateLimit-ImageRemaining", 0) != -1) _imageRemaining = atoi(res);

            }
          }
        }
        memset(response, 0, sizeof response);
        count = 0;
      }
    }
  }
  return httpCode==200;
}


char* LineNotifyESP8266::getContentType(const char* filename) {

  char buf[30];
  memset(buf, 0, sizeof buf);

  char *dot = strrchr(filename, '.');

  if (dot && ! (strcmp(dot, ".jpg") || strcmp(dot, ".jpeg"))) {
    strcpy(buf, "image/jpeg");
    return buf;
  } else if (dot && !strcmp(dot, ".gif")) {
    strcpy(buf, "image/gif");
    return buf;
  } else if (dot && !strcmp(dot, ".png")) {
    strcpy(buf, "image/png");
    return buf;
  } else if (dot && !strcmp(dot, ".bmp")) {
    strcpy(buf, "image/bmp");
    return buf;
  }
  return buf;
}

uint16_t LineNotifyESP8266::textMessageLimit(void){
  return _textLimit;
}

uint16_t LineNotifyESP8266::textMessageRemaining(void){
  return _textRemaining;
}

uint16_t LineNotifyESP8266::imageMessageLimit(void){
  return _imageLimit;
}

uint16_t LineNotifyESP8266::imageMessageRemaining(void){
  return _imageRemaining;
}

void LineNotifyESP8266::strcat_c (char *str, char c)
{
  for (; *str; str++);
  *str++ = c;
  *str++ = 0;
}
int LineNotifyESP8266::strpos(const char *haystack, const char *needle, int offset)
{
  char _haystack[strlen(haystack)];
  strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
  char *p = strstr(_haystack, needle);
  if (p)
    return p - _haystack + offset + 1;
  return -1;
}

int LineNotifyESP8266::rstrpos(const char *haystack, const char *needle, int offset)
{
  char _haystack[strlen(haystack)];
  strncpy(_haystack, haystack + offset, strlen(haystack) - offset);
  char *p = rstrstr(_haystack, needle);
  if (p)
    return p - _haystack + offset + 1;
  return -1;
}
char* LineNotifyESP8266::rstrstr(const char* haystack, const char* needle)
{
  int needle_length = strlen(needle);
  const char* haystack_end = haystack + strlen(haystack) - needle_length;
  const char* p;
  size_t i;
  for (p = haystack_end; p >= haystack; --p)
  {
    for (i = 0; i < needle_length; ++i) {
      if (p[i] != needle[i])
        goto next;
    }
    return (char*)p;
next:;
  }
  return 0;
}
char* LineNotifyESP8266::replace_char(char* str, char in, char out)
{
  char * p = str;
  while (p != '\0') {
    if (*p == in)
      *p == out;
    ++p;
  }
  return str;
}

LineNotifyESP8266 lineNotify = LineNotifyESP8266();

#endif
