#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//入口函数原型: QJS_API int _entry(ContextHandle ctx, void* user_data, int id){}
// return 错误码。返回非0则不加载插件
typedef int (*FN_entry)(ContextHandle ctx, void* user_data, int id);

//加载完成函数原型: QJS_API void _completed(ContextHandle ctx, void* user_data, int id){}
typedef void (*FN_completed)(ContextHandle ctx, void* user_data, int id);

//卸载函数原型：QJS_API void _unload(ContextHandle ctx, void* user_data, int id){}
typedef void (*FN_unload)(ContextHandle ctx, void* user_data, int id);

//扩展函数原型: 
// QJS_API ValueHandle F_funcionNameInJs(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// QJS_API ValueHandle G_jsObjName(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// QJS_API ValueHandle S_jsObjName(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// 1.函数命名的格式为 F_funcionNameInJs，例如：F_alert在js中的方法名即为alert
// 2.getter/setter 函数命名格式为: 
//   G_jsObjName - 创建一个getter的变量
//   S_jsObjName - 创建一个setter的变量
typedef ValueHandle(*FN_JS_ExtendFunction)(ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data, int id);



/*
创建扩展步骤：
1.新建一个DLL工程
2.包含Extend.h
3.实现并导出入口函数_entry（可选）
4.实现并导出加载完成函数_completed（可选）
5.实现并导出卸载函数_unload（可选）
6.实现并导出扩展函数
7.使用LoadExtend加载即可
*/

