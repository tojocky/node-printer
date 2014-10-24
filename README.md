node-printer
============

This package binds printers on POSIX and Windows OS from Node.js.

### Reason:

I was involved in a project where I need to print from Node.JS. This is the reason why I created this project and I want to share my code with others.


### Features:

* no dependecies;
* native method wrappers from Windows  and POSIX (which uses [CUPS 1.4/MAC OS X 10.6](http://cups.org/)) APIs;
* compatible with node v0.8.x, 0.9.x and v0.11.x (with 0.11.9 and 0.11.13);
* compatible with node-webkit v0.8.x and 0.9.2;
* ```getPrinters()``` to enumerate all installed printers with current jobs and statuses;
* ```getPrinter(printerName)``` to get a specific printer info with current jobs and statuses;
* ```printDirect(stringData|bufferData, printerName, format, docname, success, error)``` to send a job to a printer;
* ```getSupportedPrintFormats()``` to get all posibilbe print formats for printDirect method which depends on OS. ```RAW``` and ```TEXT``` are supported from all OS-es;
* ```getJob(printerName, jobId)``` to get a specific job info including job status;
* ```setJob(printerName, jobId, command)``` to send a command to a job (e.g. ```'CANCEL'``` to cancel the job);
* ```getSupportedJobCommands()``` to get supported job commands for setJob() depends on OS. ```'CANCEL'``` command is supported from all OS-es.


### How to install:

    npm install printer

or [direct from git](https://www.npmjs.org/doc/cli/npm-install.html):

    npm install git+https://github.com/tojocky/node-printer.git
    
### How to use:

See examples

### Author(s):

* Ion Lupascu, ionlupascu@gmail.com

### Contibutors:

Feel free to download, test and propose new futures
