#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//��ں���ԭ��: QJS_API int _entry(ContextHandle ctx){}
// return �����롣���ط�0�򲻼��ز��
typedef int (*FN_entry)(ContextHandle ctx);
//������ɺ���ԭ��: QJS_API void _completed(ContextHandle ctx){}
typedef void (*FN_completed)(ContextHandle ctx);

//��չ����ԭ��
typedef ValueHandle(*FN_JsFunction)(
	ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data);

/*
������չ���裺
1.�½�һ��DLL����
2.����Extend.h
3.ʵ�ֲ�������ں���_entry����ѡ��
4.ʵ�ֲ�����������ɺ���_completed����ѡ��
5.ʵ�ֲ�������չ����
6.ʹ��LoadExtend���ؼ���
*/