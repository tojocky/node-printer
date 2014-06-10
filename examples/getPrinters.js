var printer = require("../lib"),
    util = require('util');
console.log("installed printers:\n"+util.inspect(printer.getPrinters(), {colors:true, depth:10}));
