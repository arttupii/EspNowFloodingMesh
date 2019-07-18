const Promise = require('bluebird');
const si = require('./serialInterface');

si.begin("/dev/ttyUSB1");
si.receiveCallback(function(data){
    console.info("Received: %j", data); 
});

Promise.delay(1000)
.then(function(){
   return si.reboot().delay(3000); 
})
.then(function(){
    return si.ping();
})
.then(function(){
    return si.role("master",3);
})
.then(function(){
    return si.setKey([0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF]);
})
.then(function(){
    return si.setChannel(1);
})
.then(function(){
    return si.init();
})
.then(function(){
    return si.getRTC();
})
.then(function(){
    return si.setRTC((new Date).getTime()/1000).delay(3000);
})
.then(function(){
    return si.send([1,2,3,4], 3);
})
.then(function(){
    return si.req("MARGO", 3)
    .then(function(replies){
        console.info("Received %d replies", replies.length);
        console.info(JSON.stringify(replies));
    });
})
.catch(function(e){
console.info(e);
});

