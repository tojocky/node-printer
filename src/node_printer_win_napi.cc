#include "node_printer_napi.hpp"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#include <Wingdi.h>
#pragma comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <iostream>
#include <string>
#include <cstring>

#include <map>
#include <utility>
#include <sstream>
#include <node_version.h>

namespace
{
typedef std::map<std::string, DWORD> StatusMapType;

/** Memory value class management to avoid memory leak
    */
template <typename Type>
class MemValue : public MemValueBase<Type>
{
  public:
    /** Constructor of allocating iSizeKbytes bytes memory;
        * @param iSizeKbytes size in bytes of required allocating memory
        */
    MemValue(const DWORD iSizeKbytes)
    {
        _value = (Type *)malloc(iSizeKbytes);
    }

    ~MemValue()
    {
        free();
    }

  protected:
    virtual void free()
    {
        if (_value != NULL)
        {
            ::free(_value);
            _value = NULL;
        }
    }
};

struct PrinterHandle
{
    PrinterHandle(LPWSTR iPrinterName)
    {
        _ok = OpenPrinterW(iPrinterName, &_printer, NULL);
    }
    ~PrinterHandle()
    {
        if (_ok)
        {
            ClosePrinter(_printer);
        }
    }
    operator HANDLE() { return _printer; }
    operator bool() { return (!!_ok); }
    HANDLE &operator*() { return _printer; }
    HANDLE *operator->() { return &_printer; }
    const HANDLE &operator->() const { return _printer; }
    HANDLE _printer;
    BOOL _ok;
};

/**
     * Returns last error code and message string
     */
std::string getLastErrorCodeAndMessage()
{
    std::ostringstream s;
    DWORD erroCode = GetLastError();
    s << "code: " << erroCode;
    DWORD retSize;
    LPTSTR pTemp = NULL;
    retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            NULL,
                            erroCode,
                            LANG_NEUTRAL,
                            (LPTSTR)&pTemp,
                            0,
                            NULL);
    if (retSize && pTemp != NULL)
    {
        //pTemp[strlen(pTemp)-2]='\0'; //remove cr and newline character
        //TODO: check if it is needed to convert c string to std::string
        std::string stringMessage(pTemp);
        s << ", message: " << stringMessage;
        LocalFree((HLOCAL)pTemp);
    }

    return s.str();
}

} // namespace

Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info)
{
    // size in chars of the printer name: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144876(v=vs.85).aspx
    DWORD cSize = 0;
    GetDefaultPrinterW(NULL, &cSize);

    if (cSize == 0)
    {
        return Napi::String::New(info.Env(), "");
    }

    MemValue<char16_t>
        bPrinterName(cSize * sizeof(char16_t));
    BOOL res = GetDefaultPrinterW((LPWSTR)(bPrinterName.get()), &cSize);
    if (!res)
    {
        return Napi::String::New(info.Env(), "");
    }

    return Napi::String::New(info.Env(), (char16_t *)bPrinterName.get());
}

Napi::Value PrintDirect(const Napi::CallbackInfo &info)
{
    //TODO: to move in an unique place win and posix input parameters processing
    REQUIRE_ARGUMENTS(info, 5, info.Env());

    // can be string or buffer
    if (info.Length() <= 0)
    {
        Napi::Error::New(info.Env(), "Argument 0 missing").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    std::string data;
    Napi::Value arg0(info.Env(), info[0]);
    if (!getStringOrBufferFromNapiValue(arg0, data))
    {
        Napi::Error::New(info.Env(), "Argument 0 must be a string or Buffer").ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    std::string tempo;
    REQUIRE_ARGUMENT_STRINGW(info, 1, printername, tempo);
    REQUIRE_ARGUMENT_STRINGW(info, 2, docname, tempo);
    REQUIRE_ARGUMENT_STRINGW(info, 3, type, tempo);
    BOOL bStatus = true;

    // Open a handle to the printer.

    PrinterHandle printerHandle((LPWSTR)&printername[0]);
    DOC_INFO_1W DocInfo;
    DWORD dwJob = 0L;
    DWORD dwBytesWritten = 0L;
    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    // Fill in the structure with info about this "document."
    DocInfo.pDocName = (LPWSTR)(&docname[0]);
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = (LPWSTR)(&type[0]);

    // // Inform the spooler the document is beginning.
    dwJob = StartDocPrinterW(*printerHandle, 1, (LPBYTE)&DocInfo);
    if (dwJob > 0)
    {
        // Start a page.
        bStatus = StartPagePrinter(*printerHandle);
        if (bStatus)
        {
            // Send the data to the printer.
            //TODO: check with sizeof(LPTSTR) is the same as sizeof(char)
            bStatus = WritePrinter(*printerHandle, (LPVOID)(data.c_str()), (DWORD)data.size(), &dwBytesWritten);
            EndPagePrinter(*printerHandle);
        }
        else
        {
            std::string error_str("StartPagePrinter error: ");
            error_str += getLastErrorCodeAndMessage();
            RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
        }
        // Inform the spooler that the document is ending.
        EndDocPrinter(*printerHandle);
    }
    else
    {
        std::string error_str("StartDocPrinterW error: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    // Check to see if correct number of bytes were written.
    if (dwBytesWritten != data.size())
    {
        RETURN_EXCEPTION_STR(info.Env(), "not sent all bytes");
    }
    // MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Number, dwJob));
    return Napi::Number::New(info.Env(), dwJob);
}
