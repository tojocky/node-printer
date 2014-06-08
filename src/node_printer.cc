#include "node_printer.hpp"

void initNode(v8::Handle<v8::Object> exports) {
// only for node
  NODE_SET_METHOD(exports, "getPrinters", getPrinters);
  NODE_SET_METHOD(exports, "printDirect", PrintDirect);
  NODE_SET_METHOD(exports, "getSupportedFormats", getSupportedFormats);
}

NODE_MODULE(node_printer, initNode);
