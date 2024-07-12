// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include "..\QJS\QJS.h"
#include "..\JsExtends\JsExtendBase\JsExtendBase.h"
#include <sstream>

void printObject(ContextHandle ctx, ValueHandle obj)
{
	QJS_SCOPE(ctx);

	ValueHandle arrKeys = qjs.GetObjectPropertyKeys(ctx, obj, false, true);
	int64_t len = qjs.GetLength(ctx, arrKeys);
	printf("↓↓↓Object property↓↓↓\n");
	for (int i = 0; i < len; i++)
	{
		ValueHandle jkey = qjs.GetIndexedJsValue(ctx, i, arrKeys);
		std::string key = qjs.JsValueToStdString(ctx, jkey);
		ValueHandle jval = qjs.GetNamedJsValue(ctx, key.c_str(), obj);
		std::string val = qjs.JsValueToStdString(ctx, jval);
		printf("%s : %s\n", key.c_str(), val.c_str());
	}

	printf("↑↑↑Object property↑↑↑\n");
}

void baseTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	ValueHandle result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx, L"var a = 123;a"), qjs.TheJsNull(), "");
	if (!qjs.JsValueIsException(result))
	{
		const char* sz = qjs.JsValueToString(ctx, result);
		printf("运行成功:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	else
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

ValueHandle JsMsg(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::string msg;
	for (int i = 0; i < argc; i++)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[1]);
		if (sz)
			msg += sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	MessageBoxA(NULL, msg.c_str(), "msg", 0);

	auto item = qjs.NewStringJsValue(ctx, msg.c_str());
	return item;
}

std::string s_str = "test_getter_setter_value";
ValueHandle JsGetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	return qjs.NewStringJsValue(ctx, s_str.c_str());
}
ValueHandle JsSetter(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
			s_str = std::string("test_getter_setter_value:") + sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	return this_val;
}

ValueHandle JsTestThrow(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	auto throwWhat = qjs.NewStringJsValue(ctx, "throw test");
	auto ex = qjs.NewThrowJsValue(ctx, throwWhat);
	return ex;
}


void myTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx), NULL);

	ValueHandle jmsg = qjs.NewFunction(ctx, JsMsg, 0, NULL);
	 qjs.SetNamedJsValue(ctx, "msg", jmsg, qjs.TheJsNull());

	{
		QJS_SCOPE(ctx);
		printObject(ctx, qjs.GetGlobalObject(ctx));
	}

	{
		QJS_SCOPE(ctx);

		auto argv = qjs.NewStringJsValue(ctx, "mensong,");
		ValueHandle argvs[] = { argv, argv, argv };
		auto ret1 = qjs.CallJsFunction(ctx, jmsg, argvs, 3, qjs.TheJsNull());
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle o = qjs.NewIntJsValue(ctx, 123);
		qjs.SetObjectUserData(o, (void*)123);
		void* pu = qjs.GetObjectUserData(o);
		int iu = (int)pu;
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle bv = qjs.NewBoolJsValue(ctx, true);
		qjs.SetNamedJsValue(ctx, "bv", bv, qjs.TheJsNull());

		ValueHandle o = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "bv", bv, o);
		qjs.SetNamedJsValue(ctx, "o", o, qjs.TheJsNull());

		printObject(ctx, o);
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle v = qjs.TheJsException(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsFalse(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsNull(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsTrue(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsUndefined(); qjs.FreeValueHandle(&v);
		v = qjs.NewArrayJsValue(ctx); qjs.FreeValueHandle(&v);
		v = qjs.NewBoolJsValue(ctx, false); qjs.FreeValueHandle(&v);
		v = qjs.NewDateJsValue(ctx, 1718849344); qjs.FreeValueHandle(&v);
		v = qjs.NewDoubleJsValue(ctx, 123.0); qjs.FreeValueHandle(&v);
		v = qjs.NewFunction(ctx, JsMsg, 0, NULL); qjs.FreeValueHandle(&v);
		v = qjs.NewInt64JsValue(ctx, 123); qjs.FreeValueHandle(&v);
		v = qjs.NewIntJsValue(ctx, 123); qjs.FreeValueHandle(&v);
		v = qjs.NewObjectJsValue(ctx); qjs.FreeValueHandle(&v);
		v = qjs.NewStringJsValue(ctx, "abc"); qjs.FreeValueHandle(&v);
		v = qjs.NewThrowJsValue(ctx, qjs.NewStringJsValue(ctx, "abc")); qjs.FreeValueHandle(&v);
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle arr = qjs.NewArrayJsValue(ctx);
		ValueHandle str = qjs.NewStringJsValue(ctx, "mensong");
		qjs.SetIndexedJsValue(ctx, 10, str, arr);//设置id 10的元素为字符串mensong
		qjs.SetIndexedJsValue(ctx, 2, str, arr);
		qjs.FreeValueHandle(&str);//测试手动释放ValueHandle
		qjs.DeleteIndexedJsValue(ctx, 2, arr);
		qjs.SetNamedJsValue(ctx, "arr", arr, qjs.TheJsNull());

		auto item10 = qjs.GetIndexedJsValue(ctx, 10, arr);
		const char* sz = qjs.JsValueToString(ctx, item10);
		qjs.FreeJsValueToStringBuffer(ctx, sz);

		auto item5 = qjs.GetIndexedJsValue(ctx, 5, arr);
		const char* sz5 = qjs.JsValueToString(ctx, item5);
		qjs.FreeJsValueToStringBuffer(ctx, sz5);

		auto arrLen = qjs.GetLength(ctx, arr);
		for (size_t i = 0; i < arrLen; i++)
		{
			ValueHandle v = qjs.GetIndexedJsValue(ctx, i, arr);
			//...
		}

		bool b = qjs.JsValueIsArray(arr);
		auto jToString = qjs.GetNamedJsValue(ctx, "toString", arr);
		b = qjs.JsValueIsFunction(jToString);
		if (b)
		{
			auto jstrToString = qjs.CallJsFunction(ctx, jToString, NULL, 0, arr);
			auto strToString = qjs.JsValueToString(ctx, jstrToString);
			qjs.FreeJsValueToStringBuffer(ctx, strToString);
		}

		ValueHandle jlen = qjs.GetNamedJsValue(ctx, "length", arr);
		arrLen = qjs.JsValueToInt(ctx, jlen, 0);

		auto jpush = qjs.GetNamedJsValue(ctx, "push", arr);
		b = qjs.JsValueIsFunction(jpush);
		if (b)
		{
			ValueHandle argv[] = {qjs.NewStringJsValue(ctx, "aaa")};
			qjs.CallJsFunction(ctx, jpush, argv, sizeof(argv) / sizeof(ValueHandle), arr);

			jlen = qjs.GetNamedJsValue(ctx, "length", arr);
			arrLen = qjs.JsValueToInt(ctx, jlen, 0);
			arrLen = 0;
		}

		printf("Array Prototype:\n");
		ValueHandle pro = qjs.GetPrototype(ctx, arr);
		ValueHandle arrKeys = qjs.GetObjectPropertyKeys(ctx, pro, false, true);
		arrLen = qjs.GetLength(ctx, arrKeys);
		for (int i = 0; i < arrLen; i++)
		{
			ValueHandle jitem = qjs.GetIndexedJsValue(ctx, i, arrKeys);
			std::string key = qjs.JsValueToStdString(ctx, jitem);
			printf("%s\n", key.c_str());
		}

		auto jprototype = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "num", qjs.NewIntJsValue(ctx, 123), jprototype);
		auto jobj = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "testproto", jobj, qjs.TheJsNull());
		bool bres = qjs.SetPrototype(ctx, jobj, jprototype);
		auto jnum = qjs.RunScript(ctx, "testproto.num", qjs.TheJsNull(), "");
		int num = qjs.JsValueToInt(ctx, jnum, 0);
		num = 0;
	}

	{
		QJS_SCOPE(ctx);

		auto jint = qjs.NewIntJsValue(ctx, 65536);
		auto intstr1 = qjs.JsValueToString(ctx, jint);
		qjs.FreeJsValueToStringBuffer(ctx, intstr1);
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle test_getter_setter = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "test_getter_setter", test_getter_setter, qjs.GetGlobalObject(ctx));
		auto getter = qjs.NewFunction(ctx, JsGetter, 0, NULL);
		auto setter = qjs.NewFunction(ctx, JsSetter, 1, NULL);
		qjs.DefineGetterSetter(ctx, test_getter_setter, "gs", getter, setter);
		qjs.RunScript(ctx, "test_getter_setter.gs='123';test_getter_setter.gs", qjs.GetGlobalObject(ctx), "");

		printObject(ctx, test_getter_setter);
	}

	{
		QJS_SCOPE(ctx);
		auto g1 = qjs.GetGlobalObject(ctx);
		auto g2 = qjs.GetGlobalObject(ctx);
		auto g3 = qjs.GetGlobalObject(ctx);
		auto g4 = qjs.GetGlobalObject(ctx);

		bool b = qjs.JsValueIsGlobalObject(ctx, g1);
		b = qjs.JsValueIsGlobalObject(ctx, qjs.NewObjectJsValue(ctx));
		b = qjs.JsValueIsGlobalObject(ctx, qjs.TheJsNull());
		b = qjs.JsValueIsGlobalObject(ctx, qjs.TheJsUndefined());
	}

	{
		QJS_SCOPE(ctx);

		auto date = qjs.NewDateJsValue(ctx, 1679044555000);
		uint64_t ts = qjs.JsValueToTimestamp(ctx, date);
		qjs.SetNamedJsValue(ctx, "mydate", date, qjs.TheJsNull());
	}

	{
		QJS_SCOPE(ctx);

		auto o = qjs.NewObjectJsValue(ctx);
		auto prop = qjs.NewIntJsValue(ctx, 123);
		qjs.SetNamedJsValue(ctx, "a", prop, o);

		auto jstr1 = qjs.JsonStringify(ctx, o);
		auto ostr1 = qjs.JsValueToString(ctx, jstr1);
		auto o2 = qjs.JsonParse(ctx, ostr1);
		qjs.FreeJsValueToStringBuffer(ctx, ostr1);
		auto jstr2 = qjs.JsonStringify(ctx, o2);
		auto ostr2 = qjs.JsValueToString(ctx, jstr2);
		qjs.FreeJsValueToStringBuffer(ctx, ostr2);

		auto jsonobj = qjs.JsonParse(ctx, "[{\"a\":123}, {\"a\":456}]");
		auto jsonobjstr = qjs.JsonStringify(ctx, jsonobj);
		const char* str = qjs.JsValueToString(ctx, jsonobjstr);
		qjs.FreeJsValueToStringBuffer(ctx, str);
	}

	{
		QJS_SCOPE(ctx);

		auto arrLenTest = qjs.RunScript(ctx, "[{\"a\":123}, {\"a\":456}]", qjs.TheJsNull(), "");
		auto arrTestLen = qjs.GetLength(ctx, arrLenTest);
		auto v0 = qjs.GetIndexedJsValue(ctx, 0, arrLenTest);
		auto a0 = qjs.GetNamedJsValue(ctx, "a", v0);
		int ia0 = qjs.JsValueToInt(ctx, a0, 0);//ia0==123
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle res = qjs.CompileScript(ctx, "var a=123;var b=456;alert(a+b)", "");
		if (qjs.JsValueIsException(res))
		{
			ValueHandle exp = qjs.GetAndClearJsLastException(ctx);
			std::string strExp = qjs.JsValueToStdString(ctx, exp);
			printf(strExp.c_str());
		}
		size_t byteCodeLen = 0;
		uint8_t* byteCode = qjs.JsValueToByteCode(ctx, res, &byteCodeLen, false);
		if (byteCode)
		{
			//qjs.RunByteCode(ctx, byteCode, byteCodeLen);

			qjs.SaveByteCodeToFile(byteCode, byteCodeLen, "1.bin");
			qjs.FreeJsPointer(ctx, byteCode);

			byteCode = qjs.LoadFile(ctx, &byteCodeLen, "1.bin");
			if (byteCode)
			{
				qjs.RunByteCode(ctx, byteCode, byteCodeLen);
				qjs.FreeJsPointer(ctx, byteCode);
			}
			
		}
	}

	{
		QJS_SCOPE(ctx);

		//from qjsc-test.c
		const uint32_t qjsc_qjsc_test_size = 79;
		const uint8_t qjsc_qjsc_test[79] = {
		 0x02, 0x03, 0x0a, 0x61, 0x6c, 0x65, 0x72, 0x74,
		 0x2a, 0x49, 0x20, 0x61, 0x6d, 0x20, 0x63, 0x6f,
		 0x6d, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x72, 0x6f,
		 0x6d, 0x20, 0x62, 0x69, 0x6e, 0x2e, 0x18, 0x71,
		 0x6a, 0x73, 0x63, 0x2d, 0x74, 0x65, 0x73, 0x74,
		 0x2e, 0x6a, 0x73, 0x0e, 0x00, 0x06, 0x00, 0xa6,
		 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x0d,
		 0x01, 0xa8, 0x01, 0x00, 0x00, 0x00, 0x38, 0xe3,
		 0x00, 0x00, 0x00, 0x04, 0xe4, 0x00, 0x00, 0x00,
		 0xf0, 0xce, 0x28, 0xca, 0x03, 0x01, 0x00,
		};

		ValueHandle binRes = qjs.RunByteCode(ctx, qjsc_qjsc_test, qjsc_qjsc_test_size);
		if (qjs.JsValueIsException(binRes))
		{
			ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
			std::string s = qjs.JsValueToStdString(ctx, exception);
			printf(s.c_str());
		}
		else
		{
			std::string s = qjs.JsValueToStdString(ctx, binRes);
			printf(s.c_str());
		}
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle normalJsObj = qjs.RunScript(ctx, "var a=123;a", qjs.GetGlobalObject(ctx), "");
		size_t bytecodeLen = 0;
		uint8_t* bytecode = qjs.JsValueToByteCode(ctx, normalJsObj, &bytecodeLen, false);

		qjs.FreeJsPointer(ctx, bytecode);
	}

	{
		QJS_SCOPE(ctx);

		auto testThrowFunc = qjs.NewFunction(ctx, JsTestThrow, 0, NULL);
		qjs.SetNamedJsValue(ctx, "TestThrow", testThrowFunc, qjs.TheJsNull());
		qjs.RunScript(ctx, "try{ TestThrow(); } catch (e){ print(e) }", qjs.TheJsNull(), "");
	}

	{
		QJS_SCOPE(ctx);

		auto jNull = qjs.TheJsNull();
		auto jException = qjs.TheJsException();
		auto jFalse = qjs.TheJsFalse();
		auto jTrue = qjs.TheJsTrue();
		auto jUndefined = qjs.TheJsUndefined();
	}

	{
		ValueHandle throwMsg = qjs.NewStringJsValue(ctx, "test inline throw");
		ValueHandle jthrow = qjs.NewThrowJsValue(ctx, throwMsg);
		ValueHandle jExcep = qjs.GetAndClearJsLastException(ctx);
	}

	{
		QJS_SCOPE(ctx);

		ValueHandle testMsg = qjs.NewStringJsValue(ctx, "mensong");
		std::string s = qjs.JsValueToStdString(ctx, testMsg, "");
	}

	{
		//qjs.RunScriptFile(ctx, "test.js");
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void extendTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	auto jSample = qjs.NewObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "Sample", jSample, qjs.GetGlobalObject(ctx));
	int id1 = qjs.LoadExtend(ctx, "SampleExtend.dll", jSample, NULL);
	//qjs.UnloadExtend(ctx, id1);
	id1 = qjs.LoadExtend(ctx, "SampleExtend.dll", qjs.GetGlobalObject(ctx), NULL);
	//qjs.UnloadExtend(ctx, id1);

	//printObject(ctx, jSample);

	ValueHandle result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(ctx,
		L"Sample.testFoo();\ntestFoo();Sample.a='123';var sa=Sample.a;a='456';a"), qjs.TheJsNull(), "");
	if (!qjs.JsValueIsException(result))
	{
		const char* sz = qjs.JsValueToString(ctx, result);
		printf("运行成功:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	else
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void baseExtendTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	//ValueHandle jparent = qjs.NewObjectJsValue(ctx);
	//qjs.SetNamedJsValue(ctx, "parent", jparent, qjs.GetGlobalObject(ctx));
	ValueHandle jparent = qjs.GetGlobalObject(ctx);
	qjs.LoadExtend(ctx, "JsExtendBase.dll", jparent, NULL);
	JsExtendBase::Ins().AddPath(ctx, L"D:\\", jparent);

	//printObject(ctx, jparent);

	ValueHandle result = qjs.RunScriptFile(ctx, "baseTest.js", qjs.TheJsNull());
	if (qjs.JsValueIsException(result))
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}
	//else
	//{
	//	ValueHandle js = qjs.JsonStringify(ctx, result);
	//	std::string s = qjs.JsValueToStdString(ctx, js);
	//	printf("模块:%s\n", s.c_str());
	//}
		
	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void regExtendTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	int idBase = qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx), NULL);

	auto jReg = qjs.NewObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "Reg", jReg, qjs.GetGlobalObject(ctx));
	int idReg = qjs.LoadExtend(ctx, "JsExtendReg.dll", jReg, NULL);

	ValueHandle result = qjs.RunScriptFile(ctx, "regTest.js", qjs.TheJsNull());

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

bool WaitForExecutingJobsCallback(void* rawCurCtx, int resExecutePendingJob, void* user_data)
{
	return true;
}

void pendingJobTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx), NULL);

	ValueHandle result = qjs.RunScript(ctx, 
		"var t = setTimeout(function(p1,p2,p3){alert('setTimeout run:'+p1+p2+p3);},1000, 1,2,3);alert(debugObject(t));"
		"var n = 0;var i = setInterval(function(p1){alert('setInterval run:' + n + p1);++n;if (n > 3) clearInterval(i);},1000, ' param');alert(debugObject(i));"
		, 
		qjs.TheJsNull(), "");
	if (qjs.JsValueIsException(result))
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	qjs.WaitForExecutingJobs(rt, 10, WaitForExecutingJobsCallback, NULL);

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void fileExtendTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx), NULL);

	auto jfile = qjs.NewObjectJsValue(ctx);
	qjs.SetNamedJsValue(ctx, "File", jfile, qjs.GetGlobalObject(ctx));
	qjs.LoadExtend(ctx, "JsExtendFile.dll", jfile, NULL);

	auto result = qjs.RunScriptFile(ctx, "fileExtendTest.js", qjs.TheJsNull());
	if (qjs.JsValueIsException(result))
	{
		ValueHandle exception = qjs.GetAndClearJsLastException(ctx);
		const char* sz = qjs.JsValueToString(ctx, exception);
		printf("运行错误:%s\n", sz);
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void totalScopeTest()
{
	auto m_rt = qjs.NewRuntime();
	auto m_ctx = qjs.NewContext(m_rt);

	QJS_SCOPE(m_ctx);

	//加载base插件
	std::string baseExtend = "JsExtendBase.dll";
	qjs.LoadExtend(m_ctx, baseExtend.c_str(), qjs.GetGlobalObject(m_ctx), NULL);
	//加载Path插件
	std::string pathExtend = "JsExtendPath.dll";
	ValueHandle jpath = qjs.NewObjectJsValue(m_ctx);
	qjs.SetNamedJsValue(m_ctx, "Path", jpath, qjs.TheJsNull());
	qjs.LoadExtend(m_ctx, pathExtend.c_str(), jpath, NULL);

	ValueHandle jmsg = qjs.NewFunction(m_ctx, JsMsg, 0, NULL);
	qjs.SetNamedJsValue(m_ctx, "msg", jmsg, qjs.TheJsNull());

//#ifdef _DEBUG
	qjs.RunScript(m_ctx, "process.isDebug=true;", qjs.TheJsNull(), "");
//#endif // DEBUG

	qjs.FreeContext(m_ctx);
	qjs.FreeRuntime(m_rt);
}

void testArrayBuffer()
{
	auto rt = qjs.NewRuntime();
	auto ctx = qjs.NewContext(rt);

	//加载base插件
	std::string baseExtend = "JsExtendBase.dll";
	qjs.LoadExtend(ctx, baseExtend.c_str(), qjs.GetGlobalObject(ctx), NULL);
	
	uint8_t buf[10] = { 9,8,7,6,5,4,3,2,1,0 };
	auto jArrBuf = qjs.NewArrayBufferJsValue(ctx, buf, sizeof(buf)/sizeof(uint8_t));

	qjs.FillArrayBuffer(ctx, jArrBuf, 0xf);

	//更改ArrayBuffer
	size_t bufLen = 0;
	uint8_t * buf2 = qjs.GetArrayBufferPtr(ctx, jArrBuf, &bufLen);
	for (size_t i = 0; i < bufLen - 1; i++)
		buf2[i] = 1;
	buf2[bufLen - 1] = 0;

	//重新读出
	buf2 = qjs.GetArrayBufferPtr(ctx, jArrBuf, &bufLen);


	qjs.DetachArrayBufferJsValue(ctx, &jArrBuf);

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

int main()
{
	//baseTest();
	//extendTest();
	//myTest();
	//baseExtendTest();
	//regExtendTest();
	//fileExtendTest();
	//pendingJobTest();

	//totalScopeTest();

	testArrayBuffer();

	return 0;
}
