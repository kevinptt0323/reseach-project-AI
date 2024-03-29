const spawn = require('child_process').spawn;
//const ID = process.argv[2];
const speeds = [0.04, 0.01, 0.005];

const run = function(ID, times, speed) {
  //if( ID>60 ) return;

  console.log(`start ${ID} ${times} ${speed}`);
  const child = spawn('./learn.out', [`zero${ID}.dat`, 'LR', times, speed]);
  spawn('renice', [20, child.pid]);
  child.stdout.on('end', function() {
    console.log(`end ${ID} ${times} ${speed}`);
    run(ID+3, 10000, speed);
  });
}

run(1, 10000, 0.005);
run(2, 10000, 0.005);
run(3, 10000, 0.005);
