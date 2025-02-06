using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace QJSharp
{
    public class QJS : IDisposable
    {
        #region QJS.dll import
        public enum ValueType
        {
            JS_TYPE_UNINITIALIZED = 0,
            JS_TYPE_INT = 1,
            JS_TYPE_BOOL = 2,
            JS_TYPE_NULL = 3,
            JS_TYPE_UNDEFINED = 4,
            JS_TYPE_CATCH_OFFSET = 5,
            JS_TYPE_EXCEPTION = 6,
            JS_TYPE_FLOAT64 = 7,

            /* all tags with a reference count have 0b1000 bit */
            JS_TYPE_OBJECT = 8,
            JS_TYPE_FUNCTION_BYTECODE = 9, /* used internally */
            JS_TYPE_MODULE = 10, /* used internally */
            JS_TYPE_STRING = 11,
            JS_TYPE_SYMBOL = 12,
            JS_TYPE_BIG_FLOAT = 13,
            JS_TYPE_BIG_INT = 14,
            JS_TYPE_BIG_DECIMAL = 15,
        };

        [DllImport("QJS.dll", EntryPoint = "NewRuntime")]
        private static extern UIntPtr NewRuntime();

        [DllImport("QJS.dll", EntryPoint = "FreeRuntime")]
        private static extern void FreeRuntime(UIntPtr runtime);

        [DllImport("QJS.dll", EntryPoint = "SetRuntimeUserData")]
        private static extern void SetRuntimeUserData(UIntPtr runtime, int key, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "GetRuntimeUserData")]
        private static extern UIntPtr GetRuntimeUserData(UIntPtr runtime, int key);

        [DllImport("QJS.dll", EntryPoint = "NewContext")]
        private static extern UIntPtr NewContext(UIntPtr runtime);

        [DllImport("QJS.dll", EntryPoint = "FreeContext")]
        private static extern void FreeContext(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "ResetContext")]
        private static extern void ResetContext(UIntPtr ctx);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnFreeingContextCallback(UIntPtr ctx);
        [DllImport("QJS.dll", EntryPoint = "SetFreeingContextCallback")]
        private static extern void SetFreeingContextCallback(UIntPtr ctx, OnFreeingContextCallback cb);

        [DllImport("QJS.dll", EntryPoint = "RemoveFreeingContextCallback")]
        private static extern bool RemoveFreeingContextCallback(UIntPtr ctx, OnFreeingContextCallback cb);

        [DllImport("QJS.dll", EntryPoint = "GetContextRuntime")]
        private static extern UIntPtr GetContextRuntime(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "SetContextUserData")]
        private static extern void SetContextUserData(UIntPtr ctx, int key, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "GetContextUserData")]
        private static extern UIntPtr GetContextUserData(UIntPtr ctx, int key);

        [DllImport("QJS.dll", EntryPoint = "GetGlobalObject")]
        private static extern ValueHandle GetGlobalObject(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "FreeJsPointer")]
        private static extern void FreeJsPointer(UIntPtr ctx, IntPtr ptr);

        [DllImport("QJS.dll", EntryPoint = "LoadFile")]
        private static extern IntPtr LoadFile(UIntPtr ctx, out UInt64 outLen, string filename);        

        [DllImport("QJS.dll", EntryPoint = "RunScript")]
        private static extern ValueHandle RunScript(UIntPtr ctx, byte[] script, ValueHandle parent, byte[] filename);

        [DllImport("QJS.dll", EntryPoint = "RunScriptFile")]
        private static extern ValueHandle RunScriptFile(UIntPtr ctx, string filename, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "CallJsFunction")]
        private static extern ValueHandle CallJsFunction(UIntPtr ctx, ValueHandle jsFunction, IntPtr argv, int argc, ValueHandle parent);
        
        [DllImport("QJS.dll", EntryPoint = "GetNamedJsValue")]
        private static extern ValueHandle GetNamedJsValue(UIntPtr ctx, byte[] varName, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "SetNamedJsValue")]
        private static extern bool SetNamedJsValue(UIntPtr ctx, byte[] varName, ValueHandle varValue, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "DeleteNamedJsValue")]
        private static extern bool DeleteNamedJsValue(UIntPtr ctx, byte[] varName, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "HasNamedJsValue")]
        private static extern bool HasNamedJsValue(UIntPtr ctx, byte[] varName, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "GetObjectPropertyKeys")]
        private static extern ValueHandle GetObjectPropertyKeys(UIntPtr ctx, ValueHandle jObj, 
            bool onlyEnumerable/*=true*/, bool enableSymbol/*=false*/);

        [DllImport("QJS.dll", EntryPoint = "GetFunctionName")]
        private static extern ValueHandle GetFunctionName(UIntPtr ctx, ValueHandle func);

        [DllImport("QJS.dll", EntryPoint = "GetCurFrameFunction")]
        private static extern ValueHandle GetCurFrameFunction(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "GetIndexedJsValue")]
        private static extern ValueHandle GetIndexedJsValue(UIntPtr ctx, UInt32 idx, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "SetIndexedJsValue")]
        private static extern bool SetIndexedJsValue(UIntPtr ctx, UInt32 idx, ValueHandle varValue, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "DeleteIndexedJsValue")]
        private static extern bool DeleteIndexedJsValue(UIntPtr ctx, UInt32 idx, ValueHandle parent);

        [DllImport("QJS.dll", EntryPoint = "GetLength")]
        private static extern Int64 GetLength(UIntPtr ctx, ValueHandle obj);

        [DllImport("QJS.dll", EntryPoint = "GetPrototype")]
        private static extern ValueHandle GetPrototype(UIntPtr ctx, ValueHandle jObj);

        [DllImport("QJS.dll", EntryPoint = "SetPrototype")]
        private static extern bool SetPrototype(UIntPtr ctx, ValueHandle jObj, ValueHandle protoJVal);

        [DllImport("QJS.dll", EntryPoint = "TheJsUndefined")]
        public static extern ValueHandle TheJsUndefined();

        [DllImport("QJS.dll", EntryPoint = "TheJsNull")]
        public static extern ValueHandle TheJsNull();

        [DllImport("QJS.dll", EntryPoint = "TheJsTrue")]
        public static extern ValueHandle TheJsTrue();

        [DllImport("QJS.dll", EntryPoint = "TheJsFalse")]
        public static extern ValueHandle TheJsFalse();

        [DllImport("QJS.dll", EntryPoint = "DefineGetterSetter")]
        private static extern bool DefineGetterSetter(UIntPtr ctx, ValueHandle parent,
            byte[] propName, ValueHandle getter, ValueHandle setter);

        [DllImport("QJS.dll", EntryPoint = "NewIntJsValue")]
        private static extern ValueHandle NewIntJsValue(UIntPtr ctx, int intValue);

        [DllImport("QJS.dll", EntryPoint = "NewInt64JsValue")]
        private static extern ValueHandle NewInt64JsValue(UIntPtr ctx, Int64 intValue);

        [DllImport("QJS.dll", EntryPoint = "NewUInt64JsValue")]
        private static extern ValueHandle NewUInt64JsValue(UIntPtr ctx, UInt64 intValue);

        [DllImport("QJS.dll", EntryPoint = "NewDoubleJsValue")]
        private static extern ValueHandle NewDoubleJsValue(UIntPtr ctx, double doubleValue);

        [DllImport("QJS.dll", EntryPoint = "NewStringJsValue")]
        private static extern ValueHandle NewStringJsValue(UIntPtr ctx, byte[] stringValue);

        [DllImport("QJS.dll", EntryPoint = "NewBoolJsValue")]
        private static extern ValueHandle NewBoolJsValue(UIntPtr ctx, bool boolValue);

        [DllImport("QJS.dll", EntryPoint = "NewObjectJsValue")]
        private static extern ValueHandle NewObjectJsValue(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "SetObjectUserData")]
        private static extern bool SetObjectUserData(ValueHandle value, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "GetObjectUserData")]
        private static extern IntPtr GetObjectUserData(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "NewArrayJsValue")]
        private static extern ValueHandle NewArrayJsValue(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "NewThrowJsValue")]
        private static extern ValueHandle NewThrowJsValue(UIntPtr ctx, ValueHandle throwWhat);

        [DllImport("QJS.dll", EntryPoint = "NewDateJsValue")]
        private static extern ValueHandle NewDateJsValue(UIntPtr ctx, UInt64 ms_since_1970);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BufferOnceFree(UIntPtr ctx, UIntPtr buf);
        [DllImport("QJS.dll", EntryPoint = "NewArrayBufferJsValue")]
        private static extern ValueHandle NewArrayBufferJsValue(UIntPtr ctx, IntPtr buf, int bufLen, BufferOnceFree freeFunc/* = NULL*/);

        [DllImport("QJS.dll", EntryPoint = "NewArrayBufferJsValueCopy")]
        private static extern ValueHandle NewArrayBufferJsValueCopy(UIntPtr ctx, IntPtr buf, int bufLen);

        [DllImport("QJS.dll", EntryPoint = "DetachArrayBufferJsValue")]
        private static extern void DetachArrayBufferJsValue(UIntPtr ctx, ref ValueHandle arrBuf);

        [DllImport("QJS.dll", EntryPoint = "GetArrayBufferPtr")]
        private static extern IntPtr GetArrayBufferPtr(UIntPtr ctx, ValueHandle arrBuf, ref int outBufLen);

        [DllImport("QJS.dll", EntryPoint = "GetArrayBufferByTypedArrayBuffer")]
        private static extern ValueHandle GetArrayBufferByTypedArrayBuffer(UIntPtr ctx, ValueHandle typedArrBuf,
                                                                            ref int out_byte_offset,
                                                                            ref int out_byte_length,
                                                                            ref int out_bytes_per_element);

        [DllImport("QJS.dll", EntryPoint = "FillArrayBuffer")]
        private static extern bool FillArrayBuffer(UIntPtr ctx, ValueHandle arrBuf, byte fill);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate ValueHandle _JsFunctionCallback(UIntPtr ctx, ValueHandle this_val, int argc, IntPtr argv, IntPtr user_data);
        [DllImport("QJS.dll", EntryPoint = "NewFunction")]
        private static extern ValueHandle NewFunction(UIntPtr ctx, _JsFunctionCallback cb, int argc, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "JsValueToString")]
        private static extern IntPtr JsValueToString(UIntPtr ctx, ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueToInt")]
        private static extern int JsValueToInt(UIntPtr ctx, ValueHandle value, int defVal/* = 0*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToInt64")]
        private static extern Int64 JsValueToInt64(UIntPtr ctx, ValueHandle value, Int64 defVal/* = 0*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToUInt64")]
        private static extern UInt64 JsValueToUInt64(UIntPtr ctx, ValueHandle value, UInt64 defVal/* = 0*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToDouble")]
        private static extern double JsValueToDouble(UIntPtr ctx, ValueHandle value, double defVal/* = 0.0*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToBool")]
        private static extern bool JsValueToBool(UIntPtr ctx, ValueHandle value, bool defVal/* = false*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToTimestamp")]
        private static extern UInt64 JsValueToTimestamp(UIntPtr ctx, ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "CompileScript")]
        private static extern ValueHandle CompileScript(UIntPtr ctx, byte[] script, byte[] filename/* = ""*/);

        [DllImport("QJS.dll", EntryPoint = "JsValueToByteCode")]
        private static extern IntPtr JsValueToByteCode(UIntPtr ctx, ValueHandle value, ref int outByteCodeLen, bool byte_swap/* = false*/);

        [DllImport("QJS.dll", EntryPoint = "ByteCodeToJsValue")]
        private static extern ValueHandle ByteCodeToJsValue(UIntPtr ctx, IntPtr byteCode, int byteCodeLen);

        [DllImport("QJS.dll", EntryPoint = "SaveByteCodeToFile")]
        public static extern bool SaveByteCodeToFile(IntPtr byteCode, int byteCodeLen, string filepath);

        [DllImport("QJS.dll", EntryPoint = "RunByteCode")]
        private static extern ValueHandle RunByteCode(UIntPtr ctx, IntPtr byteCode, int byteCodeLen);

        [DllImport("QJS.dll", EntryPoint = "GetValueType")]
        public static extern ValueType GetValueType(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsString")]
        public static extern bool JsValueIsString(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsInt")]
        public static extern bool JsValueIsInt(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsNumber")]
        public static extern bool JsValueIsNumber(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsDouble")]
        public static extern bool JsValueIsDouble(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsBool")]
        public static extern bool JsValueIsBool(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsObject")]
        public static extern bool JsValueIsObject(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsArray")]
        public static extern bool JsValueIsArray(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsFunction")]
        public static extern bool JsValueIsFunction(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsException")]
        public static extern bool JsValueIsException(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsUndefined")]
        public static extern bool JsValueIsUndefined(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsNull")]
        public static extern bool JsValueIsNull(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsDate")]
        public static extern bool JsValueIsDate(ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsValueIsGlobalObject")]
        private static extern bool JsValueIsGlobalObject(UIntPtr ctx, ValueHandle value);

        [DllImport("QJS.dll", EntryPoint = "JsonStringify")]
        private static extern ValueHandle JsonStringify(UIntPtr ctx, ValueHandle value,
            ValueHandle replacer/*=TheJsUndefined()*/, ValueHandle space/*=TheJsUndefined()*/);

        [DllImport("QJS.dll", EntryPoint = "JsonParse")]
        private static extern ValueHandle JsonParse(UIntPtr ctx, byte[] json);

        [DllImport("QJS.dll", EntryPoint = "GetAndClearJsLastException")]
        private static extern ValueHandle GetAndClearJsLastException(UIntPtr ctx);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate ValueHandle JsJobCallback(UIntPtr ctx, int argc, ref ValueHandle argv);
        [DllImport("QJS.dll", EntryPoint = "EnqueueJob")]
        private static extern bool EnqueueJob(UIntPtr ctx, JsJobCallback funcJob, ref ValueHandle args, int argc);

        [DllImport("QJS.dll", EntryPoint = "ExecutePendingJob")]
        private static extern int ExecutePendingJob(UIntPtr runtime, ref IntPtr outRawCtx);

        [DllImport("QJS.dll", EntryPoint = "GetContextByRaw")]
        private static extern UIntPtr GetContextByRaw(UIntPtr runtime, IntPtr rawCtx);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bool _WaitForExecutingJobsCallback(UIntPtr rawCurCtx, int resExecutePendingJob, IntPtr user_data);
        [DllImport("QJS.dll", EntryPoint = "WaitForExecutingJobs")]
        private static extern int WaitForExecutingJobs(UIntPtr runtime, 
            Int32 loopIntervalMS, _WaitForExecutingJobsCallback cb, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "SetDebuggerMode")]
        private static extern void SetDebuggerMode(UIntPtr ctx, bool onoff);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerMode")]
        private static extern bool GetDebuggerMode(UIntPtr ctx);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void DebuggerLineCallback(UIntPtr ctx, UInt32 line_no, UIntPtr pc, IntPtr user_data);
        [DllImport("QJS.dll", EntryPoint = "SetDebuggerLineCallback")]
        private static extern void SetDebuggerLineCallback(UIntPtr ctx, DebuggerLineCallback cb, IntPtr user_data);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerLineCallback")]
        private static extern bool GetDebuggerLineCallback(UIntPtr ctx, ref DebuggerLineCallback out_cb, ref IntPtr out_user_data);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerStackDepth")]
        private static extern UInt32 GetDebuggerStackDepth(UIntPtr ctx);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerBacktrace")]
        private static extern ValueHandle GetDebuggerBacktrace(UIntPtr ctx, UIntPtr pc);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerClosureVariables")]
        private static extern ValueHandle GetDebuggerClosureVariables(UIntPtr ctx, int stack_idx);

        [DllImport("QJS.dll", EntryPoint = "GetDebuggerLocalVariables")]
        private static extern ValueHandle GetDebuggerLocalVariables(UIntPtr ctx, int stack_idx);

        [DllImport("QJS.dll", EntryPoint = "LoadExtend")]
        private static extern int LoadExtend(UIntPtr ctx, string extendFile, ValueHandle parent, IntPtr userData/* = NULL*/);

        [DllImport("QJS.dll", EntryPoint = "GetExtendList")]
        private static extern IntPtr GetExtendList(UIntPtr ctx, ref int outLen);

        [DllImport("QJS.dll", EntryPoint = "GetExtendHandle")]
        private static extern UIntPtr GetExtendHandle(UIntPtr ctx, int extendId);

        [DllImport("QJS.dll", EntryPoint = "GetExtendFile")]
        private static extern IntPtr GetExtendFile(UIntPtr ctx, int extendId);

        [DllImport("QJS.dll", EntryPoint = "GetExtendParentObject")]
        private static extern ValueHandle GetExtendParentObject(UIntPtr ctx, int extendId);

        [DllImport("QJS.dll", EntryPoint = "UnloadExtend")]
        private static extern void UnloadExtend(UIntPtr ctx, int extendId);

        [DllImport("QJS.dll", EntryPoint = "FreeValueHandle")]
        public static extern void FreeValueHandle(ref ValueHandle value);
        #endregion

        UIntPtr m_runtime;
        UIntPtr m_ctx;

        private static IntPtr StructArrayConvertToIntPtr(ValueHandle[] valArr)
        {
            // 2. 计算结构体大小
            int structSize = Marshal.SizeOf(typeof(ValueHandle));

            // 3. 分配非托管内存
            IntPtr ptr = Marshal.AllocHGlobal(structSize * valArr.Length);

            try
            {
                // 4. 逐个复制结构体到非托管内存
                for (int i = 0; i < valArr.Length; i++)
                {
                    IntPtr current = new IntPtr(ptr.ToInt64() + (i * structSize));
                    Marshal.StructureToPtr(valArr[i], current, false);
                }

                return ptr;
            }
            catch
            {
                // 发生异常时释放内存
                Marshal.FreeHGlobal(ptr);
                return IntPtr.Zero;
            }
        }

        private static ValueHandle[] IntPtrConvertToStructArray(IntPtr ptr, int length)
        {
            // 2. 计算结构体大小
            int structSize = Marshal.SizeOf(typeof(ValueHandle));

            // 3. 创建目标数组
            ValueHandle[] structArray = new ValueHandle[length];

            // 4. 逐个从非托管内存复制到结构体数组
            for (int i = 0; i < length; i++)
            {
                IntPtr current = new IntPtr(ptr.ToInt64() + (i * structSize));
                structArray[i] = Marshal.PtrToStructure<ValueHandle>(current);
            }

            return structArray;
        }

        public QJS()
        {
            m_runtime = NewRuntime();
            m_ctx = NewContext(m_runtime);
        }
        public void Dispose()
        {
            FreeContext(m_ctx);
            FreeRuntime(m_runtime);
            m_ctx = UIntPtr.Zero;
            m_runtime = UIntPtr.Zero;
        }

        public void Reset()
        {
            ResetContext(m_ctx);
        }

        private Dictionary<int, Object> m_runtimeUserDatas = new Dictionary<int, Object>();
        private int m_runtimeUserDataIndex = 0;
        public void SetRuntimeUserData(int key, Object user_data)
        {
            m_runtimeUserDatas[++m_runtimeUserDataIndex] = user_data;
            SetRuntimeUserData(m_runtime, key, (IntPtr)m_runtimeUserDataIndex);
        }
        public Object GetRuntimeUserData(int key)
        {
            int idx = (int)GetRuntimeUserData(m_runtime, key);
            if (m_runtimeUserDatas.TryGetValue(idx, out Object user_data))
            {
                return user_data;
            }
            return null;
        }

        private Dictionary<int, Object> m_contextUserDatas = new Dictionary<int, Object>();
        private int m_contextUserDataIndex = 0;
        public void SetContextUserData(int key, Object user_data)
        {
            m_contextUserDatas[++m_contextUserDataIndex] = user_data;
            SetContextUserData(m_ctx, key, (IntPtr)m_contextUserDataIndex);
        }
        public Object GetContextUserData(int key)
        {
            int idx = (int)GetContextUserData(m_ctx, key);
            if (m_contextUserDatas.TryGetValue(idx, out Object user_data))
            {
                return user_data;
            }
            return null;
        }

        public ValueHandle GetGlobalObject()
        {
            return GetGlobalObject(m_ctx);
        }

        public void SetFreeingContextCallback(OnFreeingContextCallback cb)
        {
            SetFreeingContextCallback(m_ctx, cb);
        }

        public bool RemoveFreeingContextCallback(OnFreeingContextCallback cb)
        {
            return RemoveFreeingContextCallback(m_ctx, cb);
        }

        public byte[] LoadFile(string filename)
        {
            UInt64 outLen = 0;
            IntPtr data = LoadFile(m_ctx, out outLen, filename);
            if (data == IntPtr.Zero)
                return null;
            byte[] bytes = new byte[outLen];
            if (outLen == 0)
                return bytes;
            Marshal.Copy(data, bytes, 0, (int)outLen);
            FreeJsPointer(m_ctx, data);
            return bytes;
        }

        public ValueHandle RunScript(string script, ValueHandle parent, string filename)
        {
            byte[] uScript = Encoding.UTF8.GetBytes(script);
            byte[] uFilename = Encoding.UTF8.GetBytes(filename);
            return RunScript(m_ctx, uScript, parent, uFilename);
        }

        public ValueHandle RunScriptFile(string filename, ValueHandle parent)
        {
            return RunScriptFile(m_ctx, filename, parent);
        }

        public ValueHandle CallJsFunction(ValueHandle jsFunction, ValueHandle[] args, ValueHandle parent)
        {
            if (args == null || args.Length == 0)
            {
                ValueHandle[] argv = new ValueHandle[0];
                return CallJsFunction(m_ctx, jsFunction, IntPtr.Zero, 0, parent);
            }
            else
            {
                IntPtr ptr = StructArrayConvertToIntPtr(args);
                return CallJsFunction(m_ctx, jsFunction, ptr, args.Length, parent);
            }
        }

        public ValueHandle GetNamedJsValue(string varName, ValueHandle parent)
        {
            byte[] uVarName = Encoding.UTF8.GetBytes(varName);
            return GetNamedJsValue(m_ctx, uVarName, parent);
        }

        public bool SetNamedJsValue(string varName, ValueHandle varValue, ValueHandle parent)
        {
            byte[] uVarName = Encoding.UTF8.GetBytes(varName);
            return SetNamedJsValue(m_ctx, uVarName, varValue, parent);
        }

        public bool DeleteNamedJsValue(string varName, ValueHandle parent)
        {
            byte[] uVarName = Encoding.UTF8.GetBytes(varName);
            return DeleteNamedJsValue(m_ctx, uVarName, parent);
        }

        public bool HasNamedJsValue(string varName, ValueHandle parent)
        {
            byte[] uVarName = Encoding.UTF8.GetBytes(varName);
            return HasNamedJsValue(m_ctx, uVarName, parent);
        }

        public ValueHandle GetObjectPropertyKeys(ValueHandle jObj,
            bool onlyEnumerable = true, bool enableSymbol = false)
        {
            return GetObjectPropertyKeys(m_ctx, jObj, onlyEnumerable, enableSymbol);
        }

        public ValueHandle GetFunctionName(ValueHandle func)
        {
            return GetFunctionName(m_ctx, func);
        }

        public ValueHandle GetCurFrameFunction()
        {
            return GetCurFrameFunction(m_ctx);
        }

        public ValueHandle GetIndexedJsValue(UInt32 idx, ValueHandle parent)
        {
            return GetIndexedJsValue(m_ctx, idx, parent);
        }

        public bool SetIndexedJsValue(UInt32 idx, ValueHandle varValue, ValueHandle parent)
        {
            return SetIndexedJsValue(m_ctx, idx, varValue, parent);
        }

        public bool DeleteIndexedJsValue(UInt32 idx, ValueHandle parent)
        {
            return DeleteIndexedJsValue(m_ctx, idx, parent);
        }

        public Int64 GetLength(ValueHandle obj)
        {
            return GetLength(m_ctx, obj);
        }

        public ValueHandle GetPrototype(ValueHandle jObj)
        {
            return GetPrototype(m_ctx, jObj);
        }

        public bool SetPrototype(ValueHandle jObj, ValueHandle protoJVal)
        {
            return SetPrototype(m_ctx, jObj, protoJVal);
        }

        public bool DefineGetterSetter(ValueHandle parent, string propName, ValueHandle getter, ValueHandle setter)
        {
            byte[] uPropName = Encoding.UTF8.GetBytes(propName);
            return DefineGetterSetter(m_ctx, parent, uPropName, getter, setter);
        }

        public ValueHandle NewIntJsValue(int intValue)
        {
            return NewIntJsValue(m_ctx, intValue);
        }

        public ValueHandle NewInt64JsValue(Int64 intValue)
        {
            return NewInt64JsValue(m_ctx, intValue);
        }

        public ValueHandle NewUInt64JsValue(UInt64 intValue)
        {
            return NewUInt64JsValue(m_ctx, intValue);
        }

        public ValueHandle NewDoubleJsValue(double doubleValue)
        {
            return NewDoubleJsValue(m_ctx, doubleValue);
        }

        public ValueHandle NewStringJsValue(string stringValue)
        {
            byte[] uStr = Encoding.UTF8.GetBytes(stringValue);
            return NewStringJsValue(m_ctx, uStr);
        }

        public ValueHandle NewBoolJsValue(bool boolValue)
        {
            return NewBoolJsValue(m_ctx, boolValue);
        }

        public ValueHandle NewObjectJsValue()
        {
            return NewObjectJsValue(m_ctx);
        }

        private Dictionary<int, Object> m_objectUserDatas = new Dictionary<int, Object>();
        private int m_objectUserDataIndex = 0;
        public bool SetObjectUserData(ValueHandle value, Object user_data)
        {
            m_objectUserDatas[++m_objectUserDataIndex] = user_data;
            return SetObjectUserData(value, (IntPtr)m_objectUserDataIndex);
        }
        public Object GetObjectUserDataObject(ValueHandle value)
        {
            int idx = (int)GetObjectUserData(value);
            if (m_objectUserDatas.TryGetValue(idx, out Object user_data))
            {
                return user_data;
            }
            return null;
        }

        public ValueHandle NewArrayJsValue()
        {
            return NewArrayJsValue(m_ctx);
        }

        public ValueHandle NewThrowJsValue(ValueHandle throwWhat)
        {
            return NewThrowJsValue(m_ctx, throwWhat);
        }

        public ValueHandle NewDateJsValue(UInt64 ms_since_1970)
        {
            return NewDateJsValue(m_ctx, ms_since_1970);
        }

        public ValueHandle NewArrayBufferJsValueCopy(byte[] buf)
        {
            GCHandle handle = GCHandle.Alloc(buf, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();
            ValueHandle v = NewArrayBufferJsValueCopy(m_ctx, ptr, buf.Length);
            handle.Free();
            return v;
        }

        public void DetachArrayBufferJsValue(ref ValueHandle arrBuf)
        {
            DetachArrayBufferJsValue(m_ctx, ref arrBuf);
        }

        public byte[] GetArrayBuffer(ValueHandle arrBuf)
        {
            int bufLen = 0;
            IntPtr ptr = GetArrayBufferPtr(m_ctx, arrBuf, ref bufLen);
            if (ptr == IntPtr.Zero)
                return null;
            byte[] byteArray = new byte[bufLen];
            if (bufLen == 0)
                return byteArray;
            Marshal.Copy(ptr, byteArray, 0, bufLen); // 复制数据到byte数组
            return byteArray;
        }

        public ValueHandle GetArrayBufferByTypedArrayBuffer(ValueHandle typedArrBuf,
                                                            ref int out_byte_offset,
                                                            ref int out_byte_length,
                                                            ref int out_bytes_per_element)
        {
            return GetArrayBufferByTypedArrayBuffer(m_ctx, typedArrBuf, 
                ref out_byte_offset, 
                ref out_byte_length, 
                ref out_bytes_per_element);
        }

        public bool FillArrayBuffer(ValueHandle arrBuf, byte fill)
        {
            return FillArrayBuffer(m_ctx, arrBuf, fill);
        }

        public delegate ValueHandle JsFunctionCallback(QJS qjs, ValueHandle this_val, ValueHandle[] args, Object user_data);
        private Dictionary<int, KeyValuePair<JsFunctionCallback, Object>> m_newFunctionUserDatas = 
            new Dictionary<int, KeyValuePair<JsFunctionCallback, Object>>();
        private int m_newFunctionUserDataIndex = 0;
        public ValueHandle NewFunction(JsFunctionCallback cb, int argc, Object user_data)
        {
            m_newFunctionUserDatas[++m_newFunctionUserDataIndex] = new KeyValuePair<JsFunctionCallback, object>(cb, user_data);
            return NewFunction(m_ctx, _JsFunction, argc, (IntPtr)m_newFunctionUserDataIndex);
        }
        ValueHandle _JsFunction(UIntPtr ctx, ValueHandle this_val, int argc, IntPtr argv, IntPtr user_data)
        {
            if (m_newFunctionUserDatas.TryGetValue(user_data.ToInt32(), out var pair))
            {
                if (pair.Key != null)
                {
                    ValueHandle[] args = IntPtrConvertToStructArray(argv, argc);
                    ValueHandle v = pair.Key(this, this_val, args, pair.Value);
                    return v;
                }
            }
            return TheJsUndefined();
        }

        public string JsValueToString(ValueHandle value)
        {
            IntPtr p = JsValueToString(m_ctx, value);
            if (p == IntPtr.Zero)
                return null;

            //return Marshal.PtrToStringAnsi(p);

            int length = 0;
            while (Marshal.ReadByte(p, length) != 0) 
                length++; // 计算字符串长度（不包括结尾的\0字符）
            byte[] byteArray = new byte[length];
            Marshal.Copy(p, byteArray, 0, length);//复制数据到byte数组
            string result = Encoding.UTF8.GetString(byteArray); //转UTF-8编码
            return result;
        }

        public int JsValueToInt(ValueHandle value, int defVal = 0)
        {
            return JsValueToInt(m_ctx, value, defVal);
        }

        public Int64 JsValueToInt64(ValueHandle value, Int64 defVal = 0)
        {
            return JsValueToInt64(m_ctx, value, defVal);
        }

        public UInt64 JsValueToUInt64(ValueHandle value, UInt64 defVal = 0)
        {
            return JsValueToUInt64(m_ctx, value, defVal);
        }

        public double JsValueToDouble(ValueHandle value, double defVal = 0.0)
        {
            return JsValueToDouble(m_ctx, value, defVal);
        }

        public bool JsValueToBool(ValueHandle value, bool defVal/* = false*/)
        {
            return JsValueToBool(m_ctx, value, defVal);
        }

        public UInt64 JsValueToTimestamp(ValueHandle value)
        {
            return JsValueToTimestamp(m_ctx, value);
        }

        public ValueHandle CompileScript(string script, string filename = "")
        {
            byte[] uScript = Encoding.UTF8.GetBytes(script);
            byte[] uFilename = Encoding.UTF8.GetBytes(filename);
            return CompileScript(m_ctx, uScript, uFilename);
        }

        public byte[] JsValueToByteCode(ValueHandle value, bool byte_swap = false)
        {
            int outSize = 0;
            IntPtr ptr = JsValueToByteCode(m_ctx, value, ref outSize, byte_swap);
            if (ptr == IntPtr.Zero) 
                return null;
            byte[] outBytes = new byte[outSize];
            if (outSize == 0)
                return outBytes;
            Marshal.Copy(ptr, outBytes, 0, outSize);
            return outBytes;
        }

        public ValueHandle ByteCodeToJsValue(byte[] byteCode)
        {
            GCHandle handle = GCHandle.Alloc(byteCode, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();
            ValueHandle v = ByteCodeToJsValue(m_ctx, ptr, byteCode.Length);            
            handle.Free();
            return v;
        }

        public ValueHandle RunByteCode(byte[] byteCode)
        {
            GCHandle handle = GCHandle.Alloc(byteCode, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();
            ValueHandle v = RunByteCode(m_ctx, ptr, byteCode.Length);
            handle.Free();
            return v;
        }

        public bool JsValueIsGlobalObject(ValueHandle value)
        {
            return JsValueIsGlobalObject(m_ctx, value);
        }

        public ValueHandle JsonStringify(ValueHandle value, 
            ValueHandle replacer/*=TheJsUndefined()*/, ValueHandle space/*=TheJsUndefined()*/)
        {
            return JsonStringify(m_ctx, value, replacer, space);
        }
        public ValueHandle JsonStringify(ValueHandle value)
        {
            return JsonStringify(m_ctx, value, TheJsUndefined(), TheJsUndefined());
        }

        public ValueHandle JsonParse(string json)
        {
            byte[] uJson = Encoding.UTF8.GetBytes(json);
            return JsonParse(m_ctx, uJson);
        }

        public ValueHandle GetAndClearJsLastException()
        {
            return GetAndClearJsLastException(m_ctx);
        }

        public bool EnqueueJob(JsJobCallback funcJob, ValueHandle[] args)
        {
            if (args.Length == 0)
            {
                ValueHandle arg = new ValueHandle();
                return EnqueueJob(m_ctx, funcJob, ref arg, 0);
            }
            else
            {
                return EnqueueJob(m_ctx, funcJob, ref args[0], args.Length);
            }
        }

        public int ExecutePendingJob(ref IntPtr outRawCtx)
        {
            return ExecutePendingJob(m_runtime, ref outRawCtx);
        }

        public delegate bool WaitForExecutingJobsCallback(QJS qjs, int resExecutePendingJob, Object user_data);
        private Dictionary<int, KeyValuePair<WaitForExecutingJobsCallback, Object>> m_waitForExecutingJobsUserDatas = new Dictionary<int, KeyValuePair<WaitForExecutingJobsCallback, Object>>();
        private int m_waitForExecutingJobsUserDataIndex = 0;
        public int WaitForExecutingJobs(Int32 loopIntervalMS, WaitForExecutingJobsCallback cb, Object user_data)
        {
            m_waitForExecutingJobsUserDatas[++m_waitForExecutingJobsUserDataIndex] = new KeyValuePair<WaitForExecutingJobsCallback, object>(cb, user_data);
            return WaitForExecutingJobs(m_runtime, loopIntervalMS, _WaitForExecutingJobs, (IntPtr)m_waitForExecutingJobsUserDataIndex);
        }
        bool _WaitForExecutingJobs(UIntPtr rawCurCtx, int resExecutePendingJob, IntPtr user_data)
        {
            if (m_waitForExecutingJobsUserDatas.TryGetValue(user_data.ToInt32(), out var pair))
            {
                if (pair.Key != null)
                    return pair.Key(this, resExecutePendingJob, pair.Value);
            }
            return true;
        }

        public void SetDebuggerMode(bool onoff)
        {
            SetDebuggerMode(m_ctx, onoff);
        }

        public bool GetDebuggerMode()
        {
            return GetDebuggerMode(m_ctx);
        }

        public void SetDebuggerLineCallback(DebuggerLineCallback cb, IntPtr user_data)
        {
            SetDebuggerLineCallback(m_ctx, cb, user_data);
        }

        public bool GetDebuggerLineCallback(ref DebuggerLineCallback out_cb, ref IntPtr out_user_data)
        {
            return GetDebuggerLineCallback(m_ctx, ref out_cb, ref out_user_data);
        }

        public UInt32 GetDebuggerStackDepth()
        {
            return GetDebuggerStackDepth(m_ctx);
        }

        public ValueHandle GetDebuggerBacktrace(UIntPtr pc)
        {
            return GetDebuggerBacktrace(m_ctx, pc);
        }

        public ValueHandle GetDebuggerClosureVariables(int stack_idx)
        {
            return GetDebuggerClosureVariables(m_ctx, stack_idx);
        }

        public ValueHandle GetDebuggerLocalVariables(int stack_idx)
        {
            return GetDebuggerLocalVariables(m_ctx, stack_idx);
        }

        public int LoadExtend(string extendFile, ValueHandle parent, IntPtr userData/* = NULL*/)
        {
            return LoadExtend(m_ctx, extendFile, parent, userData);
        }

        public int[] GetExtendList()
        {
            int count = 0;
            IntPtr intPtr = GetExtendList(m_ctx, ref count);
            if (intPtr == IntPtr.Zero)
            {
                return null;
            }
            else
            {
                int[] array = new int[count];
                Marshal.Copy(intPtr, array, 0, count);
                return array;
            }
        }

        public UIntPtr GetExtendHandle(int extendId)
        {
            return GetExtendHandle(m_ctx, extendId);
        }

        public string GetExtendFile(int extendId)
        {
            IntPtr ptr = GetExtendFile(m_ctx, extendId);
            if (ptr == IntPtr.Zero)
            {
                return null;
            }
            return Marshal.PtrToStringAnsi(ptr);
        }

        public ValueHandle GetExtendParentObject(int extendId)
        {
            return GetExtendParentObject(m_ctx, extendId);
        }

        public void UnloadExtend(int extendId)
        {
            UnloadExtend(m_ctx, extendId);
        }

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ValueHandle
    {
        UIntPtr ctx;
        UInt64 value;
    }
    
}
