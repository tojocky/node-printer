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

/**
 * Send file to printer
 *
 * @param filename String, mandatory, specifying filename to print
 * @param docname String, mandatory, specifying document name
 * @param printer String, mandatory, specifying printer name
 *
 * @returns jobId for success, or error message for failure.
 */
MY_NODE_MODULE_CALLBACK(PrintFile);

MY_NODE_MODULE_CALLBACK(preparePrintStream);
MY_NODE_MODULE_CALLBACK(writeStreamChunk);
MY_NODE_MODULE_CALLBACK(finishPrintStream);

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


// util class

/** Memory value class management to avoid memory leak
*/
template<typename Type>
class MemValueBase
{
public:
    MemValueBase(): _value(NULL) {}

    /** Destructor. The allocated memory will be deallocated
    */
    ~MemValueBase() {
        free();
    }

    Type * get() {return _value; }
    Type * operator ->() { return &_value; }
    operator bool() const { return (_value != NULL); }
protected:
    Type *_value;

    virtual void free() {};
};


#endif
