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
