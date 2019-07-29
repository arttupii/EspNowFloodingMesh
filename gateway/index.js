const Promise = require('bluebird');
const si = require('./serialInterface');
var _=require("underscore");
var simpleMqtt=require("./simpleMqtt");
const config = require("./config.js")
let polycrc = require('polycrc')

si.begin(config.usbPort);
si.receiveCallback(function(replyId, data){
    console.info("Received: %j", data);
    simpleMqtt.parse(replyId, data);
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
      return si.role("master");
  })
  .then(function(){
      return si.getMAC();
  })
  .then(function(mac){
      var crc24 = parseInt(polycrc.crc24(new Buffer(mac)))&0xffffff;

      if(config.mesh.bsid!==crc24 && config.mesh.bsid===0x112233) {
        console.info("(HOX!!! SET THIS VALUE TO ALL YOUR NODES --> \"const int bsid = 0x%s;\"). Update also config.js!!!", crc24.toString(16));
        console.info("Default Bsid is used!!!");
      }
      return si.setBSID(config.mesh.bsid);

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
