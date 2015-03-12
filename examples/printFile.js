var printer = require("../lib"),
    filename = __filename;

console.log('try to print file: ' + filename);

printer.printFile({filename:filename
	, printer:'HP_LaserJet_Pro_MFP_M125nw' // printer name, if missing then will print to default printer
	, success:function(jobID){
		console.log("sent to printer with ID: "+jobID);
	}
	, error:function(err){console.log(err);}
});

