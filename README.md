Espnow mesh usb adapter for esp32/esp2866

Dependencies:
https://github.com/arttupii/espNowAESBroadcast
https://github.com/arttupii/ArduinoCommands


## Exambles;
### Initialize mesh network
>ROLE MASTER;
<ACK;
>KEY SET [00,11,22,33,44,55,66,77,88,99,AA,BB,CC,DD,EE,FF];
<ACK;
>CHANNEL SET 1;
<ACK;
>INIT;
<ACK;
### Reboot usb adapter
>REBOOT;
<ACK REBOOTING;
<READY;
### Send message with ttl 3
>SEND 3 [11,22,33,44,55,66]
>ACK
###
### Request with ttl 3 (nodes/node send/sends reply with 2314 replyId)
>REQ 3 [11,22,33,44,55,66]
>ACK 2314
###
### Message received
REC 0 [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
###
### Request with ttl 3 (nodes/node send/sends reply with 2314 replyId)
>REQ 3 [11,22,33,44,55,66]
>ACK 2314
###
### Message received with replyId
REC 2314 [53,4C,41,56,45,20,48,45,4C,4C,4F,20,4D,45,53,53,41,47,45,0];
###
### Invalid command
>ABCD;
<NACK INVALID COMMAND
### 
REC 1234 [53,1C,42,56,45,23,44,45,0];
