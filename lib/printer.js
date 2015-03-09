var printer_helper = {}
    fs = require("fs")
    child_process = require("child_process")
    os = require("os")
    path = require("path"),
    native_lib_path = path.join(__dirname, '../build/Release/node_printer.node'),
    printer_helper;

if(fs.existsSync(native_lib_path)) {
    printer_helper = require(native_lib_path);
} else {
    printer_helper = require('./node_printer_'+process.platform+'_'+process.arch+'.node');
}

/** Return all installed printers including active jobs
 */
module.exports.getPrinters = getPrinters;

/** send data to printer
 */
module.exports.printDirect = printDirect

/** Get supported print format for printDirect
 */
module.exports.getSupportedPrintFormats = printer_helper.getSupportedPrintFormats;

/**
 * Get possible job command for setJob. It depends on os.
 * @return Array of string. e.g.: DELETE, PAUSE, RESUME
 */
module.exports.getSupportedJobCommands = printer_helper.getSupportedJobCommands;

/** get printer info object. It includes all active jobs
 */
module.exports.getPrinter = getPrinter;
module.exports.getSelectedPaperSize = getSelectedPaperSize;

/// Return default printer name
module.exports.getDefaultPrinterName = printer_helper.getDefaultPrinterName;

/** get printer job info object
 */
module.exports.getJob = getJob;
module.exports.setJob = setJob;

/** Get printer info with jobs
 * @param printerName printer name to extract the info
 * @return printer object info:
 *		TODO: to enum all possible attributes
 */
function getPrinter(printerName)
{
    if(!printerName) {
        printerName = printer_helper.getDefaultPrinterName();
    }

	return correctPrinterinfo(printer_helper.getPrinter(printerName));
}

// Finds selected paper size pertaining to the specific printer out of all supported ones in driver_options
function getSelectedPaperSize(printerName){
    var printer = getPrinter(printerName);
    var selectedSize = "";
    if (printer.driver_options && printer.driver_options.PageSize) {
        Object.keys(printer.driver_options.PageSize).forEach(function(key){
            if (printer.driver_options.PageSize[key] === "true")
                selectedSize = key;
        });
    }
    return selectedSize;
}

function getJob(printerName, jobId)
{
	return printer_helper.getJob(printerName, jobId);
}

function setJob(printerName, jobId, command)
{
	return printer_helper.setJob(printerName, jobId, command);
}

function getPrinters(){
    var printers = printer_helper.getPrinters();
    for(i in printers){
        correctPrinterinfo(printers[i]);
    }
    return printers;
}

function correctPrinterinfo(printer) {
    if(printer.status || !printer.options || !printer.options['printer-state']){
        return;
    }

    var status = printer.options['printer-state'];
    // Add posix status
    if(status == '3'){
        status = 'IDLE'
    }
    else if(status == '4'){
        status = 'PRINTING'
    }
    else if(status == '5'){
        status = 'STOPPED'
    }

    // correct date type
    var k;
    for(k in printer.options) {
        if(/time$/.test(k) && printer.options[k] && !(printer.options[k] instanceof Date)) {
            printer.options[k] = new Date(printer.options[k] * 1000);
        }
    }

    printer.status = status;
}

/*
print raw data. This function is intend to be asynchronous

parameters:
   parameters - Object, parameters objects with the following structure:
      data - String, mandatory, data to printer
      printer - String, optional, name of the printer, if missing, will try to print to default printer
      docname - String, optional, name of document showed in printer status
      type - String, optional, only for wind32, data type, one of the RAW, TEXT
      media - String, optional, page size, e.g. A4, Letter or other
      fit_to_page - Bool, optional, used with `media` and if true scales the document to fit page
      success - Function, optional, callback function
      error - Function, optional, callback function if exists any error

   or

   data - String, mandatory, data to printer
   printer - String, optional, name of the printer, if missing, will try to print to default printer
   docname - String, optional, name of document showed in printer status
   type - String, optional, data type, one of the RAW, TEXT
   media - String, optional, page size, e.g. A4, Letter or other
   fit_to_page - Bool, optional, used with `media` and if true scales the document to fit page
   success - Function, optional, callback function with first argument job_id
   error - Function, optional, callback function if exists any error
*/
function printDirect(parameters){
    var data = parameters
        , printer
        , docname
        , type
        , media
        , fit_to_page
        , success
        , error;

    if(arguments.length==1){
        //TODO: check parameters type
        //if (typeof parameters )
        data = parameters.data;
        printer = parameters.printer;
        docname = parameters.docname;
        type = parameters.type;
        media = parameters.media;
        fit_to_page = parameters.fit_to_page;
        success = parameters.success;
        error = parameters.error;
    }else{
        printer = arguments[1];
        type = arguments[2];
        docname = arguments[3];
        media = arguments[4];
        fit_to_page = arguments[5];
        success = arguments[6];
        error = arguments[7];
    }

   if(!type){
      type = "RAW";
   }

   // Set default printer name
   if(!printer) {
       printer = printer_helper.getDefaultPrinterName();
   }

    type = type.toUpperCase();

   if(!docname){
      docname = "node print job";
   }

   if (!media){
       media = "";
   }

   if (fit_to_page && fit_to_page !== "false")
       fit_to_page = "true";
   else
       fit_to_page = "false";

   //TODO: check parameters type
   if(printer_helper.printDirect){// call C++ binding
      try{
         var res = printer_helper.printDirect(data, printer, docname, type, media, fit_to_page, success, error);
         if(res){
            success(res);
         }else{
            error(Error("Something wrong in printDirect"));
         }
      }catch (e){
         error(e);
      }
    }else{
      error("Not supported");
   }
}
