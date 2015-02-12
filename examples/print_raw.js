var printer = require("../lib");

printer.printDirect({data:"print from Node.JS buffer" // or simple String: "some text"
	//, printer:'Foxit Reader PDF Printer' // printer name, if missing then will print to default printer
	, type: 'RAW' // type: RAW, TEXT, PDF, JPEG, .. depends on platform
	, success:function(jobID){
		console.log("sent to printer with ID: "+jobID);
	}
	, error:function(err){console.log(err);}
});
