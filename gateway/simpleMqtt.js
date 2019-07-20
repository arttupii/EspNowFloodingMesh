const _ = require("underscore");
var mqtt = require('mqtt')
const config = require("./config.js")
const si = require('./serialInterface');
const fs = require('fs');

var client  = mqtt.connect(config.mqtt.host)

var mqttCache = {};

var writeDelayed = 0;
function writeChacheFile() {
 if(writeDelayed===0) {
    setTimeout(function(){
      if(writeDelayed>1) {
        writeDelayed=0;
        writeChacheFile();
      }
      writeDelayed=0;
    },15000);
    fs.writeFileSync(config.dbCacheFile, JSON.stringify(mqttCache,0,3));
    console.info("Update cache file...");
  }
  writeDelayed++;
}

function readCacheFileFromDisk(){
  var rawdata;
   try {
    rawdata = fs.readFileSync(config.dbCacheFile);
  } catch(e){}
  if(rawdata) {
    mqttCache = JSON.parse(rawdata);
  } else {
    mqttCache = {};
  }
}

readCacheFileFromDisk();

client.on('connect', function () {
  _.forEach(_.keys(mqttCache), function(topic){
    console.info("Subscribe topic %s from cache", topic);
    client.subscribe(config.mqtt.root+topic);
  });
})

function updateValueMqttCache(shortTopic, value) {
  if(mqttCache[shortTopic]===undefined) {
    mqttCache[shortTopic] = {
      status:{}
    }
  }
  mqttCache[shortTopic].value = value;
  mqttCache[shortTopic].lastUpdate = new Date(Date.now()).toString();
  writeChacheFile();
}
function updateErrorMqttCache(shortTopic, status) {
  if(mqttCache[shortTopic]===undefined) {
    mqttCache[shortTopic] = {
      status:{}
    }
  }
  mqttCache[shortTopic].status.errorStatus = status;
  mqttCache[shortTopic].status.lastUpdate = new Date(Date.now()).toString();
  writeChacheFile();
}

function generateDataUpdateMsg(shortTopic, value, buffer) {
  if(buffer!==undefined) {
    if(buffer=="") {
      buffer+="MQTT\n";
      return buffer+="M:"+shortTopic+":"+value+"\n";
    }
  }
  return "MQTT\nM:"+shortTopic+":"+value+"\n";
}

client.on('message', function (topic, message) {
  // message is Buffer
  console.log("From MQTT broker:"+message.toString())
  console.info("     --->", topic);

  var shortTopic=topic.replace(config.mqtt.root,"");
  var payload = message.toJSON().data;
  var v = _.map(payload,function(c){
    return String.fromCharCode(c);
  }).join("");

  updateValueMqttCache(shortTopic, v);

  var msg = generateDataUpdateMsg(shortTopic,v);
  var tryCnt=3;
  function send() {
    return si.req(msg, config.mesh.ttl)
    .then(function(){
      updateErrorMqttCache(shortTopic, "OK");
    })
    .error(function (e) {
      console.info("No reply from node...");
      tryCnt--;
      if(tryCnt>0) {
        console.info("Try again...");
        return send();
      } else {
        updateErrorMqttCache(shortTopic, "NoReceiver");
      }
    });
  }
  send();
});

function parse(simpleMqtt, replyId, data) {
  var str = _.map(data,function(c){
    return String.fromCharCode(c);
  }).join("");

  if(str.indexOf("MQTT")===0){
    var subscribedTopics = []
    _.each(str.split("\n"), function(c){
      if(c.indexOf("S:")===0||c.indexOf("P:")===0) {
        var s = c.split(":");
        if(s[0]==="S") {
          console.info("Subscribe ", config.mqtt.root+s[1]);
          client.subscribe(config.mqtt.root+s[1]);
          subscribedTopics.push(s[1]);
        }
        if(s[0]==="P") {
          console.info("Publish ", config.mqtt.root+s[1],s[2]);
          client.publish(config.mqtt.root+s[1],s[2]);
          updateValueMqttCache(s[1], s[2]);
        }
        updateErrorMqttCache(s[1], "OK");
      }
  });
  if(parseInt(replyId)>0) {
    //Ack required
    console.info("MQTT: Ack requested by client...");
    var msg = "";
    _.forEach(subscribedTopics, function(t){
      if(mqttCache.hasOwnProperty(t)) {
        var value = mqttCache[t].value;
        msg=generateDataUpdateMsg(t, value, msg);
      }
    });
    console.info("Send ack to client. ReplyId=%d, msg\"%s\"", replyId, msg);
    si.reply(msg, replyId, config.mesh.ttl);
  }
}
}

module.exports.parse=parse;
