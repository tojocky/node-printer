#ifndef NODE_PRINTER_HPP
#define NODE_PRINTER_HPP

#include "macros.hh"

#include <node.h>
#include <v8.h>

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
MY_NODE_MODULE_CALLBACK(PrintDirect);

/** Retrieve all printers and jobs
 * posix: minimum version: CUPS 1.1.21/OS X 10.4
 */
MY_NODE_MODULE_CALLBACK(getPrinters);

/**
 * Return default printer name, if null then default printer is not set
 */
MY_NODE_MODULE_CALLBACK(getDefaultPrinterName);

/** Retrieve printer info and jobs
 * @param printer name String
 */
MY_NODE_MODULE_CALLBACK(getPrinter);

/** Retrieve printer driver info
 * @param printer name String
 */
MY_NODE_MODULE_CALLBACK(getPrinterDriverOptions);

/** Retrieve job info
 *  @param printer name String
 *  @param job id Number
 */
MY_NODE_MODULE_CALLBACK(getJob);

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
MY_NODE_MODULE_CALLBACK(setJob);

/** Get supported print formats for printDirect. It depends on platform
 */
MY_NODE_MODULE_CALLBACK(getSupportedPrintFormats);

/** Get supported job commands for setJob method
 */
MY_NODE_MODULE_CALLBACK(getSupportedJobCommands);

//TODO:
// optional ability to get printer spool
#endif
