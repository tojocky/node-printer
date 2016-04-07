#ifndef NODE_PRINTER_SRC_MACROS_H
#define NODE_PRINTER_SRC_MACROS_H

#include <node_version.h>

// NODE_MODULE_VERSION was incremented for v0.11


#if NODE_VERSION_AT_LEAST(0, 11, 9)
#  define MY_NODE_MODULE_ISOLATE_DECL v8::Isolate* isolate = v8::Isolate::GetCurrent();
#  define MY_NODE_MODULE_ISOLATE      isolate
#  define MY_NODE_MODULE_ISOLATE_PRE  isolate, 
#  define MY_NODE_MODULE_ISOLATE_POST , isolate 
#  define MY_NODE_MODULE_HANDLESCOPE MY_NODE_MODULE_ISOLATE_DECL v8::HandleScope scope(MY_NODE_MODULE_ISOLATE)
#  define MY_NODE_MODULE_CALLBACK(name) void name(const v8::FunctionCallbackInfo<v8::Value>& iArgs)
#  define V8_VALUE_NEW(type, value)   v8::type::New(MY_NODE_MODULE_ISOLATE_PRE value)
#  define V8_VALUE_NEW_DEFAULT(type)   v8::type::New(MY_NODE_MODULE_ISOLATE)
#  define V8_STRING_NEW_UTF8(value)   v8::String::NewFromUtf8(MY_NODE_MODULE_ISOLATE_PRE value)
#  define V8_STRING_NEW_2BYTES(value)   v8::String::NewFromTwoByte(MY_NODE_MODULE_ISOLATE_PRE value)

#  define RETURN_EXCEPTION(msg)  isolate->ThrowException(v8::Exception::TypeError(msg));    \
    return

#  define RETURN_EXCEPTION_STR(msg) RETURN_EXCEPTION(V8_STRING_NEW_UTF8(msg))
#  define MY_NODE_MODULE_RETURN_VALUE(value)   iArgs.GetReturnValue().Set(value);   \
    return
#  define MY_NODE_MODULE_RETURN_UNDEFINED()   return
#else
#  define MY_NODE_MODULE_ISOLATE_DECL
#  define MY_NODE_MODULE_ISOLATE
#  define MY_NODE_MODULE_ISOLATE_PRE
#  define MY_NODE_MODULE_ISOLATE_POST
#  define MY_NODE_MODULE_HANDLESCOPE v8::HandleScope scope;
#  define MY_NODE_MODULE_CALLBACK(name) v8::Handle<v8::Value> name(const v8::Arguments& iArgs)
#  define V8_VALUE_NEW(type, value)   v8::type::New(value)
#  define V8_VALUE_NEW_DEFAULT(type)   v8::type::New()
#  define V8_STRING_NEW_UTF8(value)   v8::String::New(MY_NODE_MODULE_ISOLATE_PRE value)
#  define V8_STRING_NEW_2BYTES(value)   v8::String::New(MY_NODE_MODULE_ISOLATE_PRE value)

#  define RETURN_EXCEPTION(msg) return v8::ThrowException(v8::Exception::TypeError(msg)) 

#  define RETURN_EXCEPTION_STR(msg) RETURN_EXCEPTION(V8_STRING_NEW_UTF8(msg))
#  define MY_NODE_MODULE_RETURN_VALUE(value)   return scope.Close(value)
#  define MY_NODE_MODULE_RETURN_UNDEFINED()   return scope.Close(v8::Undefined())
#endif

#if NODE_VERSION_AT_LEAST(0, 11, 10) // for node-webkit v.0.9.2 which uses node v0.11.9
#  define V8_VALUE_NEW_V_0_11_10(type, value)          v8::type::New(MY_NODE_MODULE_ISOLATE_PRE value)
#  define V8_VALUE_NEW_DEFAULT_V_0_11_10(type)         v8::type::New(MY_NODE_MODULE_ISOLATE)
#else
#  define V8_VALUE_NEW_V_0_11_10(type, value)          v8::type::New(value)
#  define V8_VALUE_NEW_DEFAULT_V_0_11_10(type)         v8::type::New()
#endif


#define V8_STR_CONC(left, right)                              \
	v8::String::Concat(V8_STRING_NEW_UTF8(left), V8_STRING_NEW_UTF8(right))
		
#define REQUIRE_ARGUMENTS(args, n)                                                   \
    if (args.Length() < (n)) {                                                 \
       RETURN_EXCEPTION_STR("Expected " #n " arguments");                       \
    }


#define REQUIRE_ARGUMENT_EXTERNAL(i, var)                                      \
    if (args.Length() <= (i) || !args[i]->IsExternal()) {                      \
        RETURN_EXCEPTION_STR("Argument " #i " invalid");       \
    }                                                                          \
    v8::Local<v8::External> var = v8::Local<v8::External>::Cast(args[i]);

#define REQUIRE_ARGUMENT_OBJECT(args, i, var)                                      \
    if (args.Length() <= (i) || !args[i]->IsObject()) {                      \
        RETURN_EXCEPTION_STR("Argument " #i " is not an object");       \
    }                                                                          \
    v8::Local<v8::Object> var = v8::Local<v8::Object>::Cast(args[i]);


#define REQUIRE_ARGUMENT_FUNCTION(i, var)                                      \
    if (args.Length() <= (i) || !args[i]->IsFunction()) {                      \
        RETURN_EXCEPTION_STR("Argument " #i " must be a function");                 \
    }                                                                          \
    v8::Local<v8::Function> var = v8::Local<v8::Function>::Cast(args[i]);


#define ARG_CHECK_STRING(args, i)                                        \
    if (args.Length() <= (i) || !args[i]->IsString()) {                        \
        RETURN_EXCEPTION_STR("Argument " #i " must be a string");                   \
    }                                                                          \

#define REQUIRE_ARGUMENT_STRING(args, i, var)                                        \
    ARG_CHECK_STRING(args, i);                                                       \
    v8::String::Utf8Value var(args[i]->ToString());

#define REQUIRE_ARGUMENT_STRINGW(args, i, var)                                        \
    ARG_CHECK_STRING(args, i);                                                       \
    v8::String::Value var(args[i]->ToString());


#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                     \
    v8::Local<v8::Function> var;                                                       \
    if (args.Length() > i && !args[i]->IsUndefined()) {                        \
        if (!args[i]->IsFunction()) {                                          \
            RETURN_EXCEPTION_STR("Argument " #i " must be a function");             \
        }                                                                      \
        var = v8::Local<v8::Function>::Cast(args[i]);                                  \
    }


#define REQUIRE_ARGUMENT_INTEGER(args, i, var)                             \
    int var;                                                                   \
    if (args[i]->IsInt32()) {                                             \
        var = args[i]->Int32Value();                                           \
    }                                                                          \
    else {                                                                     \
        RETURN_EXCEPTION_STR("Argument " #i " must be an integer");                 \
    }

#define OPTIONAL_ARGUMENT_INTEGER(args, i, var, default)                             \
    int var;                                                                   \
    if (args.Length() <= (i)) {                                                \
        var = (default);                                                       \
    }                                                                          \
    else if (args[i]->IsInt32()) {                                             \
        var = args[i]->Int32Value();                                           \
    }                                                                          \
    else {                                                                     \
        RETURN_EXCEPTION_STR("Argument " #i " must be an integer");                 \
    }
#define EMIT_EVENT(obj, argc, argv)                                            \
    TRY_CATCH_CALL((obj),                                                      \
        Local<Function>::Cast((obj)->Get(String::NewSymbol("emit"))),          \
        argc, argv                                                             \
    );

#define TRY_CATCH_CALL(context, callback, argc, argv)                          \
{   TryCatch try_catch;                                                        \
    (callback)->Call((context), (argc), (argv));                               \
    if (try_catch.HasCaught()) {                                               \
        FatalException(try_catch);                                             \
    }                                                                          \
}

#endif
