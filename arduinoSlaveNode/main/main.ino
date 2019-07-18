#include <EspNowAESBroadcast.h>
#include"SimpleMqtt.h"

#define ESP_NOW_CHANNEL 1
//AES 128bit
unsigned char secredKey[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE, 0xFF};
SimpleMQTT simpleMqtt;


void espNowAESBroadcastRecv(const uint8_t *data, int len, uint32_t replyPrt){
  if(len>0) {
    if(replyPrt) { //Reply asked. Send reply
      
    } else {
     
    }
    simpleMqtt.parse(data, len);
  }
}

void setup() {
  Serial.begin(115200);
  simpleMqtt.setDeviceName("device1"); //set unique name for node device!!!
  
  //Set device in AP mode to begin with
  espNowAESBroadcast_RecvCB(espNowAESBroadcastRecv);
  espNowAESBroadcast_secredkey(secredKey);
  espNowAESBroadcast_setToMasterRole(false, 3/*ttl*/);
      
  espNowAESBroadcast_begin(ESP_NOW_CHANNEL);

  simpleMqtt.handleSend([](const char *mqttMsg, int len){
     espNowAESBroadcast_send((uint8_t*)mqttMsg, len, 3); //Send MQTT commands via mesh network
  });

  espNowAESBroadcast_requestInstantTimeSyncFromMaster();
  while (espNowAESBroadcast_isSyncedWithMaster() == false); //Wait sync with master
  
  bool success = simpleMqtt.subscribe("led"); //Subscribe own led state from MQTT server device1/led/set
}

bool ledOn=false;

void setLedOn(){
  ledOn=true;
  simpleMqtt.publishS("led", "on");
  Serial.println("LED ON");
}
void setLedOff(){
  ledOn=false;
  simpleMqtt.publishS("led", "off");
  Serial.println("LED OFF");
}

void loop() {
  espNowAESBroadcast_loop();
  simpleMqtt.handleSubscribe([](const char *uri, const char* value){
    if(strcmp("device1/led/set", uri)==0) {
      if(strcmp("on", value)==0) {
        setLedOn();
      }
      if(strcmp("off", value)==0) {
        setLedOff();
      }
     
    }
  });
  delay(10);

  int c = Serial.read();
  if(c == '1' && ledOn==false) {
    setLedOn();
  }
  if(c == '0' && ledOn==true) {
    setLedOff();
  }
}
