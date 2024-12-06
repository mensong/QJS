
process.isDebug = true;

debug(debugObject(process));
debug(debugObject(process.env));
debug(process.env.getEnv('temp'));

var m = require("module.js");
debug(debugObject(m));

include("incTest.js");

//console.log("我","是","console.log");

alert(inputBox("inputBox测试", "标题", "内容"));

const st = Date.now();
sleep(2000);
alert("sleeped:" + (Date.now() - st));