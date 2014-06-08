#include "node_printer.hpp"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#pragma  comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

v8::Handle<v8::Value> getPrinters(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;
    PRINTER_INFO_2 *printers = NULL;
    DWORD printers_size = 0;
    DWORD printers_size_bytes = 0;
    DWORD Level = 2;
    DWORD flags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_NETWORK | PRINTER_ENUM_NAME;
    // First try to retrieve the number of printers
    BOOL bError = EnumPrinters(flags, NULL, 2, NULL, 0, &printers_size_bytes, &printers_size);
    if(bError)
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Error on EnumPrinters to retrieve the number of printers")));
    }
    // allocate the required memmory
    if((printers = (PRINTER_INFO_2*) malloc(printers_size_bytes)) == 0)
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Error on allocate memmory for printers")));
    }

    bError = EnumPrinters(flags, NULL, 2, (LPBYTE)printers, printers_size_bytes, &printers_size_bytes, &printers_size_bytes, &printers_size);
    if(bError)
    {
        free(printers);
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Error on EnumPrinters")));
    }
    v8::Local<v8::Array> result = v8::Array::New(printers_size);
    for(int i = 0, PRINTER_INFO_2 *printer = printers; i < printers_size; ++i, ++printer)
    {
        v8::Local<v8::Object> result_printer = v8::Object::New();
        //LPTSTR               pPrinterName;
        result_printer->Set(v8::String::NewSymbol("name"), v8::String::New(printer->pPrinterName));
        //DWORD                Status;
        // statuses from:
        // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
        if(printer->Status & PRINTER_STATUS_WAITING)
        {
            result_printer->Set(v8::String::NewSymbol("status"), v8::String::New("IDLE"));
        }
        else if(printer->Status & PRINTER_STATUS_PRINTING)
        {
            result_printer->Set(v8::String::NewSymbol("status"), v8::String::New("PRINTING"));
        }
        else if(printer->Status & PRINTER_STATUS_OFFLINE)
        {
            result_printer->Set(v8::String::NewSymbol("status"), v8::String::New("STOPPED"));
        }
        else if(printer->Status & PRINTER_STATUS_BUSY)
        {
            result_printer->Set(v8::String::NewSymbol("status"), v8::String::New("BUSY"));
        }
        else if(printer->Status & PRINTER_STATUS_ERROR)
        {
            result_printer->Set(v8::String::NewSymbol("status"), v8::String::New("ERROR"));
            if(printer->Status & PRINTER_STATUS_NO_TONER)
            {
                result_printer->Set(v8::String::NewSymbol("statusReason"), v8::String::New("NO-TONER"));
            }
            else if(printer->Status & PRINTER_STATUS_OUT_OF_MEMORY)
            {
                result_printer->Set(v8::String::NewSymbol("statusReason"), v8::String::New("OUT-OF-MEMORY"));
            }
            else if(printer->Status & PRINTER_STATUS_PAPER_JAM)
            {
                result_printer->Set(v8::String::NewSymbol("statusReason"), v8::String::New("PAPER-JAM"));
            }
            else if(printer->Status & PRINTER_STATUS_PAPER_OUT)
            {
                result_printer->Set(v8::String::NewSymbol("statusReason"), v8::String::New("PAPER-OUT"));
            }
            else if(printer->Status & PRINTER_STATUS_PAPER_PROBLEM)
            {
                result_printer->Set(v8::String::NewSymbol("statusReason"), v8::String::New("PAPER-PROBLEM"));
            }
        }
        //TODO: to finish to extract all data
        //LPTSTR               pServerName;
        //LPTSTR               pShareName;
        //LPTSTR               pPortName;
        //LPTSTR               pDriverName;
        //LPTSTR               pComment;
        //LPTSTR               pLocation;
        //LPDEVMODE            pDevMode;
        //LPTSTR               pSepFile;
        //LPTSTR               pPrintProcessor;
        //LPTSTR               pDatatype;
        //LPTSTR               pParameters;
        //PSECURITY_DESCRIPTOR pSecurityDescriptor;
        //DWORD                Attributes;
        //DWORD                Priority;
        //DWORD                DefaultPriority;
        //DWORD                StartTime;
        //DWORD                UntilTime;
        //DWORD                cJobs;
        //DWORD
        //AveragePPM;
        result->Set(i, result_printer);
    }
    free(printers);
    return scope.Close(result);
}

v8::Handle<v8::Value> getSupportedFormats(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    v8::Local<v8::Array> result = v8::Array::New();
    int i = 0;
    result->Set(i++, v8::String::New("RAW"));
    result->Set(i++, v8::String::New("TEXT"));
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

    char* data(NULL);
    size_t data_len(0);
    v8::Handle<v8::Value> arg0(iArgs[0]);

    if(arg0->IsString())
    {
        v8::String::Utf8Value data_str(arg0->ToString());
        data = *data_str;
        data_len = data_str.length();
    }
    else if(arg0->IsObject() && arg0.As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())
    {
        data = static_cast<char*>(arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayData());
        data_len = arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayDataLength();
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string or NativeBuffer")));
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 3, type);
    int args_length = iArgs.Length();

    BOOL     bStatus = true;
    HANDLE     hPrinter = NULL;
    DOC_INFO_1 DocInfo;
    DWORD      dwJob = 0L;
    DWORD      dwBytesWritten = 0L;

    // Open a handle to the printer.
    bStatus = OpenPrinter( (LPTSTR)(*printername), &hPrinter, NULL );
    if (bStatus) {
        // Fill in the structure with info about this "document."
        DocInfo.pDocName = (LPTSTR)(*docname);
        DocInfo.pOutputFile = NULL;
        DocInfo.pDatatype = (LPTSTR)(*type);

        // Inform the spooler the document is beginning.
        dwJob = StartDocPrinter( hPrinter, 1, (LPBYTE)&DocInfo );
        if (dwJob > 0) {
            // Start a page.
            bStatus = StartPagePrinter( hPrinter );
            if (bStatus) {
                // Send the data to the printer.
                //TODO: check with sizeof(LPTSTR) is the same as sizeof(char)
                bStatus = WritePrinter( hPrinter, (LPTSTR)(data), data_len, &dwBytesWritten);
                EndPagePrinter (hPrinter);
            }else{
                RETURN_EXCEPTION_STR("StartPagePrinter error");
            }
            // Inform the spooler that the document is ending.
            EndDocPrinter( hPrinter );
        }else{
            RETURN_EXCEPTION_STR("StartDocPrinter error");
        }
        // Close the printer handle.
        ClosePrinter( hPrinter );
    }else{
        RETURN_EXCEPTION(v8::String::Concat(v8::String::New("OpenPrinter error "), args[1]->ToString()))
    }
    // Check to see if correct number of bytes were written.
    if (!bStatus || (dwBytesWritten != data_len)) {
        bStatus = false;
        RETURN_EXCEPTION_STR("not sent all bytes");
    } else {
        bStatus = true;
    }
    bool ret_sttaus = false||bStatus;
    return scope.Close(v8::Boolean::New(ret_sttaus));
}
