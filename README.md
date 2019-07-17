ESPNOW mesh usb adapter for esp32/esp2866/esp01

Dependencies:
- https://github.com/arttupii/espNowAESBroadcast
- https://github.com/arttupii/ArduinoCommands


#### Example messages
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
##### Reply received with 2314 replyId
```
>REC 2314 [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
```
##### Message received
```
<REC [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
```
##### Invalid command
```
<ABCD;
>NACK INVALID COMMAND;
```
##### RTC time SET command (EPOC)
```
<RTC SET 23456;
>ACK;
```
##### RTC time GET command (EPOC)
```
<RTC GET;
>ACK 243495;
```
#### Test setup
ESP2866, ESP01 and ESP32(usb)
![Test setup](https://raw.githubusercontent.com/arttupii/EspNowUsb/master/testSetup.png)


