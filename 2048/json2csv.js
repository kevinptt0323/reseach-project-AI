var obj = require('./result-61-61.json');
var str = "";

for(var filename in obj) {
  str += filename + ",";
}
str += "\n";

for(var i=0; i<1000; i++) {
  for(var filename in obj) {
    str += obj[filename][i] + ",";
  }
  str += "\n";
}
console.log(str);
