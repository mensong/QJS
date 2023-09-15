/* File generated automatically by the QuickJS compiler. */

#include "quickjs-libc.h"

const uint32_t qjsc_qjsc_test_size = 71;

const uint8_t qjsc_qjsc_test[71] = {
 0x02, 0x02, 0x06, 0x61, 0x61, 0x61, 0x18, 0x71,
 0x6a, 0x73, 0x63, 0x2d, 0x74, 0x65, 0x73, 0x74,
 0x2e, 0x6a, 0x73, 0x0e, 0x00, 0x06, 0x00, 0xa6,
 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x1a,
 0x01, 0xa8, 0x01, 0x00, 0x00, 0x00, 0x3f, 0xe3,
 0x00, 0x00, 0x00, 0x00, 0x3e, 0xe3, 0x00, 0x00,
 0x00, 0x00, 0xbe, 0x7b, 0x39, 0xe3, 0x00, 0x00,
 0x00, 0x38, 0xe3, 0x00, 0x00, 0x00, 0xce, 0x28,
 0xc8, 0x03, 0x01, 0x03, 0x1f, 0x21, 0x26,
};

static JSContext *JS_NewCustomContext(JSRuntime *rt)
{
  JSContext *ctx = JS_NewContextRaw(rt);
  if (!ctx)
    return NULL;
  JS_AddIntrinsicBaseObjects(ctx);
  JS_AddIntrinsicDate(ctx);
  JS_AddIntrinsicEval(ctx);
  JS_AddIntrinsicStringNormalize(ctx);
  JS_AddIntrinsicRegExp(ctx);
  JS_AddIntrinsicJSON(ctx);
  JS_AddIntrinsicProxy(ctx);
  JS_AddIntrinsicMapSet(ctx);
  JS_AddIntrinsicTypedArrays(ctx);
  JS_AddIntrinsicPromise(ctx);
  JS_AddIntrinsicBigInt(ctx);
  return ctx;
}

int main(int argc, char **argv)
{
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  js_std_set_worker_new_context_func(JS_NewCustomContext);
  js_std_init_handlers(rt);
  JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);
  ctx = JS_NewCustomContext(rt);
  js_std_add_helpers(ctx, argc, argv);
  js_std_eval_binary(ctx, qjsc_qjsc_test, qjsc_qjsc_test_size, 0);
  js_std_loop(ctx);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
