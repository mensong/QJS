#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//��ں���ԭ��
// return �����롣���ط�0�򲻼��ز��
typedef int (*FN_entry)(ContextHandle ctx);

//��չ����ԭ��
typedef ValueHandle(*FN_JsFunction)(
	ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data);

/*
������չ���裺
1.�½�һ��DLL����
2.����Extend.h
3.ʵ�ֲ�������ں���entry����ѡ��
4.ʵ�ֲ�������չ����
5.ʹ��LoadExtend���ؼ���
*/