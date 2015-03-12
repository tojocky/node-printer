var printer = require("../lib"),
    util = require('util'),
    printers = printer.getPrinters();

printers.forEach(function(iPrinter, i){
    console.log('' + i + 'ppd for printer "' + iPrinter.name + '":' + util.inspect(printer.getPrinterDriverOptions(iPrinter.name), {colors:true, depth:10} ));
    console.log('\tselected page size:'+ printer.getSelectedPaperSize(iPrinter.name) + '\n');
});

