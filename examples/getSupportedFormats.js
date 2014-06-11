var printer = require("../lib"),
    util = require('util');
console.log("supported formats are:\n"+util.inspect(printer.getSupportedFormats(), {colors:true, depth:10}));
