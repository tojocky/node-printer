var printer = require("../lib");
printer.printDirect({data:"print from Node.JS"
	, printer:"Foxit PDF Printer"
	, type: "TEXT"
	, success:function(){
		console.log("ok");
	}
	, error:function(err){console.log(err);}
});