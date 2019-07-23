const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')

const Promise = require('bluebird');
const cmdParser = require('./parser');
const si = require('./serialInterface');
const _ = require("underscore");
const parser = new Readline()
var callback;

var port;
function begin(portName, baudRate=115200) {
    port = new SerialPort(portName, { "baudRate": baudRate });
    port.pipe(parser);
    console.info("begin", portName, baudRate);
}

var replyIdDB = {};

var actNackCallback;
parser.on('data', function(line){
    var parameters = cmdParser.parse(line);
    if(parameters[0]===("ACK")||parameters[0]===("NACK")) {
        //ACK received;
        if(actNackCallback) {
            actNackCallback(parameters);
        }
    }
    if(parameters[0]==="REC"){
        if(parameters[1]==0) {
            //Normal message
            callback(0,parameters[2]);
        } else {
            //Reply message
            var replyId = parameters[1];
            if(replyIdDB.hasOwnProperty(replyId)) {
                replyIdDB[replyId].add(parameters[2]);
            } else {
              callback(replyId,parameters[2]);
            }
        }
    }
});

function waitAckNack() {
    if(actNackCallback) {
actNackCallback();
    }
    return new Promise(function(resolve){
        actNackCallback = resolve;
    });
}

var waitList=[];
function mutex(m){
  if(m){
    var p = new Promise(function(r){
        waitList.push(r);
        if(waitList.length===1) {
          r();
        }
    });
    return p;
  } else {
    waitList.shift();
    if(waitList.length) {
      waitList[0]();
    }
  }
}

function ping() {

    return mutex(true).then(function(done){
            console.info("Ping");
            var ret = new Promise(function(resolve, reject){
                return waitAckNack()
                .then(function(p){
                    if(p[0]==="ACK") {
                        resolve();
                    } {
                       reject("PING FAILED");
                    }
                });
            }).timeout(
            1000, "Ping operation timed out");
            port.write('PING;\n')
            return ret.then(function(){
              mutex(false);
        }).error(function(e){
            return Promise.reject(e);
        });
    });
}
function init() {
    return mutex(true).then(function(done){
    console.info("Init");
    port.write('INIT;')
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("INIT FAILED");
            }
        })
      }).timeout(
    5000, "Init operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
    });
}
function role(role, ttl) {
    return mutex(true).then(function(done){
    if(role=="master") {
        role="MASTER";
    } else {
        role="SLAVE";
    }
    console.info("Role %s, ttl=%d", role, ttl);

    port.write('ROLE ' + role + " " + ttl + ";")
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("ROLE FAILED");
            }
        })
      }).timeout(
    1000, "Role operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function setChannel(c) {
    return mutex(true).then(function(done){
    console.info("Channel %d;", c);

    port.write("CHANNEL SET " +  c + ";")
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("CHANNEL FAILED");
            }
        })
      }).timeout(
    1000, "Channel operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function reboot() {
    return mutex(true).then(function(done){
    console.info("reboot");

    port.write("REBOOT;");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("REBOOT FAILED");
            }
        });
      }).timeout(
    1000, "Reboot operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function setKey(key) {
    return mutex(true).then(function(done){
    console.info("key %j", key);

    if(key.length!==16) return Promise.reject("Invalid key size");

    port.write("KEY SET [" + _.map(key,function(a){
        return a.toString(16).toUpperCase();
    }).join(",")+"];");

    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("KEY FAILED");
            }
        });
      }).timeout(
    1000, "Key operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function setBSID(bsid) {
    return mutex(true).then(function(done){

      var h = "{" + _.map(bsid,function(a){
          return "0x"+a.toString(16).toUpperCase();
      }).join(",")+"}";

    console.info("SET BSID---> %j (HOX!!! SET THIS VALUE TO ALL YOUR NODES --> \"char bsid[] = %s;\")", h,h);

    if(bsid.length!==6) return Promise.reject("Invalid key size");

    port.write("BSID SET [" + _.map(bsid,function(a){
        return a.toString(16).toUpperCase();
    }).join(",")+"];");

    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("bsid FAILED");
            }
        });
      }).timeout(
    1000, "Bsid operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

function setInitializationVector(iv) {
    return mutex(true).then(function(done){
    console.info("InitializationVector %j", iv);

    if(iv.length!==16) return Promise.reject("Invalid key size");

    port.write("IV SET [" + _.map(iv,function(a){
        return a.toString(16).toUpperCase();
    }).join(",")+"];");

    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("KEY FAILED");
            }
        });
      }).timeout(
    1000, "InitializationVector operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function convertToBinaryArray(a){
    function f(c) {
        return _.map(c, function(b){
            return b.toString(16).toUpperCase();
        }).join(",");
    }
    if(Array.isArray(a)){
        return f(a);
    } else {
        return f(_.map(a.toString().split(""), function(a){
          return a.charCodeAt(0);
        })) + ",0";
    }
}

function send(message, ttl=0) {
    return mutex(true).then(function(){
    var j = convertToBinaryArray(message);
    console.info("send ttl=%d, %s", ttl, j);

    port.write("SEND " + ttl +" [" + j +"];");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("SEND FAILED");
            }
        });
      }).timeout(
    1000, "Send operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

function reply(message, replyPrt, ttl=0) {
    return mutex(true).then(function(){
    var j = convertToBinaryArray(message);
    console.info("send ttl=%d, %s", ttl, j);

    port.write("REPLY " + ttl + " " + +replyPrt + " [" + j +"];");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("SEND FAILED");
            }
        });
      }).timeout(
    1000, "Reply operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

function request(message, ttl=0, timeout=3000, replyCount=1) {
    var ret;
    return mutex(true).then(function(){
    console.info("request ttl=%d, %j", ttl, message);
    var replyId;
    port.write("REQ " + ttl +" [" + convertToBinaryArray(message) +"];");
    return new Promise(function(resolve, reject){
        waitAckNack()
        .error(function(){})
        .then(function(r){
          mutex(false);
          return r;
        })
        .then(function(p){
          replyId = p[1];

            if(p[0]==="ACK") {
                replyIdDB[replyId] = {
                  data: [],
                  add: function(a){
                    this.data.push(a);
                    replyCount--;
                    ret = this.data;
                    if(replyCount<=0){
                      resolve(this.data);
                    }
                  }
                };
                console.info("Wait all Replies with replyId: " + p[1]);
            } else {
               reject("SEND FAILED");
            }
        });
    }).timeout(
    timeout)
    .error(function(e) {
      console.info("Timeout", e);
    }).then(function(){
      console.info("Request handled");
  });
}).then(function(){
  return ret;
});
}

function getRTC(message) {
    return mutex(true).then(function(done){
    console.info("RTC GET");

    port.write("RTC GET;");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(parseInt(p[1]));
            } {
               reject("RTC GET FAILED");
            }
        });
      }).timeout(
    1000, "RTC GET operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function getMAC() {
    return mutex(true).then(function(done){
    console.info("MAC GET");

    port.write("MAC;");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(p[1]);
            } {
               reject("MAC GET FAILED");
            }
        });
      }).timeout(
    1000, "MAC GET operation timed out").then(function(ret){
      mutex(false);
      return ret;
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

function setRTC(epoch) {
    return mutex(true).then(function(done){
    epoch = parseInt(epoch);
    console.info("RTC %d", epoch);

    port.write("RTC SET " + epoch +";");
    return new Promise(function(resolve, reject){
        return waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(parseInt(p[1]));
            } {
               reject("RTC SET FAILED");
            }
        });
      }).timeout(
    1000, "RTC SET operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

module.exports.begin = begin;
module.exports.receiveCallback = function(cb) {
    callback = cb;
}
module.exports.init = init;
module.exports.role = role;
module.exports.setKey = setKey;
module.exports.ping = ping;
module.exports.setChannel = setChannel;
module.exports.reboot = reboot;
module.exports.getRTC = getRTC;
module.exports.setRTC = setRTC;
module.exports.setInitializationVector = setInitializationVector;
module.exports.setBSID = setBSID;
module.exports.getMAC = getMAC;
module.exports.send = send;
module.exports.req = request;
module.exports.reply = reply;
