var _ = require("underscore");

function parse(line) {
    var buffer=[];
    var paramIndex=0;
    var binaryData=[];

    var commentStarted=false;
    var controlMark=false;
    var index=0;

    var parameters =[];

    function p(a){
          if(a=='\\' && controlMark==false) {
            controlMark = true;
            return;
          }
          if(controlMark) {
            controlMark = false;
            buffer.push(a);
            return;
          } 

        if(a=='#') {
          commentStarted = true;
          return;
        }

        if(commentStarted) {
          if (a == '\n' || a == '\r') {
            commentStarted=false;
          }
          return;
        }
        
        if (a == ' ' || a == '\t' || a == ';') {
            //Handle params
          parameters.push(buffer.join(""));
          buffer =[];
          return;
        } else if (a == '[') {
          buffer = [];
          binaryDataStarted = true;
          return;
        } else if (a == '\n' || a == '\r') {
        } else if (a == ']' /*|| a == ','*/) {
          parameters.push(_.map(buffer.join("").split(","), function(a){
            return parseInt(a,16);
          }));
          buffer = [];
          return;
        } else {

        }
        if (a == ';') {
          parameters = parameters.pop();
        }
        buffer.push(a);

    }

    for(var i=0;i<line.length;i++){
        p(line[i]);    
    };
    return parameters;
}

console.info(parse("REC 3623206401 ddd [50,4F,4C,4F,0];"));

module.exports.parse = parse;
