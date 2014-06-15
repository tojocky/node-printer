var printer = require("../lib"),
    util = require('util'),
    printerName = 'raw_printer',
    printerFormat = 'RAW';

printer.printDirect({
    data:"print from Node.JS buffer", // or simple String: "some text"
	printer:printerName, // printer name
	type: 'RAW', // type: RAW, TEXT, PDF, JPEG, .. depends on platform
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
        console.log("cancelled: "+is_ok+", current job info:"+util.inspect(printer.getJob(printerName, jobID), {depth: 10, colors:true}));
	},
	error:function(err){console.log(err);}
});

