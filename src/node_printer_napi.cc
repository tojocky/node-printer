#include "node_printer_napi.hpp"
#include <iostream>
#include <node_buffer.h>

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "getDefaultPrinterName"),
        Napi::Function::New(env, getDefaultPrinterName));
    exports.Set(
        Napi::String::New(env, "printDirect"),
        Napi::Function::New(env, PrintDirect));
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
        std::cout << "string" << std::endl;
        std::cout << oData << std::endl;
        return true;
    }
    if (iValue.IsObject() && iValue.IsBuffer())
    {
        oData.assign(iValue.As<Napi::Buffer<char>>().Data(), iValue.As<Napi::Buffer<char>>().Length());
        std::cout << "Buffer" << std::endl;
        std::cout << oData << std::endl;
        return true;
    }
    return false;
}
