#include <node.h>
#include <v8.h>

#include "macros.h"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#pragma  comment(lib, "Winspool.lib")
#endif

// 
// RawDataToPrinter - sends binary data directly to a printer 
//  
// data:        String/NativeBuffer, mandatory, raw data bytes
// printername: String, mandatory, specifying printer name
// docname:     String, mandatory, specifying document name
// type        	String, mandatory, specifying data type. E.G.: RAW, TEXT, ...
// 
// Returns: true for success, false for failure. 
//  
v8::Handle<v8::Value> PrintDirect(const v8::Arguments& args) {
	v8::HandleScope scope;
	REQUIRE_ARGUMENTS(4);
    // can be string or buffer
    if(args.Length()<=0)
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 missing")));
    }
    char* data(NULL);
    size_t data_len(0);
    v8::Handle<v8::Value> arg0(args[0]);
    if(arg0->IsString())
    {
        v8::String::Utf8Value data_str(arg0->ToString());
        data = *data_str;
        data_len = data_str.length();
    }
    else if(arg0->IsObject() && arg0.As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())
    {
        data = static_cast<char*>(arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayData());
        data_len = arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayDataLength();
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string or NativeBuffer")));
    }

	REQUIRE_ARGUMENT_STRING(1, printername);
	REQUIRE_ARGUMENT_STRING(2, docname);
	REQUIRE_ARGUMENT_STRING(3, type);
#if _MSC_VER
	int args_length = args.Length();

	BOOL     bStatus = true;
	HANDLE     hPrinter = NULL;
	DOC_INFO_1 DocInfo;
	DWORD      dwJob = 0L;
	DWORD      dwBytesWritten = 0L;
	
    // Open a handle to the printer. 
    bStatus = OpenPrinter( (LPTSTR)(*printername), &hPrinter, NULL );
    if (bStatus) {
        // Fill in the structure with info about this "document." 
        DocInfo.pDocName = (LPTSTR)(*docname);
        DocInfo.pOutputFile = NULL;
        DocInfo.pDatatype = (LPTSTR)(*type);

        // Inform the spooler the document is beginning. 
        dwJob = StartDocPrinter( hPrinter, 1, (LPBYTE)&DocInfo );
        if (dwJob > 0) {
            // Start a page. 
            bStatus = StartPagePrinter( hPrinter );
            if (bStatus) {
                // Send the data to the printer. 
				//TODO: check with sizeof(LPTSTR) is the same as sizeof(char)
                bStatus = WritePrinter( hPrinter, (LPTSTR)(data), data_len, &dwBytesWritten);
                EndPagePrinter (hPrinter);
            }else{
				RETURN_EXCEPTION_STR("StartPagePrinter error");
            }
            // Inform the spooler that the document is ending. 
            EndDocPrinter( hPrinter );
        }else{
			RETURN_EXCEPTION_STR("StartDocPrinter error");
        }
        // Close the printer handle. 
        ClosePrinter( hPrinter );
    }else{
		RETURN_EXCEPTION(v8::String::Concat(v8::String::New("OpenPrinter error "), args[1]->ToString()))
    }
    // Check to see if correct number of bytes were written. 
    if (!bStatus || (dwBytesWritten != data.length())) {
        bStatus = false;
		RETURN_EXCEPTION_STR("not sent all bytes");
    } else {
        bStatus = true;
    }
	bool ret_sttaus = false||bStatus;
	return scope.Close(v8::Boolean::New(ret_sttaus));
#else
	return scope.Close(v8::Boolean::New(false));
	//#error Wrong OS for compile raw printer
#endif
}

void init(v8::Handle<v8::Object> target) {
// only for node
#if _MSC_VER
  NODE_SET_METHOD(target, "printDirect", PrintDirect);
#endif
}

NODE_MODULE(node_printer, init);
