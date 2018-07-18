#include "node_printer_napi.hpp"
#include <iostream>
#include <node_buffer.h>

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "getDefaultPrinterName"),
        Napi::Function::New(env, getDefaultPrinterName));
    exports.Set(
        Napi::String::New(env, "getPrinters"),
        Napi::Function::New(env, getPrinters));
    exports.Set(
        Napi::String::New(env, "printDirect"),
        Napi::Function::New(env, PrintDirect));
    exports.Set(
        Napi::String::New(env, "getSupportedJobCommands"),
        Napi::Function::New(env, getSupportedJobCommands));
    exports.Set(
        Napi::String::New(env, "printFile"),
        Napi::Function::New(env, PrintFile));
    exports.Set(
        Napi::String::New(env, "getPrinter"),
        Napi::Function::New(env, getPrinter));
    exports.Set(
        Napi::String::New(env, "getPrinterDriverOptions"),
        Napi::Function::New(env, getPrinterDriverOptions));
    exports.Set(
        Napi::String::New(env, "getSupportedPrintFormats"),
        Napi::Function::New(env, getSupportedPrintFormats));
    exports.Set(
        Napi::String::New(env, "getJob"),
        Napi::Function::New(env, getJob));
    exports.Set(
        Napi::String::New(env, "setJob"),
        Napi::Function::New(env, setJob));
    return exports;
}
NODE_API_MODULE(node_printer, Init)

// Helpers

bool getStringOrBufferFromNapiValue(Napi::Value iValue, std::string &oData)
{
    if (iValue.IsString())
    {
        // Napi::String::Utf8Value data_str(iValue.ToString());
        oData.assign(iValue.ToString().Utf8Value());
        return true;
    }
    if (iValue.IsObject() && iValue.IsBuffer())
    {
        oData.assign(iValue.As<Napi::Buffer<char>>().Data(), iValue.As<Napi::Buffer<char>>().Length());
        return true;
    }
    return false;
}
