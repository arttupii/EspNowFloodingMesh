const Promise = require('bluebird');
const si = require('./serialInterface');
var _=require("underscore");
var simpleMqtt=require("./simpleMqtt");
const config = require("./config.js")

si.begin(config.usbPort);
si.receiveCallback(function(replyId, data){
    console.info("Received: %j", data);
    simpleMqtt.parse(si, replyId, data);
});

function setup() {
  setInterval(function(){
      var epoch = (new Date).getTime()/1000;
      si.setRTC(epoch);
   }, 5*60*1000);

  return Promise.delay(1000)
  .then(function(){
     return si.reboot().delay(3000);
  })
  .then(function(){
      return si.role("master",3);
  })
  .then(function(){
    return si.setInitializationVector(config.mesh.initializationVector);
  })
  .then(function(){
      return si.setKey(config.mesh.secredKey);
  })
  .then(function(){
      return si.setChannel(config.mesh.channel);
  })
  .then(function(){
      return si.init();
  })
  .then(function(){
      return si.setRTC((new Date).getTime()/1000).delay(3000);
  }).catch(function(e){
    console.info(e);
  });
}

setup();
