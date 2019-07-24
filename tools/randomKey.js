function randomIntFromInterval(min,max) // min and max included
{
    return Math.floor(Math.random()*(max-min+1)+min);
}

tmp="const char key[]={";
for(var i=0;i<16;i++) {
  console.info("LOOP");
  if(i===0) {
    tmp+="0x"+randomIntFromInterval(0,255).toString(16);
  }else {
     tmp+=", "+"0x"+randomIntFromInterval(0,255).toString(16);
  }
}

tmp+="};"
console.info(tmp)
