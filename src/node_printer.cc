#include "node_printer.hpp"

void initNode(v8::Handle<v8::Object> exports) {
// only for node
  NODE_SET_METHOD(exports, "getPrinters", getPrinters);
  NODE_SET_METHOD(exports, "getDefaultPrinterName", getDefaultPrinterName);
  NODE_SET_METHOD(exports, "getPrinter", getPrinter);
  NODE_SET_METHOD(exports, "getPrinterDriverOptions", getPrinterDriverOptions);
  NODE_SET_METHOD(exports, "getJob", getJob);
  NODE_SET_METHOD(exports, "setJob", setJob);
  NODE_SET_METHOD(exports, "printDirect", PrintDirect);
  NODE_SET_METHOD(exports, "getSupportedPrintFormats", getSupportedPrintFormats);
  NODE_SET_METHOD(exports, "getSupportedJobCommands", getSupportedJobCommands);
  
}

NODE_MODULE(node_printer, initNode);
