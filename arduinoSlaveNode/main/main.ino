#include <EspNowFloodingMesh.h>
#include<SimpleMqtt.h>

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
const char deviceName[] = "device1";
unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};
const int ttl = 3;

/*****************************/

#define LED 1 /*LED pin*/
#define BUTTON_PIN 0

SimpleMQTT simpleMqtt = SimpleMQTT(ttl, deviceName);

void espNowFloodingMeshRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  if (len > 0) {
   simpleMqtt.parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //Set device in AP mode to begin with
  espNowFloodingMesh_RecvCB(espNowFloodingMeshRecv);
  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(false, ttl);
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowFloodingMesh_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
    ESP.restart();
  }

  //Handle MQTT events from master
  simpleMqtt.handleSubscribeAndGetEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/value")) { //subscribed  initial value for led.
      if (strcmp("on", value) == 0) { //check value and set led
        digitalWrite(LED, HIGH);
        Serial.println("Init: Set Led ON");
      }
      if (strcmp("off", value) == 0) {
        digitalWrite(LED, HIGH);
        Serial.println("Init: Set Led OFF");
      }
    }
  });

  //Handle MQTT events from master
  simpleMqtt.handlePublishEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/set")) {
      if (strcmp("on", value) == 0) { //check value and set led
        Serial.println("Set LED ON");
        digitalWrite(LED, HIGH);
      }
      if (strcmp("off", value) == 0) {
        Serial.println("Set LED OFF");
        digitalWrite(LED, LOW);
      }
      if (!simpleMqtt.publish(deviceName, "/led/value", value)) {
        Serial.println("Publish failed... Reboot");
        ESP.restart();
      }
    }
  });
  bool success = simpleMqtt.subscribeTopic(deviceName,"/led/set"); //Subscribe the led state from MQTT server device1/led/set
  success = simpleMqtt.subscribeTopic(deviceName,"/led/value"); //Subscribe the led state from MQTT server (topic is device1/led/set)

  //simpleMqtt.unsubscribeTopic(deviceName,"/led/value"); //unsubscribe
}

bool buttonStatechange = false;

void loop() {
   espNowFloodingMesh_loop();

  int p = Serial.read();//digitalRead(BUTTON_PIN);

  if (p == '0' && buttonStatechange == false) {
    buttonStatechange = true;
    simpleMqtt.publish(deviceName, "/led/value", "on");
    digitalWrite(LED, HIGH);
  }
  if (p == '1' && buttonStatechange == true) {
    buttonStatechange = false;
    simpleMqtt.publish(deviceName, "/led/value", "off");
    digitalWrite(LED, LOW);
  }
  delay(10);
}
