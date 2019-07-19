#include"SimpleMqtt.h"
#include<Arduino.h>
#include <EspNowAESBroadcast.h>

SimpleMQTT::SimpleMQTT(int ttl) {
  buffer[0] = 0;
  this->ttl = ttl;
}

SimpleMQTT::~SimpleMQTT() {
}

bool SimpleMQTT::compareTopic(const char* topic, const char* deviceName, const char* t) {
  String tmp = deviceName;
  tmp += t;
  return strcmp(topic, tmp.c_str()) == 0;
}

void SimpleMQTT::setDeviceName(const char *name) {
  deviceName = name;
}

bool SimpleMQTT::publishI(const char* parameterName, long long value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%lld\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:int\n", deviceName, parameterName);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::publishF(const char* parameterName, float value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%f\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:float\n", deviceName, parameterName);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::publishB(const char* parameterName, bool value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%f\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:bool\n", deviceName, parameterName);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::publishS(const char* parameterName, const char *value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s/%s/value:%s\n", deviceName, parameterName, value);
  p += sprintf(p, "P:%s/%s/type:string\n", deviceName, parameterName);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::subscribeTopic(const char* devName, const char *valName) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nS:%s/%s\n", devName, valName);
  bool ret = send(buffer, (int)(p - buffer) + 1, 0);

  sprintf(buffer, "%s/%s", devName, valName);
  addtopicToVector(buffer);
  return ret;
}

void SimpleMQTT::addtopicToVector(char *topic) {
  int l = strlen(topic) + 1;
  char *b = (char*)malloc(l);
  memcpy(b, topic, l);
  topicVector.push_back(b);
}


void SimpleMQTT::parse(const unsigned char *data, int size, uint32_t replyId) {
  this->replyId = replyId;
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

bool SimpleMQTT::send(const char *mqttMsg, int len, uint32_t replyId) {
  if (replyId == 0) {
    bool status = espNowAESBroadcast_sendAndWaitReply((uint8_t*)mqttMsg, len, ttl, 3/*Try to send 3 times if no reply*/); //Send MQTT commands via mesh network
    if (!status) {
      //Send failed, no connection to master??? Reboot ESP???
      Serial.println("Timeout");
      return false;
    }
    return status;
  } else {
    espNowAESBroadcast_sendReply((uint8_t*)mqttMsg, len, ttl, replyId);
  }
}

void SimpleMQTT::parse2(const char *c, int l) {
  if (c[0] == 'P' || c[0] == 'M' | c[1] == ':') { //publish
    char topic[30];
    char value[30];
    int i = 2;
    for (; (i < l) && c[i] != ':'; i++); //find :
    if (i != l) { //found
      if (i > sizeof(topic)) return;
      memcpy(topic, c + 2, l - 2);
      topic[i - 2] = 0;
      memcpy(value, c + i + 1, l - i);
      value[l - i - 1] = 0;

      for (char* subscribed_topic : topicVector) {
        if (strcmp(subscribed_topic, topic) == 0) {
          subscribeCallBack(topic, value);

          if (replyId) {
            //Reply/Ack requested
            send("ACK", 4, replyId);
          }
          break;
        }
      }
    }
  }
}
