# Arduino EspNow managed flooding mesh network with mqtt

Includes:
- Mesh usb adapter codes (for esp32/esp2866).
- Mesh gateway codes (Convert messages between mesh network and MQTT broker)
- Slave node example codes (Slave node can read sensors, control switches/lights or something else)

##### Features:
- Works on EspNow broadcast
- Nearly instant connection after poweron
- Maximum number of slave nodes: unlimited
- Flooding mesh support
- a message cache. If a received packet is already found in the cache --> it will not be retransmitted or handled again
- Mesh nodes use MQTT service (subscribe/publish)
- Master node (USBAdapter=ESP32 or ESP2866) is connected to RaspberryPi's USB port
- Each Nodes can communicate with each other
- ESP32, ESP2866, ESP01
- Ping about 40-60ms
- ttl support
- Battery node support
- AES128
- Retransmission support
- Request/Reply support
- Send and pray support (Send a message to all nodes without reply/ack)
- Easy to configure (just set channel, secure keys and bsid)
- Simple mqqt interface. All nodes can use mqtt services via master node (subsribe,unsubscribe,publish,get).
- MQTT local cache on raspberry
- Arduino

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
## Flooding mesh network
In this network example ttl must be >= 5
```
               SlaveNode
                   |
                   |         Message from master to BatteryNode
                   |   ---------------------------+
                   | ttl=5               ttl=4    |
SlaveNode-------MasterNode-------------SlaveNode  |
                   |                     |        |
                   |                     |        |
                   |                     |        |
                   |                     |        |
               SlaveNode                 |        |
                   |                     |        |
                   |                     |        |
                   |                     |        +------------------------------------------------>
                   |                     | ttl=3         ttl=2              ttl=1
SlaveNode-------SlaveNode-------------SlaveNode-------SlaveNode-------------SlaveNode---------BatteryNode
   |               |                     |
   |               |                     |
   |               |                     |
   |               |                     |
   +-----------SlaveNode-----------------+
```        
###### Message relay
The Master sends a request to the farthest node and the farthest node replies.
![alt text](https://raw.githubusercontent.com/arttupii/EspNowFloodingMesh/master/pictures/messageRelay.gif)

###### Arduino libraries:
- https://github.com/arttupii/espNowFloodingMeshLibrary
- https://github.com/arttupii/ArduinoCommands
- https://github.com/arttupii/SimpleMqttLibrary
- https://github.com/kakopappa/arduino-esp8266-aes-lib (Only ESP2866)

###### Slave Node examples
- PIR-sensor node: https://github.com/arttupii/PirSendorNode

###### Early demo video
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


##### Installation
0. Install Arduino and following dependencies:
  - https://github.com/arttupii/espNowFloodingMeshLibrary
  - https://github.com/arttupii/ArduinoCommands
  - https://github.com/arttupii/SimpleMqttLibrary
  - https://github.com/kakopappa/arduino-esp8266-aes-lib (Only ESP2866)
1. Install esp8266 dev module. Use git version.
  - Instructions: https://github.com/esp8266/Arduino
2. Check Espressif FW version!!!. It should be nonos-sdk 2.2.1+100(testing).
  ![alt text](https://github.com/arttupii/EspNowFloodingMesh/blob/master/pictures/ArduinoSetupForEsp2866.png)
3. Add "https://dl.espressif.com/dl/package_esp32_dev_index.json" into the Additional Board Manager URLs field.
  ![alt text](https://raw.githubusercontent.com/arttupii/EspNowFloodingMesh/master/pictures/ArduinoAdditionalURLs.png)
4. Install esp32 dev module 1.0.3-rc1 from Arduino's Boards Manager.
  ![alt text](https://raw.githubusercontent.com/arttupii/EspNowFloodingMesh/master/pictures/ArduinoBoardManager.png)
5. Flash Usb adapter software (EspNowUsb/EspNowUsb.ino) on esp32/esp2866 (esp32 is the best choice.). (You don't need to change any parameters)
 - https://github.com/arttupii/EspNowFloodingMesh/tree/master/EspNowUsb
6. Install mqtt broker, nodejs and npm on RaspberryPi
```
      curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
      sudo apt-get install mosquitto nodejs npm
```
7. Get gateway and install npm modules
* https://github.com/arttupii/EspNowFloodingMesh/tree/master/gateway
```   
      git clone git@github.com:arttupii/EspNowFloodingMesh.git
      cd EspNowFloodingMesh
      cd gateway
      npm install
```
8. Modify gateway/config.js file:
  - set secredKey parameter (16 bytes)
  - set initializationVector parameter (16 bytes).
  - set bsid
9. Start gateway software on RaspberryPi. .
```
      a@labra:~/git/EspNowUsb/gateway/node index.js
      begin /dev/ttyUSB0 115200
      Subscribe topic device1/led/value from cache
      Subscribe topic device1/led/set from cache
      reboot
      Role MASTER, ttl=3
      MAC GET
      InitializationVector [178,75,242,247,122,197,236,12,94,31,77,193,174,70,94,117]
      key [0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255]
      Channel 1;
      Init
      RTC 1563876153

```
10. Open slave node code (arduinoSlaveNode/main/main.ino) and modify deviceName, secredKey, iv and ESP_NOW_CHANNEL paramaters.
  * https://github.com/arttupii/EspNowFloodingMesh/tree/master/arduinoSlaveNode/main
  * deviceName should be unique
  * secredKey, iv, bsid and ESP_NOW_CHANNEL must be match to config.js file on raspberryPi. Otherwise mesh network won't work.
  --> Flash slave node


###### Slave node code example
Slave node updates the button's state to topic device1/switch/led/value. The led state can be controlled with topic device1/switch/led/set on/off.
```c++
#include <EspNowFloodingMesh.h>
#include<SimpleMqtt.h>

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
const char deviceName[] = "device1";
unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};
const int ttl = 3;
const int bsid = 0x112233;
/*****************************/

#define LED 1
#define BUTTON_PIN 2

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

  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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
    ESP.restart();
  }

  //Handle MQTT events from master. Do not call publish() inside of call back. --> Endless event loop and crash
  simpleMqtt.handleEvents([](const char *topic, const char* value) {
    simpleMqtt._ifSwitch(VALUE, "led", [](MQTT_switch value){ //<--> Listening topic switch/led/value/value
      if(value==SWITCH_ON) {
        ledValue = true;
      }
      if(value==SWITCH_OFF) {
        ledValue = false;
      }
    });
    simpleMqtt._ifSwitch(SET, "led", [](MQTT_switch set){ //<-->Listening topic device1/switch/led/set
      if(set==SWITCH_ON) {
        setLed = true;
      }
      if(set==SWITCH_OFF) {
        setLed = false;
      }
    });
  });

  if (!simpleMqtt._switch(SUBSCRIBE, "led")) { //Subscribe topic device1/switch/led/set and get topic device1/switch/led/value from cache
    Serial.println("MQTT operation failed. No connection to gateway");
  }
}

bool buttonStatechange = false;

void loop() {
  espNowFloodingMesh_loop();

  int p = digitalRead(BUTTON_PIN);

  if (p == HIGH && buttonStatechange == false) {
    buttonStatechange = true;
    setLed = true;
  }
  if (p == LOW && buttonStatechange == true) {
    buttonStatechange = false;
    setLed = false;
  }

  if (ledValue == true && setLed == false) {
    ledValue = false;
    Serial.println("LED_OFF");
    //digitalWrite(LED,HIGH);
    if (!simpleMqtt._switch(PUBLISH, "led", SWITCH_OFF)) { //publish topic device1/switch/led/value off
      Serial.println("Publish failed... Reboot");
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
    }
  }

  if (ledValue == false && setLed == true) {
    ledValue = true;
    Serial.println("LED_ON");
    if (!simpleMqtt._switch(PUBLISH, "led", SWITCH_ON)) { //publish topic device1/switch/led/value on
      Serial.println("Publish failed... Reboot");
      Serial.println(ESP.getFreeHeap());
      ESP.restart();
    }
  }

  delay(100);
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


### Recommended topics for switches, sensors and so on (just because of compatibility)
```
[nodename]/[type]/[sensorOrSwitchName]/[value or set] value

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
device1/trigger/pirSensor1/value "triggered" <--Trigger to outside. For example pulse from pir-sensor.

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

***Binary***
device1/bin/data1/value dGVzdA==      <--(Binary data base64)
device1/bin/data/set dGVzdA==      <--(Binary data base64)
```

