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

/** Retrieve all printers and jobs
 * posix: minimum version: CUPS 1.1.21/OS X 10.4
 */
v8::Handle<v8::Value> getPrinters(const v8::Arguments& iArgs);

/** Retrieve printer info and jobs
 * @param printer name String
 */
v8::Handle<v8::Value> getPrinter(const v8::Arguments& iArgs);

/** Retrieve job info
 *  @param printer name String
 *  @param job id Number
 */
v8::Handle<v8::Value> getJob(const v8::Arguments& iArgs);

//TODO
/** Set job command. 
 * arguments:
 * @param printer name String
 * @param job id Number
 * @param job command String
 * Possible commands:
 *      "CANCEL"
 *      "PAUSE"
 *      "RESTART"
 *      "RESUME"
 *      "DELETE"
 *      "SENT-TO-PRINTER"
 *      "LAST-PAGE-EJECTED"
 *      "RETAIN"
 *      "RELEASE"
 */
v8::Handle<v8::Value> setJob(const v8::Arguments& iArgs);

/** Get supported print formats for printDirect. It depends on platform
 */
v8::Handle<v8::Value> getSupportedPrintFormats(const v8::Arguments& iArgs);

/** Get supported job commands for setJob method
 */
v8::Handle<v8::Value> getSupportedJobCommands(const v8::Arguments& iArgs);

//TODO:
// optional ability to get printer spool
#endif
