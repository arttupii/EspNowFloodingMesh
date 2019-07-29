# Arduino EspNow flooding mesh network UsbAdapter


###### Arduino libraries:
- https://github.com/arttupii/espNowFloodingMeshLibrary
- https://github.com/arttupii/ArduinoCommands
- https://github.com/arttupii/SimpleMqttLibrary
- https://github.com/kakopappa/arduino-esp8266-aes-lib (Only ESP2866)


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
