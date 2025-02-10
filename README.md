# QJS

quickjs(https://github.com/mensong/quickjs-win) 在windows上的封装

# C++使用

1.把QJS.dll放到调用者exe同目录

2.代码包含QJS.h

3.使用qjs.XXX就可以内嵌js引擎了

# C#使用

1.引用QJSharp模块

2.using (QJS qjs = new QJS()){ //... }

# C++示例

[Test.cpp](https://github.com/mensong/QJS/blob/master/Test/Test.cpp "Test.cpp")

# C#示例

[Form1.cs](https://github.com/mensong/QJS/blob/master/CSharp/QJSharpTest/Form1.cs "Form1.cs")

# C#安装步骤

1.Nuget搜索“QJSharp”，如果当前项目是.net framework框架，则选择“QJSharp.framework”

2.下载回来后，选中“QJS.dll/JsExtendBase.dll/JsExtendDebugger.dll/JsExtendFile.dll(StringConvert.dll)/JsExtendPath.dll/JsExtendReg.dll”

3.右键->属性，在“复制到输出目录”选择“始终复制”，在“生成操作”选择“无”

4.生成项目即可