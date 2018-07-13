#ifndef NODE_PRINTER_HPP
#define NODE_PRINTER_HPP

#include "macros_napi.hh"

#include <napi.h>

#include <node.h>
#include <v8.h>

#include <string>

/**
 * Return default printer name, if null then default printer is not set
 */
Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info);

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
bool getStringOrBufferFromV8Value(v8::Handle<v8::Value> iV8Value, std::string &oData);

#endif
