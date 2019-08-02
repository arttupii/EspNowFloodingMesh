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

function initMqttCacheObject(shortTopic){
  if(mqttCache[shortTopic]===undefined) {
    mqttCache[shortTopic] = {
      value:"",
      lastUpdate:"",
      subscribedBy:[],
      status:{
        errorStatus:"",
        lastUpdate:""
      }
    }
  }
}

function updateValueMqttCache(shortTopic, value) {
  console.info("updateValueMqttCache shortTopic:%s, value=\"%s\"",shortTopic, value);
  initMqttCacheObject(shortTopic);
  mqttCache[shortTopic].value = value;
  mqttCache[shortTopic].lastUpdate = new Date(Date.now()).toString();
  writeChacheFile();
}
function isSubscribedMqttCache(shortTopic) {
  console.info("isSubscribedMqttCache shortTopic:%s",shortTopic);

  initMqttCacheObject(shortTopic);
  return mqttCache[shortTopic].subscribedBy.length>0;
}
function addSubscriber(shortTopic, deviceName) {
  console.info("addSubscriber shortTopic:%s",shortTopic);
  initMqttCacheObject(shortTopic);
  mqttCache[shortTopic].subscribedBy.push(deviceName);
  mqttCache[shortTopic].subscribedBy = _.uniq(mqttCache[shortTopic].subscribedBy);
}
function deleteSubscriber(shortTopic, deviceName) {
  if(mqttCache.hasOwnProperty(shortTopic)){
      mqttCache[shortTopic].subscribedBy = _.filter(mqttCache[shortTopic].subscribedBy, function(s){
        return s!==deviceName;
      });
  }
}
function updateErrorMqttCache(shortTopic, status) {
  initMqttCacheObject(shortTopic);
  mqttCache[shortTopic].status.errorStatus = status;
  mqttCache[shortTopic].status.lastUpdate = new Date(Date.now()).toString();
  writeChacheFile();
}

function generateDataUpdateMsg(shortTopic, value, buffer) {
  if(buffer!==undefined) {
    if(buffer=="") {
      buffer+="MQTT MASTER\n";
      return buffer+="P:"+shortTopic+" "+value+"\n";
    }
  }
  return "MQTT MASTER\nP:"+shortTopic+" "+value+"\n";
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

  if(isSubscribedMqttCache(shortTopic)) {
    console.info("Topic has been subscribed --> publish it to mesh network");
    var msg = generateDataUpdateMsg(shortTopic,v);
    var tryCnt=3;
    function send() {
      return si.req(msg, config.mesh.ttl)
      .then(function(){
        updateErrorMqttCache(shortTopic, "OK");
      })
      .catch(function (e) {
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
  }else {
    console.info("No subscriber for topic %s", shortTopic);
  }
});


function parse(replyId, data) {
  function decompressTopic(topic) {
    if(topic[0]!==".") {
        this.t = topic;
        return topic;
    }
    var pCount=0;
    for(pCount;topic[pCount]==='.';pCount++);
    var slitIndex = 0;

    for(var i=0;i<t.length;i++) {
        if(this.t[i]==="/") {
            slitIndex++;
        }
        if(slitIndex===pCount) {
            slitIndex=i;
            break;
        }
    }
    return t.substring(0,slitIndex)+topic.substring(pCount);
}

  var str = _.map(data,function(c){
    return String.fromCharCode(c);
  }).join("");
  var deviceName = "";

  console.info("Parse: \"%s\"", str);


  if(str.indexOf("MQTT")===0) {
    var subscribedTopics = []
    _.each(str.split("\n"), function(c){
      if(c.indexOf("MQTT ")===0){
        var s = c.split(" ");
        deviceName = s[1];
        console.info("DeviceName: \"%s\"", deviceName);
        return;
      }
      if(c.indexOf("S:")===0||c.indexOf("P:")===0||c.indexOf("U:")===0||c.indexOf("G:")===0) {
        var s = c.split(":");
        var value="", shortTopic="";
        if(s[1]!==undefined){
          var e = s[1].indexOf(" ");
          if(e==-1) {
            e = s[1].length;
          }
          shortTopic = decompressTopic(s[1].substring(0,e));
          value = s[1].substring(s[1].indexOf(" ")+1);
        }
        if(s[0]==="S") {
          console.info("Subscribe ", config.mqtt.root+shortTopic);
          client.subscribe(config.mqtt.root+shortTopic);
          subscribedTopics.push(shortTopic);
          addSubscriber(shortTopic, deviceName);
        }
        if(s[0]==="G") {
          console.info(s);
          console.info("Get ", config.mqtt.root+shortTopic);
          subscribedTopics.push(shortTopic);
        }
        if(s[0]==="P") {
          client.publish(config.mqtt.root+shortTopic,value);
          updateValueMqttCache(shortTopic, value);
        }
        if(s[0]==="U") { //Unsubscribe
          console.info("Unsubscribe ", config.mqtt.root+shortTopic);
          deleteSubscriber(shortTopic, deviceName);
        }
        updateErrorMqttCache(shortTopic, "OK");
      }
  });
  if(parseInt(replyId)>0) {
    //Ack required
    console.info("MQTT: Ack requested by client...");
    var msg = "";
    _.forEach(subscribedTopics, function(t){
      if(!t.match(/.*\/trigger\/.*\/value$/)){
        if(mqttCache.hasOwnProperty(t)) {
          var value = mqttCache[t].value;
          msg=generateDataUpdateMsg(t, value, msg);
        }
      }
    });
    if(msg==="") {
      msg = "MQTT MASTER\n";
    }
    console.info("Send ack to client. ReplyId=%d, msg\"%s\"", replyId, msg);

    si.reply(msg, replyId, config.mesh.ttl);
  }
}
}

module.exports.parse=parse;
