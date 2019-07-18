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

#### Test setup
ESP2866, ESP01 and ESP32(usb)
![Test setup](https://raw.githubusercontent.com/arttupii/EspNowUsb/master/testSetup.png)


#### Nodejs-server example. 
Nodejs-server configures usb-adapter and receives some messages from mesh
```javascript
const Promise = require('bluebird');
const si = require('./serialInterface');

si.begin("/dev/ttyUSB1");
si.receiveCallback(function(data){
    console.info("Received: %j", data); 
});

Promise.delay(1000)
.then(function(){
   return si.reboot().delay(3000); //Reboot usb adapter
})
.then(function(){
    return si.ping(); //Send ping to usb adapter
})
.then(function(){
    return si.role("master",3); //Set role to master and ttl=3
})
.then(function(){
    //Set secred key
    return si.setKey([0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF]);
})
.then(function(){
    return si.setChannel(1); //Set chancel
})
.then(function(){
    return si.init(); //Initialize an usb adapter
})
.then(function(){
    return si.getRTC(); //Get RTC time 
})
.then(function(){
    return si.setRTC((new Date).getTime()/1000).delay(3000); //Set RTC time
})
.then(function(){
    return si.send([1,2,3,4], 3); //Send a message to all nodes with ttl=3
})
.then(function(){
    return si.req("MARGO", 3) //Send a message to all nodes with ttl=3 and wait replies
    .then(function(replies){
        console.info("Received %d replies", replies.length);
        console.info(JSON.stringify(replies)); //Print all replies
    });
})
.catch(function(e){
console.info(e);
});
```
#####Log output
```
a@labra:~/git/EspNowUsb/RaspberryPiServer$ node index.js 
[ 'REC', '3623206401', 'ddd', [ 80, 79, 76, 79, 0 ], '' ]
begin /dev/ttyUSB1 115200
reboot
Ping
Role MASTER, ttl=3
key [0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255]
Channel 1;
Init
RTC GET
RTC 1563452038
send ttl=3, [1,2,3,4]
request ttl=3, "MARGO"
Wait all Replies: replyID=3623206401
Received: [83,76,65,86,69,40,69,83,80,48,49,41,32,72,69,76,76,79,32,77,69,83,83,65,71,69,0]
Received: [83,76,65,86,69,40,69,83,80,48,49,41,32,72,69,76,76,79,32,77,69,83,83,65,71,69,0]
Received 1 replies
[[80,79,76,79,0]]
Received: [83,76,65,86,69,40,69,83,80,48,49,41,32,72,69,76,76,79,32,77,69,83,83,65,71,69,0]
Received: [83,76,65,86,69,40,69,83,80,48,49,41,32,72,69,76,76,79,32,77,69,83,83,65,71,69,0]
^C
a@labra:~/git/EspNowUsb/RaspberryPiServer$ 
```

