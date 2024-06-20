// Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include "..\QJS\QJS.h"
#include <sstream>

void baseTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	ValueHandle result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(L"var a=123;a"), qjs.TheJsNull(), "");
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

ValueHandle JsAlert(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::string msg;
	if (argc > 0)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[0]);
		if (sz)
			msg = sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	std::string title;
	if (argc < 2 || qjs.JsValueIsUndefined(argv[1]))
		title = "QJS";
	else
	{
		const char* sz = qjs.JsValueToString(ctx, argv[1]);
		if (sz)
			title = sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	MessageBoxA(NULL, msg.c_str(), title.c_str(), 0);

	auto item = qjs.NewStringJsValue(ctx, msg.c_str());
	return item;
}

ValueHandle JsPrint(ContextHandle ctx, ValueHandle this_val, int argc, ValueHandle* argv, void* user_data)
{
	std::stringstream ss;
	for (int i = 0; i < argc; i++)
	{
		const char* sz = qjs.JsValueToString(ctx, argv[i]);
		if (sz)
			ss << sz;
		qjs.FreeJsValueToStringBuffer(ctx, sz);
	}

	wprintf(L"%s", qjs.Utf8ToUnicode(ss.str().c_str()));

	return qjs.NewStringJsValue(ctx, ss.str().c_str());
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

	ValueHandle alertFunc = qjs.NewFunction(ctx, JsAlert, 2, NULL);
	 qjs.SetNamedJsValue(ctx, "alert", alertFunc, qjs.TheJsNull());

	ValueHandle printFunc = qjs.NewFunction(ctx, JsPrint, -1, NULL);
	qjs.SetNamedJsValue(ctx, "print", printFunc, qjs.TheJsNull());

	{
		ValueHandle arrNames = qjs.GetObjectPropertyKeys(ctx, qjs.GetGlobalObject(ctx), true, false);
		int64_t len = qjs.GetLength(ctx, arrNames);
		for (int i = 0; i < len; i++)
		{
			auto jname = qjs.GetIndexedJsValue(ctx, i, arrNames);
			std::string name = qjs.JsValueToStdString(ctx, jname);
			printf("Property name:%s\n", name.c_str());
		}
	}

	{
		auto argv = qjs.NewStringJsValue(ctx, "mensong,");
		ValueHandle argvs[] = { argv, argv, argv };
		auto ret1 = qjs.CallJsFunction(ctx, printFunc, argvs, 3, qjs.TheJsNull());
	}

	{
		ValueHandle o = qjs.NewIntJsValue(ctx, 123);
		qjs.SetObjectUserData(o, (void*)123);
		void* pu = qjs.GetObjectUserData(o);
		int iu = (int)pu;
	}

	{
		ValueHandle bv = qjs.NewBoolJsValue(ctx, true);
		qjs.SetNamedJsValue(ctx, "bv", bv, qjs.TheJsNull());

		ValueHandle o = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "bv", bv, o);
		qjs.SetNamedJsValue(ctx, "o", o, qjs.TheJsNull());

		ValueHandle arrNames = qjs.GetObjectPropertyKeys(ctx, o, true, false);
		int64_t len = qjs.GetLength(ctx, arrNames);
		for (int i = 0; i < len; i++)
		{
			auto jname = qjs.GetIndexedJsValue(ctx, i, arrNames);
			std::string name = qjs.JsValueToStdString(ctx, jname);
			printf("Property name:%s\n", name.c_str());
		}
	}

	{
		ValueHandle v = qjs.TheJsException(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsFalse(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsNull(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsTrue(); qjs.FreeValueHandle(&v);
		v = qjs.TheJsUndefined(); qjs.FreeValueHandle(&v);
		v = qjs.NewArrayJsValue(ctx); qjs.FreeValueHandle(&v);
		v = qjs.NewBoolJsValue(ctx, false); qjs.FreeValueHandle(&v);
		v = qjs.NewDateJsValue(ctx, 1718849344); qjs.FreeValueHandle(&v);
		v = qjs.NewDoubleJsValue(ctx, 123.0); qjs.FreeValueHandle(&v);
		v = qjs.NewFunction(ctx, JsPrint, 0, NULL); qjs.FreeValueHandle(&v);
		v = qjs.NewInt64JsValue(ctx, 123); qjs.FreeValueHandle(&v);
		v = qjs.NewIntJsValue(ctx, 123); qjs.FreeValueHandle(&v);
		v = qjs.NewObjectJsValue(ctx); qjs.FreeValueHandle(&v);
		v = qjs.NewStringJsValue(ctx, "abc"); qjs.FreeValueHandle(&v);
		v = qjs.NewThrowJsValue(ctx, qjs.NewStringJsValue(ctx, "abc")); qjs.FreeValueHandle(&v);
	}

	{
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

		bool b = qjs.JsValueIsArray(ctx, arr);
		auto jToString = qjs.GetNamedJsValue(ctx, "toString", arr);
		b = qjs.JsValueIsFunction(ctx, jToString);
		if (b)
		{
			auto jstrToString = qjs.CallJsFunction(ctx, jToString, NULL, 0, arr);
			auto strToString = qjs.JsValueToString(ctx, jstrToString);
			qjs.FreeJsValueToStringBuffer(ctx, strToString);
		}

		ValueHandle jlen = qjs.GetNamedJsValue(ctx, "length", arr);
		arrLen = qjs.JsValueToInt(ctx, jlen, 0);

		auto jpush = qjs.GetNamedJsValue(ctx, "push", arr);
		b = qjs.JsValueIsFunction(ctx, jpush);
		if (b)
		{
			ValueHandle argv[] = {qjs.NewStringJsValue(ctx, "aaa")};
			qjs.CallJsFunction(ctx, jpush, argv, sizeof(argv) / sizeof(ValueHandle), arr);

			jlen = qjs.GetNamedJsValue(ctx, "length", arr);
			arrLen = qjs.JsValueToInt(ctx, jlen, 0);
			arrLen = 0;
		}
	}

	{
		auto jint = qjs.NewIntJsValue(ctx, 65536);
		auto intstr1 = qjs.JsValueToString(ctx, jint);
		qjs.FreeJsValueToStringBuffer(ctx, intstr1);
	}

	{
		ValueHandle test_getter_setter = qjs.NewObjectJsValue(ctx);
		qjs.SetNamedJsValue(ctx, "test_getter_setter", test_getter_setter, qjs.GetGlobalObject(ctx));
		auto getter = qjs.NewFunction(ctx, JsGetter, 0, NULL);
		auto setter = qjs.NewFunction(ctx, JsSetter, 1, NULL);
		qjs.DefineGetterSetter(ctx, test_getter_setter, "gs", getter, setter);
		qjs.RunScript(ctx, "test_getter_setter.gs='123';test_getter_setter.gs", qjs.GetGlobalObject(ctx), "");

		ValueHandle arrNames = qjs.GetObjectPropertyKeys(ctx, test_getter_setter, false, false);
		int64_t len = qjs.GetLength(ctx, arrNames);
		for (int i = 0; i < len; i++)
		{
			auto jname = qjs.GetIndexedJsValue(ctx, i, arrNames);
			std::string name = qjs.JsValueToStdString(ctx, jname);
			printf("Property name:%s\n", name.c_str());
		}
	}

	{
		auto g1 = qjs.GetGlobalObject(ctx);
		auto g2 = qjs.GetGlobalObject(ctx);
		auto g3 = qjs.GetGlobalObject(ctx);
		auto g4 = qjs.GetGlobalObject(ctx);
	}

	{
		auto date = qjs.NewDateJsValue(ctx, 1679044555000);
		uint64_t ts = qjs.JsValueToTimestamp(ctx, date);
		qjs.SetNamedJsValue(ctx, "mydate", date, qjs.TheJsNull());
	}

	{
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
		auto arrLenTest = qjs.RunScript(ctx, "[{\"a\":123}, {\"a\":456}]", qjs.TheJsNull(), "");
		auto arrTestLen = qjs.GetLength(ctx, arrLenTest);
		auto v0 = qjs.GetIndexedJsValue(ctx, 0, arrLenTest);
		auto a0 = qjs.GetNamedJsValue(ctx, "a", v0);
		int ia0 = qjs.JsValueToInt(ctx, a0, 0);//ia0==123
	}

	{
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
		ValueHandle normalJsObj = qjs.RunScript(ctx, "var a=123;a", qjs.GetGlobalObject(ctx), "");
		size_t bytecodeLen = 0;
		uint8_t* bytecode = qjs.JsValueToByteCode(ctx, normalJsObj, &bytecodeLen, false);

		qjs.FreeJsPointer(ctx, bytecode);
	}

	{
		auto testThrowFunc = qjs.NewFunction(ctx, JsTestThrow, 0, NULL);
		qjs.SetNamedJsValue(ctx, "TestThrow", testThrowFunc, qjs.TheJsNull());
		qjs.RunScript(ctx, "try{ TestThrow(); } catch (e){ print(e) }", qjs.TheJsNull(), "");
	}

	{
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
		ValueHandle testMsg = qjs.NewStringJsValue(ctx, "mensong");
		std::string s = qjs.JsValueToStdString(ctx, testMsg, "");
	}

	{
		qjs.RunScriptFile(ctx, "test.js");
	}

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

void extendTest()
{
	RuntimeHandle rt = qjs.NewRuntime();
	ContextHandle ctx = qjs.NewContext(rt);

	ValueHandle ext1 = qjs.LoadExtend(ctx, "SampleExtend.dll", qjs.TheJsNull());
	qjs.SetNamedJsValue(ctx, "Sample", ext1, qjs.TheJsNull());
	//qjs.UnloadExtend(ctx, "SampleExtend.dll");

	ValueHandle result = qjs.RunScript(ctx, qjs.UnicodeToUtf8(L"Sample.testFoo1();"), qjs.TheJsNull(), "");
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

	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx));

	ValueHandle result = qjs.RunScriptFile(ctx, "baseTest.js");
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

	qjs.LoadExtend(ctx, "JsExtendBase.dll", qjs.GetGlobalObject(ctx));

	ValueHandle ext1 = qjs.LoadExtend(ctx, "JsExtendReg.dll", qjs.TheJsNull());
	qjs.SetNamedJsValue(ctx, "Reg", ext1, qjs.TheJsNull());

	ValueHandle result = qjs.RunScriptFile(ctx, "regTest.js");

	qjs.FreeContext(ctx);
	qjs.FreeRuntime(rt);
}

int main()
{
	//baseTest();
	//extendTest();
	//myTest();
	baseExtendTest();
	//regExtendTest();
	
	return 0;
}
