#include"SimpleMqtt.h"
#include<Arduino.h>
#include <EspNowAESBroadcast.h>

//#include <EspNowAESBroadcast.h>


SimpleMQTT::SimpleMQTT() {
  buffer[0] = 0;
}

SimpleMQTT::~SimpleMQTT() {
}

void SimpleMQTT::setDeviceName(const char *name) {
  deviceName = name;
}
void SimpleMQTT::handleSend(void (*cb)(const char *, int)){
  sendCallBack = cb;
}
bool SimpleMQTT::publishI(const char* parameterName, long long value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%lld\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:int\n", deviceName, parameterName);
  sendCallBack(buffer, (int)(p - buffer) + 1);
}

bool SimpleMQTT::publishF(const char* parameterName, float value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%f\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:float\n", deviceName, parameterName);
  sendCallBack(buffer, (int)(p - buffer) + 1);
}

bool SimpleMQTT::publishB(const char* parameterName, bool value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%f\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:bool\n", deviceName, parameterName);
  sendCallBack(buffer, (int)(p - buffer) + 1);
}

bool SimpleMQTT::publishS(const char* parameterName, const char *value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%s\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:string\n", deviceName, parameterName);
  sendCallBack(buffer, (int)(p - buffer) + 1);
}

bool SimpleMQTT::subscribe(const char* parameterName) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nS:%s/%s/value\n", deviceName, parameterName);
  p += sprintf(p, "S:%s/%s/set\n", deviceName, parameterName);
  p += sprintf(p, "S:%s/%s/type\n", deviceName, parameterName);

  sendCallBack(buffer, (int)(p - buffer) + 1);

  sprintf(buffer, "%s/%s/value",deviceName, parameterName);
  addUriToVector(buffer);
  
  sprintf(buffer, "%s/%s/set",deviceName, parameterName);
  addUriToVector(buffer);
}
void SimpleMQTT::addUriToVector(char *uri){
  int l = strlen(uri)+1;
  char *b = (char*)malloc(l);
  memcpy(b,uri,l);
  uriVector.push_back(b);
}
bool SimpleMQTT::subscribeUri(const char* devName, const char *valName) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nS:%s/%s/value\n", devName, valName);
  //sendCallBack(buffer, (int)(p - buffer) + 1);

  sprintf(buffer, "%s/%s/value",devName, valName);
  addUriToVector(buffer);
}

void SimpleMQTT::parse(const unsigned char *data, int size) {
  if (data[0] == 'M' && data[1] == 'Q' && data[2] == 'T' && data[3] == 'T' && data[4] == '\n') {
    int i = 0;
    int s = 0;
    while (i < size) {
      for (; i < size; i++) {
        if (data[i] == '\n') {
          parse2((const char*)data + s, i - s);
          s = i + 1;
          i++;
        }
      }
    }
  }
}

const char* SimpleMQTT::getBuffer() {
  return buffer;
}

void SimpleMQTT::handleSubscribe(void (cb)(const char *, const char*)) {
  subscribeCallBack = cb;
}

void SimpleMQTT::parse2(const char *c, int l) {
  if (c[0] == 'P' || c[0] == 'M' | c[1] == ':') { //publish
    char uri[30];
    char value[30];
    int i = 2;
    for (; (i < l) && c[i] != ':'; i++); //find :
    if (i != l) { //found
      if (i > sizeof(uri)) return;
      memcpy(uri, c + 2, l - 2);
      uri[i - 2] = 0;
      memcpy(value, c + i + 1, l - i);
      value[l - i - 1] = 0;

      for(char* subscribed_uri:uriVector){
        if(strcmp(subscribed_uri,uri)==0){
          subscribeCallBack(uri, value);
          break;
        }
      }
    }
  }
}
