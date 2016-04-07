// Windows does not support PDF formats, but you can use imagemagick-native to achieve conversion from PDF to EMF.

var printer = require("../lib"),
    imagemagick, // will be loaded later with proper error.
    fs = require('fs'),
    filename = process.argv[2],
    printername = process.argv[2];

if(process.platform !== 'win32') {
    throw 'This application can be run only on win32 as a demo of print PDF image'
}

if(!filename) {
    throw 'PDF file name is missing. Please use the following params: <filename> [printername]'
}

try {
    imagemagick = require('imagemagick-native');
} catch(e) {
    throw 'please install imagemagick-native: `npm install imagemagick-native`'
}

var data = fs.readFileSync(filename);

console.log('data: ' + data.toString().substr(0, 20));

//console.log(imagemagick.identify({srcData: data}));

// First convert PDF into
imagemagick.convert({
    srcData: data,
    srcFormat: 'PDF',
    format: 'EMF',
}, function(err, buffer) {
    if (err) {
        throw 'something went wrong on converting to EMF: ' + err;
    }

    // Now we have EMF file, send it to printer as EMF format
    printer.printDirect({
        data: buffer,
        type: 'EMF',
        success: function(id) {
            console.log('printed with id ' + id);
        },
        error: function(err) {
            console.error('error on printing: ' + err);
        }
    })
})


