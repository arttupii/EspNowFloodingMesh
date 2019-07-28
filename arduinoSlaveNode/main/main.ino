#include <EspNowFloodingMesh.h>
#include<SimpleMqtt.h>

/********NODE SETUP********/
const char deviceName[] = "device2";
int bsid = 0x112233;
#if 1
#define ESP_NOW_CHANNEL 1
unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};;
const int ttl = 3;
#else
#include "/home/arttu/git/myEspNowMeshConfig.h" //My secred mesh setup...
#endif
/*****************************/

#define LED 1 /*LED pin*/
#define BUTTON_PIN 0

SimpleMQTT simpleMqtt = SimpleMQTT(ttl, deviceName);

void espNowFloodingMeshRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  if (len > 0) {
    simpleMqtt.parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
  }
}

bool setLed;
bool ledValue;
void setup() {
  Serial.begin(115200);

  //pinMode(LED, OUTPUT);
  //pinMode(BUTTON_PIN, INPUT_PULLUP);

  //Set device in AP mode to begin with
  espNowFloodingMesh_RecvCB(espNowFloodingMeshRecv);
  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(false, ttl);
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL, bsid);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowFloodingMesh_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
   // ESP.restart();
  }

  //Handle MQTT events from master. Do not call publish inside of call back. --> Endless event loop and crash
  simpleMqtt.handleEvents([](const char *topic, const char* value) {
    /*if (simpleMqtt.compareTopic(topic, deviceName, "/switch/led/value")) { //subscribed  initial value for led.
      if (strcmp("on", value) == 0) { //check value and set led
        ledValue = true;
      }
      if (strcmp("off", value) == 0) {
        ledValue = false;
      }
    }
    if (simpleMqtt.compareTopic(topic, deviceName, "/switch/led/set")) {
      if (strcmp("on", value) == 0) { //check value and set led
        setLed = true;
      }
      if (strcmp("off", value) == 0) {
        setLed = false;
      }
    }*/
    
    simpleMqtt._ifSwitch(VALUE, "led", [](MQTT_switch value){ //<--> topic switch/led/value
      if(value==SWITCH_ON) {
        ledValue = true;
      }
      if(value==SWITCH_OFF) {
        ledValue = false;
      }
    });
    simpleMqtt._ifSwitch(SET, "led", [](MQTT_switch value){ //<-->topic switch/led/set
      if(value==SWITCH_ON) {
        setLed = true;
      }
      if(value==SWITCH_OFF) {
        setLed = false;
      }
    });
  });

  /*
    bool success = simpleMqtt.subscribeTopic(deviceName, "/switch/led/set"); //Subscribe the led state from MQTT server device1/switch/led/set
    success = simpleMqtt.getTopic(deviceName, "/led/switch/value"); //Get the led state from MQTT server (topic is device1/switch/led/value)
  */
  if (!simpleMqtt._switch(SUBSCRIBE, "led")) { //Same as the upper, but the smarter way
    Serial.println("MQTT operation failed. No connection to gateway");
  }
}

bool buttonStatechange = false;

void loop() {
  espNowFloodingMesh_loop();

  int p = Serial.read();//digitalRead(BUTTON_PIN);

  if (p == '0' && buttonStatechange == false) {
    buttonStatechange = true;
    setLed = true;
  }
  if (p == '1' && buttonStatechange == true) {
    buttonStatechange = false;
    setLed = false;
  }

  if (ledValue == true && setLed == false) {
    ledValue = false;
    Serial.println("LED_OFF");
    //digitalWrite(LED,HIGH);
    /*if (!simpleMqtt.publish(deviceName, "/led/value", "off")) {
      Serial.println("Publish failed... Reboot");
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
      }*/
    //The better way to publish
    if (!simpleMqtt._switch(PUBLISH, "led", SWITCH_OFF)) {
      Serial.println("Publish failed... Reboot");
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
    }
  }
  if (ledValue == false && setLed == true) {
    ledValue = true;
    Serial.println("LED_ON");

    //digitalWrite(LED,HIGH);
    /*if (!simpleMqtt.publish(deviceName, "/led/value", "on")) {
      Serial.println("Publish failed... Reboot");
      ESP.restart();
      }*/
    //The better way to publish
    if (!simpleMqtt._switch(PUBLISH, "led", SWITCH_ON)) {
      Serial.println("Publish failed... Reboot");
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
    }
  }

  delay(100);

}
