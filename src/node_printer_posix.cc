#include "node_printer.hpp"

#include <string>

#include <cups/cups.h>

v8::Handle<v8::Value> getPrinters(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;

    cups_dest_t *printers = NULL, *printer = NULL;
    int printers_size = cupsGetDests(&printers);
    v8::Local<v8::Array> result = v8::Array::New(printers_size);
    int i, j;
    for(i = 0, printer = printers; i < printers_size; ++i, ++printer)
    {
        v8::Local<v8::Object> result_printer = v8::Object::New();
        result_printer->Set(v8::String::NewSymbol("name"), v8::String::New(printer->name));
        result_printer->Set(v8::String::NewSymbol("isDefault"), v8::Boolean::New(static_cast<bool>(printer->is_default)));

        if(printer->instance)
        {
            result_printer->Set(v8::String::NewSymbol("instance"), v8::String::New(printer->instance));
        }
        v8::Local<v8::Object> result_printer_options = v8::Object::New();
        cups_option_t *dest_option = NULL; 
        for(j = 0, dest_option = printer->options; j < printer->num_options; ++j, ++dest_option)
        {
            result_printer_options->Set(v8::String::NewSymbol(dest_option->name), v8::String::New(dest_option->value));
        }
        result_printer->Set(v8::String::NewSymbol("options"), result_printer_options);
        result->Set(i, result_printer);
    }
    cupsFreeDests(printers_size, printers);
    return scope.Close(result);
}

v8::Handle<v8::Value> getSupportedFormats(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    v8::Local<v8::Array> result = v8::Array::New();
    int i = 0;
    result->Set(i++, v8::String::New("RAW"));
    result->Set(i++, v8::String::New("TEXT"));
    result->Set(i++, v8::String::New("PDF"));
    result->Set(i++, v8::String::New("JPEG"));
    result->Set(i++, v8::String::New("POSTSCRIPT"));
    result->Set(i++, v8::String::New("COMMAND"));
    result->Set(i++, v8::String::New("AUTO"));
    return scope.Close(result);
}

v8::Handle<v8::Value> PrintDirect(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    REQUIRE_ARGUMENTS(iArgs, 4);

    // can be string or buffer
    if(iArgs.Length()<=0)
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 missing")));
    }

    std::string data;
    v8::Handle<v8::Value> arg0(iArgs[0]);

    if(arg0->IsString())
    {
        v8::String::Utf8Value data_str_v8(arg0->ToString());
        data.assign(*data_str_v8, data_str_v8.length());
    }
    else if(arg0->IsObject() && arg0.As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())
    {
        data.assign(static_cast<char*>(arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayData()),
                    arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayDataLength());
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string or Buffer")));
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 3, type);
    std::string type_str(*type);
    if(type_str == "RAW")
    {
        type_str = CUPS_FORMAT_RAW;
    }
    else if (type_str == "TEXT")
    {
        type_str = CUPS_FORMAT_TEXT;
    }
    else if (type_str == "PDF")
    {
        type_str = CUPS_FORMAT_PDF;
    }
    else if (type_str == "JPEG")
    {
        type_str = CUPS_FORMAT_JPEG;
    }
    else if (type_str == "POSTSCRIPT")
    {
        type_str = CUPS_FORMAT_POSTSCRIPT;
    }
    else if (type_str == "COMMAND")
    {
        type_str = CUPS_FORMAT_COMMAND;
    }
    else if (type_str == "AUTO")
    {
        type_str = CUPS_FORMAT_AUTO;
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("unsupported format type")));
    }
    int num_options = 0;
    cups_option_t *options = NULL;
    int job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, *printername, *docname, num_options, options);
    if(job_id > 0)
    {
        cupsStartDocument(CUPS_HTTP_DEFAULT, *printername, job_id, *docname, type_str.c_str(), 1 /*last document*/);
        /* cupsWriteRequestData can be called as many times as needed */
        //TODO: to split big buffer
        cupsWriteRequestData(CUPS_HTTP_DEFAULT, data.c_str(), data.size());
        cupsFinishDocument(CUPS_HTTP_DEFAULT, *printername);
    }
    return scope.Close(v8::Number::New(job_id));
}
