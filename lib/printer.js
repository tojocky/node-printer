var printer_helper = {},
    fs = require("fs"),
    child_process = require("child_process"),
    os = require("os"),
    path = require("path"),
    binding_path = path.resolve(__dirname, './node_printer.node'),
    printer_helper;

if(fs.existsSync(binding_path)) {
    printer_helper = require(binding_path);
} else {
    printer_helper = require('./node_printer_'+process.platform+'_'+process.arch+'.node');
}

/** Return all installed printers including active jobs
 */
module.exports.getPrinters = getPrinters;

/** send data to printer
 */
module.exports.printDirect = printDirect;

/// send file to printer
module.exports.printFile = printFile;

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
module.exports.getPrinterDriverOptions = getPrinterDriverOptions;

/// Return default printer name
module.exports.getDefaultPrinterName = getDefaultPrinterName;

/** get printer job info object
 */
module.exports.getJob = getJob;
module.exports.setJob = setJob;

/**
 * return user defined printer, according to https://www.cups.org/documentation.php/doc-2.0/api-cups.html#cupsGetDefault2 :
 * "Applications should use the cupsGetDests and cupsGetDest functions to get the user-defined default printer,
 * as this function does not support the lpoptions-defined default printer"
 */
function getDefaultPrinterName() {
  var printerName = printer_helper.getDefaultPrinterName();
  if(printerName) {
    return printerName;
  }

  // seems correct posix behaviour
  var printers= getPrinters();
  if(printers && printers.length){
    var i = printers.length;
    for(i in printers) {
        var printer = printers[i];
        if(printer.isDefault === true) {
            return printer.name;
        }
    }
  }

  // printer not found, return nothing(undefined)
}

/** Get printer info with jobs
 * @param printerName printer name to extract the info
 * @return printer object info:
 *		TODO: to enum all possible attributes
 */
function getPrinter(printerName)
{
    if(!printerName) {
        printerName = getDefaultPrinterName();
    }
    var printer = printer_helper.getPrinter(printerName);
    correctPrinterinfo(printer);
    return printer;
}

/** Get printer driver options includes advanced options like supported paper size
 * @param printerName printer name to extract the info (default printer used if printer is not provided)
 * @return printer driver info:
 */
function getPrinterDriverOptions(printerName)
{
    if(!printerName) {
        printerName = getDefaultPrinterName();
    }

    return printer_helper.getPrinterDriverOptions(printerName);
}

/** Finds selected paper size pertaining to the specific printer out of all supported ones in driver_options
 * @param printerName printer name to extract the info (default printer used if printer is not provided)
 * @return selected paper size
 */
function getSelectedPaperSize(printerName){
    var driver_options = getPrinterDriverOptions(printerName);
    var selectedSize = "";
    if (driver_options && driver_options.PageSize) {
        Object.keys(driver_options.PageSize).forEach(function(key){
            if (driver_options.PageSize[key])
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
    if(printers && printers.length){
        var i = printers.length;
        for(i in printers){
            correctPrinterinfo(printers[i]);
        }
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
 options - JS object with CUPS options, optional
 success - Function, optional, callback function
 error - Function, optional, callback function if exists any error

 or

 data - String, mandatory, data to printer
 printer - String, optional, name of the printer, if missing, will try to print to default printer
 docname - String, optional, name of document showed in printer status
 type - String, optional, data type, one of the RAW, TEXT
 options - JS object with CUPS options, optional
 success - Function, optional, callback function with first argument job_id
 error - Function, optional, callback function if exists any error
 */
function printDirect(parameters){
    var data = parameters
        , printer
        , docname
        , type
        , options
        , success
        , error;

    if(arguments.length==1){
        //TODO: check parameters type
        //if (typeof parameters )
        data = parameters.data;
        printer = parameters.printer;
        docname = parameters.docname;
        type = parameters.type;
        options = parameters.options||{};
        success = parameters.success;
        error = parameters.error;
    }else{
        printer = arguments[1];
        type = arguments[2];
        docname = arguments[3];
        options = arguments[4];
        success = arguments[5];
        error = arguments[6];
    }

    if(!type){
        type = "RAW";
    }

    // Set default printer name
    if(!printer) {
        printer = getDefaultPrinterName();
    }

    type = type.toUpperCase();

    if(!docname){
        docname = "node print job";
    }

    if (!options){
        options = {};
    }

    //TODO: check parameters type
    if(printer_helper.printDirect){// call C++ binding
        try{
            var res = printer_helper.printDirect(data, printer, docname, type, options);
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

/**
parameters:
   parameters - Object, parameters objects with the following structure:
      filename - String, mandatory, data to printer
      docname - String, optional, name of document showed in printer status
      printer - String, optional, mane of the printer, if missed, will try to retrieve the default printer name
      success - Function, optional, callback function
      error - Function, optional, callback function if exists any error
*/
function printFile(parameters){
    var filename,
        docname,
        printer,
        options,
        success,
        error;

    if((arguments.length !== 1) || (typeof(parameters) !== 'object')){
        throw new Error('must provide arguments object');
    }

    filename = parameters.filename;
    docname = parameters.docname;
    printer = parameters.printer;
    options = parameters.options || {};
    success = parameters.success;
    error = parameters.error;

    if(!success){
        success = function(){};
    }

    if(!error){
        error = function(err){
            throw err;
        };
    }

    if(!filename){
        var err = new Error('must provide at least a filename');
        return error(err);
    }

    // try to define default printer name
    if(!printer) {
        printer = getDefaultPrinterName();
    }

    if(!printer) {
        return error(new Error('Printer parameter of default printer is not defined'));
    }

    // set filename if docname is missing
    if(!docname){
        docname = filename;
    }

    //TODO: check parameters type
    if(printer_helper.printFile){// call C++ binding
        try{
            // TODO: proper success/error callbacks from the extension
            var res = printer_helper.printFile(filename, docname, printer, options);

            if(!isNaN(parseInt(res))) {
                success(res);
            } else {
                error(Error(res));
            }
        } catch (e) {
            error(e);
        }
    } else {
        error("Not supported");
    }
}
