var printer = require("../lib");

console.log('default printer name: ' + (printer.getDefaultPrinterName() || 'is not defined on your computer'));

