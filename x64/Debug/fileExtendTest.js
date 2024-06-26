
process.isDebug = true;

var text = "123\n456\n我\n爱\n你\n000";
File.writeTextFile("1.txt", text, "GBK");

File.readTextFile("1.txt", function(line, no){
	debug(line);
}, "GBK");

var filename = process.env.getEnv('temp') + "\\mensong.txt";
File.writeTextFile(filename, "mensong");
debug(File.readTextFile(filename));