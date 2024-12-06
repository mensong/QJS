#pragma once
#define QJS_EXPORTS
#include "QJS.h"

//��ں���ԭ��: QJS_API int _entry(ContextHandle ctx, void* user_data, int id){}
// return �����롣���ط�0�򲻼��ز��
typedef int (*FN_entry)(ContextHandle ctx, void* user_data, int id);

//������ɺ���ԭ��: QJS_API void _completed(ContextHandle ctx, void* user_data, int id){}
typedef void (*FN_completed)(ContextHandle ctx, void* user_data, int id);

//ж�غ���ԭ�ͣ�QJS_API void _unload(ContextHandle ctx, void* user_data, int id){}
typedef void (*FN_unload)(ContextHandle ctx, void* user_data, int id);

//��չ����ԭ��: 
// QJS_API ValueHandle F_funcionNameInJs(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// QJS_API ValueHandle G_jsObjName(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// QJS_API ValueHandle S_jsObjName(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data, int id){}
// 1.���������ĸ�ʽΪ F_funcionNameInJs�����磺F_alert��js�еķ�������Ϊalert
// 2.getter/setter ����������ʽΪ: 
//   G_jsObjName - ����һ��getter�ı���
//   S_jsObjName - ����һ��setter�ı���
typedef ValueHandle(*FN_JS_ExtendFunction)(ContextHandle ctx, ValueHandle this_val, 
	int argc, ValueHandle* argv, void* user_data, int id);



/*
������չ���裺
1.�½�һ��DLL����
2.����Extend.h
3.ʵ�ֲ�������ں���_entry����ѡ��
4.ʵ�ֲ�����������ɺ���_completed����ѡ��
5.ʵ�ֲ�����ж�غ���_unload����ѡ��
6.ʵ�ֲ�������չ����
7.ʹ��LoadExtend���ؼ���
*/

