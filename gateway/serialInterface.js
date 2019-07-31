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
    if(line.indexOf("READY;")!==-1) {
	console.info("Reboot detected!!!");
	callback(0,0,"REBOOT");
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
            5000, "Ping operation timed out");
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

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("INIT FAILED");
            }
        });
        port.write('INIT;')
        return ret;
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

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("ROLE FAILED");
            }
        })
        port.write('ROLE ' + role + " " + ttl + ";")
        return ret;
      }).timeout(
    5000, "Role operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function setChannel(c) {
    return mutex(true).then(function(done){
    console.info("Channel %d;", c);

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("CHANNEL FAILED");
            }
        })
        port.write("CHANNEL SET " +  c + ";")
        return ret;
      }).timeout(
    5000, "Channel operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function reboot() {
    return mutex(true).then(function(done){
    console.info("reboot");

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("REBOOT FAILED");
            }
        });
        port.write("REBOOT;");
        return ret;
      }).timeout(
    3000, "Reboot operation timed out").then(function(){
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



    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("KEY FAILED");
            }
        });
        port.write("KEY SET [" + _.map(key,function(a){
            return a.toString(16).toUpperCase();
        }).join(",")+"];");
        return ret;
      }).timeout(
    5000, "Key operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function setBSID(bsid) {
    return mutex(true).then(function(done){

    console.info("SET BSID---> 0x%s", bsid.toString(16));

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("bsid FAILED");
            }
        });
        port.write("BSID SET " + bsid + ";");
        return ret;
      }).timeout(
    5000, "Bsid operation timed out").then(function(){
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


    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("KEY FAILED");
            }
        });
        port.write("IV SET [" + _.map(iv,function(a){
            return a.toString(16).toUpperCase();
        }).join(",")+"];");

        return ret;
      }).timeout(
    5000, "InitializationVector operation timed out").then(function(){
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

    return new Promise(function(resolve, reject){
        var ret  = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("SEND FAILED");
            }
        });
        port.write("SEND " + ttl +" [" + j +"];");
        return ret;
      }).timeout(
    5000, "Send operation timed out").then(function(){
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

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve();
            } {
               reject("SEND FAILED");
            }
        });
        port.write("REPLY " + ttl + " " + +replyPrt + " [" + j +"];");
        return ret;
      }).timeout(
    5000, "Reply operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}

function request(message, ttl=0, timeout=3000, wantedReplyCount=1) {
  function reqSend(){
        var ret;
        return mutex(true).then(function(){
        console.info("request ttl=%d, %j", ttl, message);
        var replyId;
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
                        wantedReplyCount--;
                        ret = this.data;
                        if(wantedReplyCount<=0){
                          resolve(this.data);
                        }
                      }
                    };
                    console.info("Wait all Replies with replyId: " + p[1]);
                } else {
                   reject("SEND FAILED");
                }
            });
            port.write("REQ " + ttl +" [" + convertToBinaryArray(message) +"];");
        }).timeout(
        3000, "REQ operation timed out")
        .error(function(e) {
          console.info("MESSAGE!!!:", message);
          console.info("Timeout!!", e);
        }).then(function(){
          console.info("Request handled");
      });
    }).then(function(){
      return ret;
    });
  }
  return reqSend();
}

function getRTC(message) {
    return mutex(true).then(function(done){
    console.info("RTC GET");

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(parseInt(p[1]));
            } {
               reject("RTC GET FAILED");
            }
        });
        port.write("RTC GET;");

        return ret;
      }).timeout(
    5000, "RTC GET operation timed out").then(function(){
      mutex(false);
    }).error(function(e){
        return Promise.reject(e);
    });
});
}
function getMAC() {
    return mutex(true).then(function(done){
    console.info("MAC GET");

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(p[1]);
            } {
               reject("MAC GET FAILED");
            }
        });
        port.write("MAC;");
        return ret;
      }).timeout(
    5000, "MAC GET operation timed out").then(function(ret){
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

    return new Promise(function(resolve, reject){
        var ret = waitAckNack()
        .then(function(p){
            if(p[0]==="ACK") {
                resolve(parseInt(p[1]));
            } {
               reject("RTC SET FAILED");
            }
        });
        port.write("RTC SET " + epoch +";");
        return ret;
      }).timeout(
    5000, "RTC SET operation timed out").then(function(){
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
