var printer = require("../lib");

console.log(
  "default printer name with getPrinter(): " +
    (printer.getPrinter().name || "is not defined on your computer")
);
