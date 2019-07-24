# EspNow flooding mesh


Includes:
- Mesh usb adapter codes (esp32/esp2866).
- Mesh gateway codes (Convert messages between mesh network and MQTT broker)
- Slave node codes (Slave node can read sensors, control switches/lights or something else)

##### Features:
- Works on ESP now broadcast (or 802.11 mac frame)
- Nearly instant connection after poweron
- Maximum number of slave nodes: unlimited
- All nodes have the same bsid
- Flooding mesh support
- Mesh nodes use MQTT service (subscribe/publish)
- Master node (USBAdapter=ESP32 or ESP2866) is connected to RaspberryPi's USB port
- Each Nodes can communicate with each other
- ESP32, ESP2866, ESP01
- Ping about 40-60ms
- ttl
- Battery node support
- AES128
- Retransmission support
- Request/Reply support
- Send and pray support (Send a message to all nodes without reply/ack)
- Easy to configure
- Simple mqqt interface.
- Automatic node discovery
- MQTT local cache on raspberry
- Arduino

###### Slave Node examples
- PIR-sensor node: https://github.com/arttupii/PirSendorNode

###### Demo video
- https://youtu.be/tXgNWhqPE14

###### Mesh usb adapter
- Esp32/Esp2866
- https://github.com/arttupii/EspNowUsb/tree/master/EspNowUsb

###### Mesh slave node codes
- Esp32/Esp2866/Esp-01
- https://github.com/arttupii/EspNowUsb/tree/master/arduinoSlaveNode/main

###### MeshGateway software for RaspberryPi (conversation between mesh and mqtt broker)
 - https://github.com/arttupii/EspNowFloodingMesh/tree/master/gateway
 - See config.js file (https://github.com/arttupii/EspNowUsb/blob/master/RaspberryPiServer/config.js)
```   cd gateway
      sudo apt-get install mosquitto nodejs npm
      nano config.js
      npm install
      node index.js
```

##### Installation
1. Flash Usb adapter software (EspNowUsb/EspNowUsb.ino) to esp32 or esp2866. (You don't need change any parameters)
2. Install gateway softwares to RaspberryPi
```   cd gateway
      sudo apt-get install mosquitto nodejs npm
      npm install
```
3. Modify gateway/config.js file:
  - set secredKey parameter (16 bytes)
  - set initializationVector parameter (16 bytes).
4. Start gateway software on RaspberryPi. .
```
a@labra:~/git/EspNowUsb/gateway/node index.js
begin /dev/ttyUSB0 115200
Subscribe topic device1/led/value from cache
Subscribe topic device1/led/set from cache
reboot
Role MASTER, ttl=NaN
MAC GET
InitializationVector [178,75,242,247,122,197,236,12,94,31,77,193,174,70,94,117]
key [0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255]
Channel 1;
Init
RTC 1563876153

```
5. Open slave node code (arduinoSlaveNode/main/main.ino) and modify deviceName, secredKey, iv and ESP_NOW_CHANNEL paramaters.
  * deviceName should be unique
  * secredKey, iv, bsid and ESP_NOW_CHANNEL must be match to config.js file on raspberryPi. Otherwise mesh network won't work.
  --> Flash slave node


### Recommended topics for switches, sensors and so on (just because of compatibility)
```
[device/nodename]/[type]/[sensorOrSwitchName]/[value/set] value

***SWITCH***
device1/switch/lamp1/value on
device1/switch/lamp1/value off       <--Current lamp state (only node should change this!)
device1/switch/lamp1/set on          <--Request set lamp on from outside
device1/switch/lamp1/set off         <--Request set lamp off from outside
device1/switch/alarm/value off       <--Current lamp state (only node should change this!)
device1/switch/alarm/value on       <--Current lamp state (only node should change this!)
device1/switch/alarm/set on          <--Request set lamp on from outside

***TEMPERATURE SENSORS***
device1/temp/outside/value 24.8   <--Celsius
device1/humidity/bedroom/value 55        <--percentage

device1/temp/thermostat1/set 21.2  <--Set thermostat

***TRIGGER***
device1/trigger/pirSensor1/value "on" <--Trigger to outside. For example pulse from pir-sensor. 
device1/trigger/pirSensor1/value "off" <--Trigger to outside. For example pulse from pir-sensor 

***CONTACT***
device1/contact/switch1/value open
device1/contact/switch1/value closed

***DIMMER***
device1/dimmer/myDimmer1/value 0     <--min value==off
device1/dimmer/myDimmer1/value 255   <--max value==on
device1/dimmer/myDimmer1/set 0     <--min value==off
device1/dimmer/myDimmer1/set 255   <--max value==on

***STRING***
device1/string/message/value HelloWorld!      <--(only node should change this!)
device1/string/screen/set HelloWorld!      <--(short string message from outside)

***NUMBER***
device1/number/thing/value min,max,step      <--(number message to outside)
device1/number/thing/set min,max,step      <--(number message from outside)

***FLOAT***
device1/float/thing1/value 343.23      <--(number message to outside)
device1/float/thing1/set 123.32      <--(number message from outside)

***INT***
device1/int/thing1/value 123      <--(int message to outside)
device1/int/thing1/set 456      <--(int message from outside)

***ROLLERSHUTTER***
device1/shutter/myrollershutter/set open
device1/shutter/myrollershutter/set close
device1/shutter/myrollershutter/set stop
device1/shutter/myrollershutter/value open
device1/shutter/myrollershutter/value close
device1/shutter/myrollershutter/value stop

***Counter***
device1/counter/gasMeter/value 23412343252      <--(gasMeter value to outside)
device1/counter/gasMeter/value 23412343252      <--(gasMeter value to outside)
```

```
 ____________________________________
(                                    )
|                                    |
(            Internet                )
|                                    |
(____________________________________)
                     ^
                     |MQTT
                     |
+--------------------|-------+
| RaspberryPi        V       |
|-------------+   +--+-------|
| MeshGateway |<->| MQTT     |     +-------------------------------------+
|             |   | broker   |     |    ESPNOW mesh network              |
+-----+-------+---+----------+     |                             Node6   |
      ^                            |     Node1        Node3              |
      |    USB(SerialData)         |  +------------+   Node3     Node5   |
      +------------------------------>| USBAdapter |           Node4     |
                                   |  | (Master)   |  NodeX    Node7     |
                                   |  +------------+                     |
                                   +-------------------------------------+
```               
###### Arduino libraries:
- https://github.com/arttupii/espNowFloodingMeshLibrary
- https://github.com/arttupii/ArduinoCommands
- https://github.com/arttupii/SimpleMqttLibrary


###### Slave node code example
```c++
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
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowFloodingMesh_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
    ESP.restart();
  }

  //Handle MQTT events from master. Do not call publish inside of call back. --> Endless event loop and crash
  simpleMqtt.handleSubscribeAndGetEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/value")) { //subscribed  initial value for led.
      if (strcmp("on", value) == 0) { //check value and set led
        ledValue=true;
      }
      if (strcmp("off", value) == 0) {
        ledValue=false;
      }
    }
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/set")) {
      if (strcmp("on", value) == 0) { //check value and set led
        setLed=true;
      }
      if (strcmp("off", value) == 0) {
        setLed=false;
      }
    }
  });

  //Handle MQTT events from master
  simpleMqtt.handlePublishEvents([](const char *topic, const char* value) {
    if (simpleMqtt.compareTopic(topic, deviceName, "/led/set")) {
      if (strcmp("on", value) == 0) { //check value and set led
        setLed=true;
      }
      if (strcmp("off", value) == 0) {
        setLed=false;
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
    setLed=true;
  }
  if (p == '1' && buttonStatechange == true) {
    buttonStatechange = false;
    setLed=false;
  }

  if(ledValue==true && setLed==false) {
    ledValue=false;
    //digitalWrite(LED,HIGH);
    if (!simpleMqtt.publish(deviceName, "/led/value", "off")) {
      Serial.println("Publish failed... Reboot");
      ESP.restart();
    }
  }
  if(ledValue==false && setLed==true) {
    ledValue=true;
    //digitalWrite(LED,HIGH);
    if (!simpleMqtt.publish(deviceName, "/led/value", "on")) {
      Serial.println("Publish failed... Reboot");
      ESP.restart();
    }
  }

  delay(10);
}
```
#### Config file for MeshGateway on RasperryPi
```javascript
module.exports = {
  "usbPort": "/dev/ttyUSB0",
  "mesh": {
    "secredKey": [0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF],
    "ttl": 3,
    "channel": 1
  },
  "dbCacheFile":"./cache.json",
  "mqtt": {
    "host": "mqtt://localhost",
    "root": "mesh/"
  }
}
```
#### Example messages (USBAdapter)
##### Initialize mesh network
```
<READY;
>ROLE MASTER;
<ACK;
>IV  SET [10,31,42,53,46,15,36,57,83,19,11,55,14,33,24,51];
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
