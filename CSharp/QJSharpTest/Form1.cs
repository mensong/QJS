using QJSharp;
using System;
using System.IO;
using System.Windows.Forms;

namespace QJSharpTest
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        string testFunctionParam = "����";
        string strVal = "�ַ���ֵ";

        private void button1_Click(object sender, EventArgs e)
        {
            using (QJS qjs = new QJS())
            {
                int extId = qjs.LoadBaseExtend();
                               
                {
                    qjs.SetFreeingContextCallback(OnFreeingContext);
                }

                {
                    qjs.SetRuntimeUserData(1, strVal);
                    string userData2 = (string)qjs.GetRuntimeUserData(1);
                    if (strVal == userData2)
                        listBox1.Items.Add("SetRuntimeUserData/GetRuntimeUserData ����ͨ��");
                    else
                        listBox1.Items.Add("SetRuntimeUserData/GetRuntimeUserData ���Բ�ͨ��");
                }

                {
                    qjs.SetContextUserData(1, strVal);
                    string userData2 = (string)qjs.GetContextUserData(1);
                    if (strVal == userData2)
                        listBox1.Items.Add("SetContextUserData/GetContextUserData ����ͨ��");
                    else
                        listBox1.Items.Add("SetContextUserData/GetContextUserData ���Բ�ͨ��");
                }

                {
                    byte[] loadedData = qjs.LoadFile("QJS.dll");
                    if (loadedData != null)
                        listBox1.Items.Add("LoadFile ����ͨ��");
                    else
                        listBox1.Items.Add("LoadFile ���Բ�ͨ��");
                }

                {
                    ValueHandle res = qjs.RunScript("'" + strVal + "'", QJS.TheJsUndefined(), "");
                    string script2 = qjs.JsValueToString(res);
                    if (strVal == script2)
                        listBox1.Items.Add("RunScript ����ͨ��");
                    else
                        listBox1.Items.Add("RunScript ���Բ�ͨ��");
                }

                {
                    using (StreamWriter textWriter = new StreamWriter("1.js"))
                    {
                        textWriter.Write("'" + strVal + "'");
                        textWriter.Close();
                    }

                    ValueHandle res = qjs.RunScriptFile("1.js", QJS.TheJsUndefined());
                    string script3 = qjs.JsValueToString(res);
                    if (strVal == script3)
                        listBox1.Items.Add("RunScriptFile ����ͨ��");
                    else
                        listBox1.Items.Add("RunScriptFile ���Բ�ͨ��");
                }

                {
                    ValueHandle jFunc = qjs.NewFunction(JsFunction, 1, "123");
                    ValueHandle[] args = new ValueHandle[1];
                    args[0] = qjs.NewStringJsValue(testFunctionParam);
                    qjs.CallJsFunction(jFunc, args, qjs.GetGlobalObject());
                }

                {
                    ValueHandle jarr = qjs.NewArrayJsValue();
                    qjs.SetIndexedJsValue(0, qjs.NewStringJsValue("1"), jarr);
                    qjs.SetIndexedJsValue(1, qjs.NewStringJsValue("2"), jarr);
                    qjs.SetIndexedJsValue(2, qjs.NewStringJsValue("3"), jarr);
                    long size = qjs.GetLength(jarr);
                    bool bSuccess = true;
                    for (int i = 0; i < size; i++)
                    {
                        ValueHandle jstr = qjs.GetIndexedJsValue((uint)i, jarr);
                        string s = qjs.JsValueToString(jstr);
                        if ((i+1).ToString() != s)
                        {
                            bSuccess = false;
                            break;
                        }
                    }
                    if (bSuccess)
                        listBox1.Items.Add("NewArrayJsValue/SetIndexedJsValue/GetIndexedJsValue ����ͨ��");
                    else
                        listBox1.Items.Add("NewArrayJsValue/SetIndexedJsValue/GetIndexedJsValue ���Բ�ͨ��");
                }

                {
                    byte[] bytes = { 0, 1, 2, 3 };
                    ValueHandle jBuff = qjs.NewArrayBufferJsValueCopy(bytes);
                    byte[] bytes1 = qjs.GetArrayBuffer(jBuff);

                    if (bytes.Length == bytes1.Length)
                    {
                        bool bSuccess = true;
                        for (int i = 0; i < bytes.Length; i++)
                        {
                            if (bytes[i] != bytes1[i])
                            {
                                bSuccess = false;
                                break;
                            }
                        }
                        if (bSuccess)
                            listBox1.Items.Add("NewArrayBufferJsValueCopy/GetArrayBuffer ����ͨ��");
                        else
                            listBox1.Items.Add("NewArrayBufferJsValueCopy/GetArrayBuffer ���Բ�ͨ��");
                    }
                    else
                        listBox1.Items.Add("NewArrayBufferJsValueCopy/GetArrayBuffer ���Բ�ͨ��");
                }

                {
                    string sJson = "{\"a\":123}";
                    ValueHandle jObj = qjs.JsonParse(sJson);
                    ValueHandle jstr = qjs.JsonStringify(jObj);
                    string sJson2 = qjs.JsValueToString(jstr);
                    if (sJson == sJson2)
                        listBox1.Items.Add("JsonParse/JsonStringify ����ͨ��");
                    else
                        listBox1.Items.Add("JsonParse/JsonStringify ���Բ�ͨ��");
                }

                {
                    ValueHandle jobj = qjs.NewObjectJsValue();
                    if (qjs.SetNamedJsValue("������", qjs.NewStringJsValue(strVal), jobj))
                    {
                        ValueHandle jv = qjs.GetNamedJsValue("������", jobj);
                        string s = qjs.JsValueToString(jv);
                        if (s == strVal)
                            listBox1.Items.Add("SetNamedJsValue/GetNamedJsValue ����ͨ��");
                        else
                            listBox1.Items.Add("SetNamedJsValue/GetNamedJsValue ���Բ�ͨ��");
                    }
                    else
                        listBox1.Items.Add("SetNamedJsValue ���Բ�ͨ��");
                }

                {
                    qjs.LoadExtend("JsExtendBase.dll", qjs.GetGlobalObject(), IntPtr.Zero);
                    ValueHandle jAlert = qjs.GetNamedJsValue("alert", qjs.GetGlobalObject());
                    if (QJS.JsValueIsFunction(jAlert))
                        listBox1.Items.Add("LoadExtend ����ͨ��");
                    else
                        listBox1.Items.Add("LoadExtend ���Բ�ͨ��");
                }

                {
                    ValueHandle jobj = qjs.NewObjectJsValue();
                    ValueHandle jfuncGetter = qjs.NewFunction(JGetter, 0, null);
                    ValueHandle jfuncSetter = qjs.NewFunction(JSetter, 1, null);
                    bool b = qjs.DefineGetterSetter(jobj, "gs", jfuncGetter, jfuncSetter);
                    qjs.SetNamedJsValue("obj", jobj, qjs.GetGlobalObject());
                    ValueHandle jres = qjs.RunScript("obj.gs='mensong';var a = obj.gs;a", qjs.GetGlobalObject(), "");
                    string s = qjs.JsValueToString(jres);
                    if (s == "mensong")
                        listBox1.Items.Add("DefineGetterSetter ����ͨ��");
                    else
                        listBox1.Items.Add("DefineGetterSetter ���Բ�ͨ��");                  
                }

            }
        }

        string gsval = "getter setter value";
        ValueHandle JGetter(QJS qjs, ValueHandle this_val, ValueHandle[] args, Object user_data)
        {
            return qjs.NewStringJsValue(gsval);
        }

        ValueHandle JSetter(QJS qjs, ValueHandle this_val, ValueHandle[] args, Object user_data)
        {
            if (args.Length > 0)
            {
                gsval = qjs.JsValueToString(args[0]);
            }

            return QJS.TheJsUndefined();
        }

        void OnFreeingContext(UIntPtr ctx)
        {
            listBox1.Items.Add("SetFreeingContextCallback ����ͨ��");
        }

        ValueHandle JsFunction(QJS qjs, ValueHandle this_val, ValueHandle[] args, Object user_data)
        {
            if (args.Length == 1)
            {
                string argStr = qjs.JsValueToString(args[0]);
                if (argStr == testFunctionParam)
                    listBox1.Items.Add("NewFunction/CallJsFunction ����ͨ��");
                else
                    listBox1.Items.Add("NewFunction/CallJsFunction ���Բ�ͨ��");
            }
            else
            {
                listBox1.Items.Add("NewFunction/CallJsFunction ���Բ�ͨ��");
            }

            return QJS.TheJsUndefined();
        }
    }
}
