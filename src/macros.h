#ifndef NODE_PRINTER_SRC_MACROS_H
#define NODE_PRINTER_SRC_MACROS_H

#define RETURN_EXCEPTION(msg)                              \
return v8::ThrowException(v8::Exception::TypeError(msg)); 

#define RETURN_EXCEPTION_STR(msg)                              \
return v8::ThrowException(v8::Exception::TypeError(                    \
            v8::String::New(msg))                                  \
        ); 


#define V8_STR_CONC(left, right)                              \
	v8::String::Concat(v8::String::New(left), v8::String::New(right))
		
#define REQUIRE_ARGUMENTS(args, n)                                                   \
    if (args.Length() < (n)) {                                                 \
        return v8::ThrowException(                                                 \
            v8::Exception::TypeError(v8::String::New("Expected " #n " arguments"))      \
        );                                                                     \
    }


#define REQUIRE_ARGUMENT_EXTERNAL(i, var)                                      \
    if (args.Length() <= (i) || !args[i]->IsExternal()) {                      \
        return v8::ThrowException(                                                 \
            v8::Exception::TypeError(v8::String::New("Argument " #i " invalid"))       \
        );                                                                     \
    }                                                                          \
    v8::Local<v8::External> var = v8::Local<v8::External>::Cast(args[i]);


#define REQUIRE_ARGUMENT_FUNCTION(i, var)                                      \
    if (args.Length() <= (i) || !args[i]->IsFunction()) {                      \
        return v8::ThrowException(v8::Exception::TypeError(                            \
            v8::String::New("Argument " #i " must be a function"))                 \
        );                                                                     \
    }                                                                          \
    v8::Local<v8::Function> var = v8::Local<v8::Function>::Cast(args[i]);


#define ARG_CHECK_STRING(args, i)                                        \
    if (args.Length() <= (i) || !args[i]->IsString()) {                        \
        return v8::ThrowException(v8::Exception::TypeError(                            \
            v8::String::New("Argument " #i " must be a string"))                   \
        );                                                                     \
    }                                                                          \

#define REQUIRE_ARGUMENT_STRING(args, i, var)                                        \
    ARG_CHECK_STRING(args, i);                                                       \
    v8::String::Utf8Value var(args[i]->ToString());


#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                     \
    v8::Local<v8::Function> var;                                                       \
    if (args.Length() > i && !args[i]->IsUndefined()) {                        \
        if (!args[i]->IsFunction()) {                                          \
            return v8::ThrowException(v8::Exception::TypeError(                        \
                v8::String::New("Argument " #i " must be a function"))             \
            );                                                                 \
        }                                                                      \
        var = v8::Local<v8::Function>::Cast(args[i]);                                  \
    }


#define OPTIONAL_ARGUMENT_INTEGER(i, var, default)                             \
    int var;                                                                   \
    if (args.Length() <= (i)) {                                                \
        var = (default);                                                       \
    }                                                                          \
    else if (args[i]->IsInt32()) {                                             \
        var = args[i]->Int32Value();                                           \
    }                                                                          \
    else {                                                                     \
        return v8::ThrowException(v8::Exception::TypeError(                            \
            v8::String::New("Argument " #i " must be an integer"))                 \
        );                                                                     \
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
