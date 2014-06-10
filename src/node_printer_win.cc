#include "node_printer.hpp"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#pragma  comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <string>
#include <map>

namespace{
    typedef std::map<std::string, DWORD> StatusMapType;
    typedef std::map<std::string, DWORD> AttributeMapType;
    const StatusMapType& getStatusMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define STATUS_PRINTER_ADD(value, type) result.insert(std::make_pair(#type, type))
        STATUS_PRINTER_ADD("BUSY", PRINTER_STATUS_BUSY);
        STATUS_PRINTER_ADD("DOOR-OPEN", PRINTER_STATUS_DOOR_OPEN);
        STATUS_PRINTER_ADD("ERROR", PRINTER_STATUS_ERROR);
        STATUS_PRINTER_ADD("INITIALIZING", PRINTER_STATUS_INITIALIZING);
        STATUS_PRINTER_ADD("IO-ACTIVE", PRINTER_STATUS_IO_ACTIVE);
        STATUS_PRINTER_ADD("MANUAL-FEED", PRINTER_STATUS_MANUAL_FEED);
        STATUS_PRINTER_ADD("NO-TONER", PRINTER_STATUS_NO_TONER);
        STATUS_PRINTER_ADD("NOT-AVAILABLE", PRINTER_STATUS_NOT_AVAILABLE);
        STATUS_PRINTER_ADD("OFFLINE", PRINTER_STATUS_OFFLINE);
        STATUS_PRINTER_ADD("OUT-OF-MEMORY", PRINTER_STATUS_OUT_OF_MEMORY);
        STATUS_PRINTER_ADD("OUTPUT-BIN-FULL", PRINTER_STATUS_OUTPUT_BIN_FULL);
        STATUS_PRINTER_ADD("PAGE-PUNT", PRINTER_STATUS_PAGE_PUNT);
        STATUS_PRINTER_ADD("PAPER-JAM", PRINTER_STATUS_PAPER_JAM);
        STATUS_PRINTER_ADD("PAPER-OUT", PRINTER_STATUS_PAPER_OUT);
        STATUS_PRINTER_ADD("PAPER-PROBLEM", PRINTER_STATUS_PAPER_PROBLEM);
        STATUS_PRINTER_ADD("PAUSED", PRINTER_STATUS_PAUSED);
        STATUS_PRINTER_ADD("PENDING-DELETION", PRINTER_STATUS_PENDING_DELETION);
        STATUS_PRINTER_ADD("POWER-SAVE", PRINTER_STATUS_POWER_SAVE);
        STATUS_PRINTER_ADD("PRINTING", PRINTER_STATUS_PRINTING);
        STATUS_PRINTER_ADD("PROCESSING", PRINTER_STATUS_PROCESSING);
        STATUS_PRINTER_ADD("SERVER-UNKNOWN", PRINTER_STATUS_SERVER_UNKNOWN);
        STATUS_PRINTER_ADD("TONER-LOW", PRINTER_STATUS_TONER_LOW);
        STATUS_PRINTER_ADD("USER-INTERVENTION", PRINTER_STATUS_USER_INTERVENTION);
        STATUS_PRINTER_ADD("WAITING", PRINTER_STATUS_WAITING);
        STATUS_PRINTER_ADD("WARMING-UP", PRINTER_STATUS_WARMING_UP);
#undef STATUS_PRINTER_ADD
        return result;
    }
    const AttributeMapType& getAttributeMap()
    {
        static AttributeType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define ATTRIBUTE_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        ATTRIBUTE_PRINTER_ADD("DIRECT", PRINTER_ATTRIBUTE_DIRECT);
        ATTRIBUTE_PRINTER_ADD("DO-COMPLETE-FIRST", PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST);
        ATTRIBUTE_PRINTER_ADD("ENABLE-DEVQ", PRINTER_ATTRIBUTE_ENABLE_DEVQ);
        ATTRIBUTE_PRINTER_ADD("HIDDEN", HIDDEN);
        ATTRIBUTE_PRINTER_ADD("KEEPPRINTEDJOBS", PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS);
        ATTRIBUTE_PRINTER_ADD("LOCAL", PRINTER_ATTRIBUTE_LOCAL);
        ATTRIBUTE_PRINTER_ADD("NETWORK", PRINTER_ATTRIBUTE_NETWORK);
        ATTRIBUTE_PRINTER_ADD("PUBLISHED", PRINTER_ATTRIBUTE_PUBLISHED);
        ATTRIBUTE_PRINTER_ADD("QUEUED", PRINTER_ATTRIBUTE_QUEUED);
        ATTRIBUTE_PRINTER_ADD("RAW-ONLY", PRINTER_ATTRIBUTE_RAW_ONLY);
        ATTRIBUTE_PRINTER_ADD("SHARED", PRINTER_ATTRIBUTE_SHARED);
        // XP
#ifdef PRINTER_ATTRIBUTE_FAX
        ATTRIBUTE_PRINTER_ADD("FAX", PRINTER_ATTRIBUTE_FAX);
#endif
        // vista
#ifdef PRINTER_ATTRIBUTE_FRIENDLY_NAME
        ATTRIBUTE_PRINTER_ADD("FRIENDLY-NAME", PRINTER_ATTRIBUTE_FRIENDLY_NAME);
        ATTRIBUTE_PRINTER_ADD("MACHINE", PRINTER_ATTRIBUTE_MACHINE);
        ATTRIBUTE_PRINTER_ADD("PUSHED-USER", PRINTER_ATTRIBUTE_PUSHED_USER);
        ATTRIBUTE_PRINTER_ADD("PUSHED-MACHINE", PRINTER_ATTRIBUTE_PUSHED_MACHINE);
#endif
        // server 2003
#ifdef PRINTER_ATTRIBUTE_TS
        ATTRIBUTE_PRINTER_ADD("TS", PRINTER_ATTRIBUTE_TS);
#endif
#undef STATUS_PRINTER_ADD
        return result;
    }
#undef COMBINE__
}

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
        v8::Local<v8::Array> result_printer_status = v8::Array::New();
        //DWORD                Status;
        // statuses from:
        // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
        int i_status = 0;
        for(StatusMapType::const_iterator itStatus = getStatusMap().begin(); itStatus < getStatusMap().end(); ++itStatus, ++i_status)
        {
            if(printer->Status & itStatus->second)
            {
                result_printer_status.Set(i_status, v8::String::New(itStatus->first));
            }
        }
        result_printer->Set(v8::String::NewSymbol("status"), result_printer_status);
        //LPTSTR               pServerName;
        if(printer->pServerName != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pServerName));
        }
        //LPTSTR               pShareName;
        if(printer->pShareName != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pShareName));
        }
        //LPTSTR               pPortName;
        if(printer->pPortName != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pPortName));
        }
        //LPTSTR               pDriverName;
        if(printer->pDriverName != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pDriverName));
        }
        //LPTSTR               pComment;
        if(printer->pComment != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pComment));
        }
        //LPTSTR               pLocation;
        if(printer->pLocation != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("serverName"), v8::String::New(printer->pLocation));
        }
        //LPTSTR               pSepFile;
        if(printer->pSepFile != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("sepFile"), v8::String::New(printer->pSepFile));
        }
        //LPTSTR               pPrintProcessor;
        if(printer->pPrintProcessor != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("printProcessor"), v8::String::New(printer->pPrintProcessor));
        }
        //LPTSTR               pDatatype;
        if(printer->pDatatype != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("datatype"), v8::String::New(printer->pDatatype));
        }
        //LPTSTR               pParameters;
        if(printer->pParameters != NULL)
        {
            result_printer->Set(v8::String::NewSymbol("parameters"), v8::String::New(printer->pParameters));
        }
        //DWORD                Attributes;
        v8::Local<v8::Array> result_printer_attributes = v8::Array::New();
        int i_attribute = 0;
        for(AttributeMapType::const_iterator itAttribute = getAttributeMap().begin(); itAttribute < getAttributeMap().end(); ++itAttribute, ++i_attribute)
        {
            if(printer->Attributes & itAttribute->second)
            {
                result_printer_attributes.Set(i_attribute, v8::String::New(itAttribute->first));
            }
        }
        result_printer->Set(v8::String::NewSymbol("attributes"), result_printer_attributes);
        //DWORD                Priority;
        result_printer->Set(v8::String::NewSymbol("priority"), v8::Number::New(printer->Priority));
        //DWORD                DefaultPriority;
        result_printer->Set(v8::String::NewSymbol("defaultPriority"), v8::Number::New(printer->DefaultPriority));
        //DWORD                cJobs;
        result_printer->Set(v8::String::NewSymbol("cJobs"), v8::Number::New(printer->cJobs));
        //DWORD                AveragePPM;
        result_printer->Set(v8::String::NewSymbol("averagePPM"), v8::Number::New(printer->AveragePPM));

        //DWORD                StartTime;
        result_printer->Set(v8::String::NewSymbol("startTime"), v8::Date::New(printer->StartTime));
        //DWORD                UntilTime;
        result_printer->Set(v8::String::NewSymbol("untilTime"), v8::Date::New(printer->UntilTime));

        //TODO: to finish to extract all data
        //LPDEVMODE            pDevMode;
        //PSECURITY_DESCRIPTOR pSecurityDescriptor;

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
    //TODO: to move in an unique place win and posix input parameters processing
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
        data.assign(*data_str, data_str.length());
    }
    else if(arg0->IsObject() && arg0.As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())
    {
        data.assign(static_cast<char*>(arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayData()),
                    arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayDataLength());
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string or NativeBuffer")));
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 3, type);

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
                bStatus = WritePrinter( hPrinter, (LPTSTR)(data._c_str()), data.size(), &dwBytesWritten);
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
    return scope.Close(v8::Number::New(dwJob));
}
