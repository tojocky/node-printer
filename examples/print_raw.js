var printer = require("../lib");

printer.printDirect({data:new Buffer("print from Node.JS buffer") // or simple String: "some text"
	, printer:'raw_printer' // printer name
	, type: 'RAW' // type: RAW, TEXT, PDF, JPEG, .. depends on platform
	, success:function(jobID){
		console.log("sent to printer with ID: "+jobID);
	}
	, error:function(err){console.log(err);}
});
