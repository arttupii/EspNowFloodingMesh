#include <EspNowAESBroadcast.h>
#include"SimpleMqtt.h"

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
unsigned char secredKey[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const char deviceName[] = "device1";
const int ttl = 3;

#define LED 1 /*LED pin*/
#define BUTTON_PIN 0
/*****************************/

SimpleMQTT simpleMqtt = SimpleMQTT(ttl);

void espNowAESBroadcastRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  if (len > 0) {
    Serial.println("********************************************************************");
    Serial.println(replyPrt);
    simpleMqtt.parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //Set device in AP mode to begin with
  espNowAESBroadcast_RecvCB(espNowAESBroadcastRecv);
  espNowAESBroadcast_secredkey(secredKey);
  espNowAESBroadcast_setToMasterRole(false, ttl);
  espNowAESBroadcast_begin(ESP_NOW_CHANNEL);

  espNowAESBroadcast_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowAESBroadcast_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
    ESP.restart();
  }

  //Handle MQTT events from master
  simpleMqtt.handleSubscribeEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/value")) { //subscribed  initial value for led.
      if (strcmp("on", value) == 0) { //check value and set led
        digitalWrite(LED,HIGH);
        Serial.println("Init: Set Led ON");
      }
      if (strcmp("off", value) == 0) {
        digitalWrite(LED,HIGH);
        Serial.println("Init: Set Led OFF");
      }
    }
  });
  
  //Handle MQTT events from master
  simpleMqtt.handlePublishEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/set")) { //Trigger topic,  /device1/led/set
      if (strcmp("on", value) == 0) { //check value and set led
        Serial.println("Set LED ON");
        digitalWrite(LED, HIGH);
      }
      if (strcmp("off", value) == 0) {
        Serial.println("Set LED OFF");
        digitalWrite(LED, LOW);
      }
      if(!simpleMqtt.publish(deviceName, "/led/value", value)) { 
        Serial.println("Publish failed... Reboot"); 
        ESP.restart();
      }
    }
  });
  bool success = simpleMqtt.subscribeTopic(deviceName,"/led/set"); //Subscribe own led state from MQTT server device1/led/set
  success = simpleMqtt.subscribeTopic(deviceName,"/led/value"); //Subscribe own led state from MQTT server device1/led/set
}

bool buttonStatechange = false;

void loop() {
  espNowAESBroadcast_loop();

  int p = Serial.read();//digitalRead(BUTTON_PIN);

  if (p == '0' && buttonStatechange == false) {
    buttonStatechange = true;
    simpleMqtt.publish(deviceName, "/led/value", "on");
    digitalWrite(LED,HIGH);
  }
  if (p == '1' && buttonStatechange == true) {
    buttonStatechange = false;
    simpleMqtt.publish(deviceName, "/led/value", "off");
    digitalWrite(LED,LOW);
  }
  delay(10);
}
