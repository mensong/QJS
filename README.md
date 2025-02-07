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
