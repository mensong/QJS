#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//入口函数原型
// return 错误码。返回非0则不加载插件
typedef int (*FN_entry)(ContextHandle ctx);

//扩展函数原型
typedef ValueHandle(*FN_JsFunction)(
	ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data);

/*
创建扩展步骤：
1.新建一个DLL工程
2.包含Extend.h
3.实现并导出入口函数entry（可选）
4.实现并导出扩展函数
5.使用LoadExtend加载即可
*/