#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//���
typedef int (*FN_entry)(ContextHandle ctx);

//��ú����б�
typedef const char* (*FN_function_list)();

//����ԭ��
typedef ValueHandle(*FN_JsFunction)(
	ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data);