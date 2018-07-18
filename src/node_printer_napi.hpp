#ifndef NODE_PRINTER_HPP
#define NODE_PRINTER_HPP

#include "macros_napi.hh"

#include <napi.h>

// #include <node.h>
// #include <v8.h>

#include <string>

/**
 * Send file to printer
 *
 * @param filename String, mandatory, specifying filename to print
 * @param docname String, mandatory, specifying document name
 * @param printer String, mandatory, specifying printer name
 *
 * @returns jobId for success, or error message for failure.
 */
Napi::Value PrintFile(const Napi::CallbackInfo &info);

/**
 * Return default printer name, if null then default printer is not set
 */
Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info);

/** Retrieve all printers and jobs
 * posix: minimum version: CUPS 1.1.21/OS X 10.4
 */
Napi::Value getPrinters(const Napi::CallbackInfo &info);

/**
 * Send data to printer
 *
 * @param data String/NativeBuffer, mandatory, raw data bytes
 * @param printername String, mandatory, specifying printer name
 * @param docname String, mandatory, specifying document name
 * @param type String, mandatory, specifying data type. E.G.: RAW, TEXT, ...
 *
 * @returns true for success, false for failure.
 */
Napi::Value PrintDirect(const Napi::CallbackInfo &info);

/** Get supported job commands for setJob method
 */
Napi::Value getSupportedJobCommands(const Napi::CallbackInfo &info);

// //TODO:
// // optional ability to get printer spool

// // util class

// /** Memory value class management to avoid memory leak
//  * TODO: move to std::unique_ptr on switching to C++11
// */
template <typename Type>
class MemValueBase
{
public:
  MemValueBase() : _value(NULL) {}

  /** Destructor. The allocated memory will be deallocated
    */
  virtual ~MemValueBase() {}

  Type *get() { return _value; }
  Type *operator->() { return &_value; }
  operator bool() const { return (_value != NULL); }

protected:
  Type *_value;

  virtual void free(){};
};

/**
 * try to extract String or buffer from v8 value
 * @param iV8Value - source v8 value
 * @param oData - destination data
 * @return TRUE if value is String or Buffer, FALSE otherwise
 */
bool getStringOrBufferFromNapiValue(Napi::Value iValue, std::string &oData);

#endif
