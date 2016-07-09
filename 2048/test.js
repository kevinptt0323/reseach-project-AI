const spawn = require('child_process').spawn;
//const ID = process.argv[2];
const speeds = [0.04, 0.01, 0.005];

const run = function(ID) {
  if( ID>60 ) return;
  const childs = speeds.map(function(speed) {
    console.log(`start ${ID} ${speed}`);
    const child = spawn('./learn.out', ['zero' + ID + '.dat', 'LR', 10000, speed]);
    spawn('renice', [20, child.pid]);
    child.stdout.on('end', function() {
      console.log(`end ${ID} ${speed}`);
      run(ID+1);
    });
  });
}

run(1);
