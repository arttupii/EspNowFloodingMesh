#include"SimpleMqtt.h"
#include<Arduino.h>
#include <EspNowFloodingMesh.h>

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

bool SimpleMQTT::publish(const char* deviceName, const char* parameterName, const char *value) {
  char *p = buffer;
  p += sprintf(p, "MQTT\nP:%s%s:%s\n", deviceName, parameterName, value);
  return send(buffer, (int)(p - buffer) + 1, 0);
}

bool SimpleMQTT::subscribeTopic(const char* devName, const char *valName) {
  char *p = buffer;
  sprintf(buffer, "%s%s", devName, valName);
  addtopicToVector(buffer);

  p += sprintf(p, "MQTT\nS:%s%s\n", devName, valName);
  bool ret = send(buffer, (int)(p - buffer) + 1, 0);

  return ret;
}

void SimpleMQTT::addtopicToVector(char *topic) {
  int l = strlen(topic) + 1;
  char *b = (char*)malloc(l);
  memcpy(b, topic, l);
  topicVector.push_back(b);
}


void SimpleMQTT::parse(const unsigned char *data, int size, uint32_t replyId, bool subscribeSequance) {
  this->replyId = replyId;
  if (data[0] == 'M' && data[1] == 'Q' && data[2] == 'T' && data[3] == 'T' && data[4] == '\n') {
    int i = 0;
    int s = 0;
    Serial.println((const char*)data);
    while (i < size) {
      for (; i < size; i++) {
        if (data[i] == '\n') {
          parse2((const char*)data + s, i - s, subscribeSequance);
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

void SimpleMQTT::handleSubscribeEvents(void (cb)(const char *, const char*)) {
  subscribeCallBack = cb;
}
void SimpleMQTT::handlePublishEvents(void (cb)(const char *, const char*)) {
  publishCallBack = cb;
}

bool SimpleMQTT::send(const char *mqttMsg, int len, uint32_t replyId) {
  static SimpleMQTT *myself = this;
  if (replyId == 0) {
    bool status = espNowFloodingMesh_sendAndWaitReply((uint8_t*)mqttMsg, len, ttl, 3, [](const uint8_t *data, int size) {
      if (size > 0) {
        myself->parse(data, size, 0, true); //Parse simple Mqtt protocol messages
      }
    }); //Send MQTT commands via mesh network
    if (!status) {
      //Send failed, no connection to master??? Reboot ESP???
      Serial.println("Timeout");
      return false;
    }
    return status;
  } else {
    espNowFloodingMesh_sendReply((uint8_t*)mqttMsg, len, ttl, replyId);
  }
}

void SimpleMQTT::parse2(const char *c, int l, bool subscribeSequance) {
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
          if (subscribeSequance) {
            subscribeCallBack(topic, value);
          } else {
            publishCallBack(topic, value);
          }
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
