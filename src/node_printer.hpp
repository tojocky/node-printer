#ifndef NODE_PRINTER_HPP
#define NODE_PRINTER_HPP

#include <node.h>
#include <v8.h>

#include "macros.h"

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
v8::Handle<v8::Value> PrintDirect(const v8::Arguments& iArgs);

/**
 * posix: minimum version: CUPS 1.1.21/OS X 10.4
 */
v8::Handle<v8::Value> getPrinters(const v8::Arguments& iArgs);

/** Get Supported format array depends on the system
 */
v8::Handle<v8::Value> getSupportedFormats(const v8::Arguments& iArgs);

//TODO:
// to see if the printer is already printing
// optional ability to get printer spool
#endif
