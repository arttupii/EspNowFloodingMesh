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
  console.info("---------->",msg);
  si.send(msg, config.mesh.ttl);
  //client.end()
})

function parse(data) {
  var str = _.map(data,function(c){
    return String.fromCharCode(c);
  }).join("");

  if(str.indexOf("MQTT")===0){
    console.info("1111111",str);

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
}
}
/*var t = "MQTT\nP:device1/led/value:on\nP:device2/led/value:on\n".split("");
t = _.map(t,function(c){
  return c.charCodeAt(0);
});
parse(t)*/
module.exports.parse=parse;
