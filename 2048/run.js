const spawnSync = require('child_process').spawnSync;
const speeds = ['0.040', '0.010', '0.005'];
const times = 5;

let res = {};

for(let i=1; i<=1; i++) {
  for(let j=1; j<=3; j++) {
    for(let speed of speeds) {
      let filename = `LR${i}-${j}-10-${speed}.dat`;
      let sum = 0;
      console.error(filename);
      res[filename] = [];

      for(let k=0; k<times; k++) {
        let child = spawnSync('./kevinptt.out', [filename]);
        let ret = child.stdout.toString().split("\n");
        res[filename].push(parseInt(ret[ret.length-2]));
      }
    }
  }
}
console.log(JSON.stringify(res, null, 2));
