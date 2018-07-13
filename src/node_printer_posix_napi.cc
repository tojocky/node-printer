#include "node_printer_napi.hpp"

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <node_version.h>

#include <cups/cups.h>
#include <cups/ppd.h>

namespace
{
typedef std::map<std::string, int> StatusMapType;
typedef std::map<std::string, std::string> FormatMapType;

} // namespace

Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info))
{

    //This does not return default user printer name according to https://www.cups.org/documentation.php/doc-2.0/api-cups.html#cupsGetDefault2
    //so leave as undefined and JS implementation will loop in all printers
    /*
    const char * printerName = cupsGetDefault();

    // return default printer name only if defined
    if(printerName != NULL) {
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(printerName));
    }
    */
    return info.Env().Null();
}
