const assert = require("assert");
exports.testLoadLibrary = function(test) {
  test.doesNotThrow(function() {
    printer = require("../");
  });
  test.done();
};

exports.testGetprinters = function(test) {
  printer = require("../");
  try {
    test.equal(typeof printer.getPrinters(), "object");
  } catch (e) {
    test.equal(
      e.message,
      "Error on EnumPrinters: code: 1722, message: The RPC server is unavailable."
    );
  }
  test.done();
};

exports.testGetDefaultPrinterName = function(test) {
  printer = require("../");
  test.equal(
    assert(
      typeof printer.getDefaultPrinterName() === "undefined" ||
        typeof printer.getDefaultPrinterName() === "string"
    )
  );
  test.done();
};

exports.testGetSupportedJobCommands = function(test) {
  printer = require("../");
  test.equal(typeof printer.getSupportedJobCommands(), "object");
  test.done();
};

// TODO: add more tests
