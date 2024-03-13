#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//入口
typedef int (*FN_entry)(ContextHandle ctx);

//获得函数列表
typedef const char* (*FN_function_list)();

//函数原型
typedef ValueHandle(*FN_JsFunction)(
	ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data);