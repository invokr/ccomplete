var ccomplete = require("../build/Release/ccomplete.node");

var ccomplete = new ccomplete.ccomplete();

ccomplete.args = new Array("-std=c++0x", "-I/usr/include", "-I/usr/lib64/clang/3.3/include/");
ccomplete.cachesize = 100;
console.log(ccomplete.args);
console.log(ccomplete.Version());

console.log(
    ccomplete.Diagnose("/home/cloud9/workspace/cloud9/c9complete/native/demo/demo.cpp")
);

var start = new Date().getTime();
ccomplete.Complete("/home/cloud9/workspace/cloud9/c9complete/native/demo/demo.cpp", 7, 8)
var end = new Date().getTime() - start;
console.log('Execution time 1: ' + end);

start = new Date().getTime();
ccomplete.Complete("/home/cloud9/workspace/cloud9/c9complete/native/demo/demo.cpp", 7, 8)
end = new Date().getTime() - start;
console.log('Execution time 2: ' + end);

start = new Date().getTime();
ccomplete.Complete("/home/cloud9/workspace/cloud9/c9complete/native/demo/demo.cpp", 7, 8)
end = new Date().getTime() - start;
console.log('Execution time 5: ' + end);


console.log(
    ccomplete.Complete("/home/cloud9/workspace/cloud9/c9complete/native/demo/demo.cpp", 7, 8)
);

var util = require('util');
console.log(util.inspect(process.memoryUsage()));