var printer = require("../lib"),
    util = require('util'),
    printerName = 'Foxit Reader PDF Printer',
    printerFormat = 'TEXT';

printer.printDirect({
    data:"print from Node.JS buffer", // or simple String: "some text"
	printer:printerName, // printer name
	type: printerFormat, // type: RAW, TEXT, PDF, JPEG, .. depends on platform
    options: // supported page sizes may be retrieved using getPrinterDriverOptions, supports CUPS printing options
    {
        media: 'Letter',
        'fit-to-page': true
    },
	success:function(jobID){
		console.log("sent to printer with ID: "+jobID);
        var jobInfo = printer.getJob(printerName, jobID);
        console.log("current job info:"+util.inspect(jobInfo, {depth: 10, colors:true}));
        if(jobInfo.status.indexOf('PRINTED') !== -1)
        {
            console.log('too late, already printed');
            return;
        }
        console.log('cancelling...');
        var is_ok = printer.setJob(printerName, jobID, 'CANCEL');
		console.log("cancelled: "+is_ok);
		try{
			console.log("current job info:"+util.inspect(printer.getJob(printerName, jobID), {depth: 10, colors:true}));
		}catch(err){
			console.log('job deleted. err:'+err);
		}
	},
	error:function(err){console.log(err);}
});
