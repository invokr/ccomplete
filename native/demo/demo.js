var ccomplete = require("../build/Release/obj.target/ccomplete.node");
var ccomplete = new ccomplete.ccomplete();

ccomplete.args = new Array("-std=c++0x", "-I/usr/include", "-I/usr/local/include");
ccomplete.cachesize = 100;
console.log(ccomplete.args);
console.log(ccomplete.Version());

console.log(
    ccomplete.Diagnose("./demo.cpp")
);

start = new Date().getTime();
ccomplete.Complete("./demo.cpp", 7, 8)
end = new Date().getTime() - start;
console.log('Execution time: ' + end);

console.log(
    ccomplete.Complete("./demo.cpp", 7, 8)
);

var util = require('util');
console.log(util.inspect(process.memoryUsage()));
