#include "node_printer_napi.hpp"

#include <node_buffer.h>

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "getDefaultPrinterName"),
        Napi::Function::New(env, getDefaultPrinterName));
    return exports;
}
NODE_API_MODULE(node_printer, Init)

// Helpers

bool getStringOrBufferFromV8Value(v8::Handle<v8::Value> iV8Value, std::string &oData)
{
    if (iV8Value->IsString())
    {
        v8::String::Utf8Value data_str_v8(iV8Value->ToString());
        oData.assign(*data_str_v8, data_str_v8.length());
        return true;
    }
    if (iV8Value->IsObject() && node::Buffer::HasInstance(iV8Value))
    {
        oData.assign(node::Buffer::Data(iV8Value), node::Buffer::Length(iV8Value));
        return true;
    }
    return false;
}
