const _ = require("underscore");
var mqtt = require('mqtt')
const config = require("./config.js")
const si = require('./serialInterface');

var client  = mqtt.connect(config.mqtt.host)


client.on('connect', function () {
  client.subscribe('presence', function (err) {
    if (err) {
        console.info(err);
      }
  })
})

client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())
  var shortTopic=topic.replace(config.mqtt.root,"");
  var payload = message.toJSON().data;
  var v = _.map(payload,function(c){
    return String.fromCharCode(c);
  }).join("");

  var msg = "MQTT\nM:"+shortTopic+":"+v+"\n";
  var tryCnt=3;
  function send() {
    return si.req(msg, config.mesh.ttl)
    .error(function (e) {
      console.info("No reply from node...");
      tryCnt--;
      if(tryCnt>0) {
        console.info("Try again...");
        return send();
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
    _.each(str.split("\n"), function(c){
      if(c.indexOf("S:")===0||c.indexOf("P:")===0) {
        var s = c.split(":");
        if(s[0]==="S") {
          console.info("Supscribe ", config.mqtt.root+s[1]);
          client.subscribe(config.mqtt.root+s[1]);
        }
        if(s[0]==="P") {
          console.info("Publish ", config.mqtt.root+s[1],s[2]);
          client.publish(config.mqtt.root+s[1],s[2]);
        }
      }
  });
  if(parseInt(replyId)>0) {
    //Ack required
    console.info("MQTT: Ack requested by client...")
    si.reply("ACK", replyId, config.mesh.ttl);
  }
}
}

module.exports.parse=parse;
