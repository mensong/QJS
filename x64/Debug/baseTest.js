
process.isDebug = true;

debug(debugObject(process));
debug(debugObject(process.env));

var m = require("module.js");
debug(debugObject(m));

include("incTest.js");

console.log("我","是","console.log");
