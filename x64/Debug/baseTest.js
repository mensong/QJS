
process.isDebug = true;

debug(debugObject(process));
debug(debugObject(process.env));

var m = require("module.js");
debug(debugObject(m));
var util = require("util.js");
debug(debugObject(util));

console.log("我","是","console.log");
