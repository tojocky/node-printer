exports.testLoadLibrary = function(test) {
  test.doesNotThrow(function(){
    printer = require("../");
  });
  test.done();
}

exports.testGetprinters = function(test) {
  printer = require("../");
  test.equal(typeof(printer.getPrinters()), 'object');
  test.done();
}

// TODO: add more tests
