ESPNOW mesh usb adapter for esp32/esp2866/esp01. (+nodejs server and slave node codes)

Features:
- Mesh nodes use MQTT service (subscribe/publish)
- Master node (USBAdapter=ESP32 or ESP2866) is connected to RaspberryPi's USB port
- Maximum number of slave nodes: unlimited
- Flooding mesh support
- ESP32, ESP2866, ESP01
- Ping about 40-60ms
- Battery node support
- Nearly instant connection after reboot
- AES128
- Retransmission support
- Send and request reply support
- Send and pray support (Send a message to all nodes without reply/ack)



Software for master Node
https://github.com/arttupii/EspNowUsb/tree/master/EspNowUsb

Software for slave Nodes
https://github.com/arttupii/EspNowUsb/tree/master/arduinoSlaveNode/main

Sofware for RaspberryPi (conversation between mesh and mqtt broker)
 - https://github.com/arttupii/EspNowUsb/tree/master/RaspberryPiServer (needs MQTT broker)
 - See config.js file (https://github.com/arttupii/EspNowUsb/blob/master/RaspberryPiServer/config.js)
```
      sudo apt-get install mosquitto
      nano config.js
      npm install
      node index.js
```

```
 ____________________________________
(                                    )
|                                    |
(            Internet                )
|                                    |
(____________________________________)
     ^
     |
     |
     V
+----+--------+
| RaspberryPi |
|             |                    +-------------------------------------+
| MQTT        |                    |    ESPNOW mesh network              |
+-----+-------+                    |                             Node6   |
      ^                            |     Node1        Node3              |
      |      Serial Port           |  +------------+   Node3     Node5   |
      +------------------------------>| USBAdapter |           Node4     |
                                   |  | Master     |  NodeX              |
                                   |  +------------+                     |
                                   +-------------------------------------+
```               
Dependencies:
- https://github.com/arttupii/espNowAESBroadcast
- https://github.com/arttupii/ArduinoCommands


Slave node code
```
#include <EspNowAESBroadcast.h>
#include"SimpleMqtt.h"

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
unsigned char secredKey[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const char deviceName[] = "device1"; //This must be unique!!!!
const int ttl = 3;
/*****************************/

#define LED 1 /*LED pin*/
#define BUTTON_PIN 0


SimpleMQTT simpleMqtt = SimpleMQTT(ttl);

void espNowAESBroadcastRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  if (len > 0) {
    simpleMqtt.parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
  }
}

void setup() {
  Serial.begin(115200);
  simpleMqtt.setDeviceName(deviceName); //set unique name for node device!!! It is like a ip address

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
  simpleMqtt.handleSubscribe([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/set")) { //Check, is topic /device1/led/set
      if (strcmp("on", value) == 0) { //check value and set led
        setLed(true);
      }
      if (strcmp("off", value) == 0) {
        setLed(false);
      }
    }
  });

  bool success = simpleMqtt.subscribeTopic(deviceName,"led/set"); //Subscribe own led state from MQTT server device1/led/set
  success = simpleMqtt.subscribeTopic(deviceName,"led/value"); //Subscribe own led state from MQTT server device1/led/set
}


void setLed(bool on) {
  if(!simpleMqtt.publishS("led", on?"on":"off")) { //Send mqtt topic device1/led/value off
    Serial.println("Publish failed... Reboot"); //No connection to master
    ESP.restart();
  }
  Serial.println(on?"LED ON":"LED OFF");
  digitalWrite(LED, on?HIGH:LOW);
}

void loop() {
  espNowAESBroadcast_loop();

  int p = digitalRead(BUTTON_PIN);
  static bool buttonStatechange = false;

  if (p == HIGH && buttonStatechange == false) {
    buttonStatechange = true;
    setLed(true);
  }
  if (p == LOW && buttonStatechange == true) {
    buttonStatechange = false;
    setLed(false);
  }
  delay(10);
}
```

#### Example messages (USBAdapter)
##### Initialize mesh network
```
<READY;
>ROLE MASTER;
<ACK;
>KEY SET [00,11,22,33,44,55,66,77,88,99,AA,BB,CC,DD,EE,FF];
<ACK;
>CHANNEL SET 1;
<ACK;
>INIT;
<ACK;
```
##### Set ttl value for SYNC_TIME messages
```
<READY;
>ROLE MASTER 3;
<ACK;
```
##### Reboot usb adapter
```
>REBOOT;
<ACK REBOOTING;
<READY;
```
##### Send message with ttl 3
```
>SEND 3 [11,22,33,44,55,66];
<ACK;
```
##### Request with ttl 3 (nodes/node will send reply with 2314 replyId)
```
>REQ 3 [11,22,33,44,55,66];
<ACK 2314;
```
##### Reply with ttl 3 and replyId
```
>REPLY 3 2314 [11,22,33,44,55,66];
<ACK;
```
##### Reply received with 2314 replyId
```
>REC 2314 [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
```
##### Message received
```
<REC 0 [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
```
##### Invalid command
```
<ABCD;
>NACK INVALID COMMAND;
```
##### RTC time SET command (EPOC)
```
<RTC SET 23456;
>ACK 23456;
```
##### RTC time GET command (EPOC)
```
<RTC GET;
>ACK 243495;
```
