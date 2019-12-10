#include "node_printer.hpp"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#include <Wingdi.h>
#pragma  comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <node_version.h>

namespace{
    typedef std::map<std::string, DWORD> StatusMapType;

    /** Memory value class management to avoid memory leak
    */
    template<typename Type>
    class MemValue: public MemValueBase<Type> {
    public:
        /** Constructor of allocating iSizeKbytes bytes memory;
        * @param iSizeKbytes size in bytes of required allocating memory
        */
        MemValue(const DWORD iSizeKbytes) {
            _value = (Type*)malloc(iSizeKbytes);
        }
		
        ~MemValue () {
            free();
        }
    protected:
        virtual void free() {
            if(_value != NULL)
            {
                ::free(_value);
                _value = NULL;
            }
        }
    };

    struct PrinterHandle
    {
        PrinterHandle(LPWSTR iPrinterName)
        {
            _ok = OpenPrinterW(iPrinterName, &_printer, NULL);
        }
        ~PrinterHandle()
        {
            if(_ok)
            {
                ClosePrinter(_printer);
            }
        }
        operator HANDLE() {return _printer;}
        operator bool() { return (!!_ok);}
        HANDLE & operator *() { return _printer;}
        HANDLE * operator ->() { return &_printer;}
        const HANDLE & operator ->() const { return _printer;}
        HANDLE _printer;
        BOOL _ok;
    };

    const StatusMapType& getStatusMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define STATUS_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        STATUS_PRINTER_ADD("BUSY", PRINTER_STATUS_BUSY);
        STATUS_PRINTER_ADD("DOOR-OPEN", PRINTER_STATUS_DOOR_OPEN);
        STATUS_PRINTER_ADD("ERROR", PRINTER_STATUS_ERROR);
        STATUS_PRINTER_ADD("INITIALIZING", PRINTER_STATUS_INITIALIZING);
        STATUS_PRINTER_ADD("IO-ACTIVE", PRINTER_STATUS_IO_ACTIVE);
        STATUS_PRINTER_ADD("MANUAL-FEED", PRINTER_STATUS_MANUAL_FEED);
        STATUS_PRINTER_ADD("NO-TONER", PRINTER_STATUS_NO_TONER);
        STATUS_PRINTER_ADD("NOT-AVAILABLE", PRINTER_STATUS_NOT_AVAILABLE);
        STATUS_PRINTER_ADD("OFFLINE", PRINTER_STATUS_OFFLINE);
        STATUS_PRINTER_ADD("OUT-OF-MEMORY", PRINTER_STATUS_OUT_OF_MEMORY);
        STATUS_PRINTER_ADD("OUTPUT-BIN-FULL", PRINTER_STATUS_OUTPUT_BIN_FULL);
        STATUS_PRINTER_ADD("PAGE-PUNT", PRINTER_STATUS_PAGE_PUNT);
        STATUS_PRINTER_ADD("PAPER-JAM", PRINTER_STATUS_PAPER_JAM);
        STATUS_PRINTER_ADD("PAPER-OUT", PRINTER_STATUS_PAPER_OUT);
        STATUS_PRINTER_ADD("PAPER-PROBLEM", PRINTER_STATUS_PAPER_PROBLEM);
        STATUS_PRINTER_ADD("PAUSED", PRINTER_STATUS_PAUSED);
        STATUS_PRINTER_ADD("PENDING-DELETION", PRINTER_STATUS_PENDING_DELETION);
        STATUS_PRINTER_ADD("POWER-SAVE", PRINTER_STATUS_POWER_SAVE);
        STATUS_PRINTER_ADD("PRINTING", PRINTER_STATUS_PRINTING);
        STATUS_PRINTER_ADD("PROCESSING", PRINTER_STATUS_PROCESSING);
        STATUS_PRINTER_ADD("SERVER-UNKNOWN", PRINTER_STATUS_SERVER_UNKNOWN);
        STATUS_PRINTER_ADD("TONER-LOW", PRINTER_STATUS_TONER_LOW);
        STATUS_PRINTER_ADD("USER-INTERVENTION", PRINTER_STATUS_USER_INTERVENTION);
        STATUS_PRINTER_ADD("WAITING", PRINTER_STATUS_WAITING);
        STATUS_PRINTER_ADD("WARMING-UP", PRINTER_STATUS_WARMING_UP);
#undef STATUS_PRINTER_ADD
        return result;
    }

    const StatusMapType& getJobStatusMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define STATUS_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        // Common statuses
        STATUS_PRINTER_ADD("PRINTING", JOB_STATUS_PRINTING);
        STATUS_PRINTER_ADD("PRINTED", JOB_STATUS_PRINTED);
        STATUS_PRINTER_ADD("PAUSED", JOB_STATUS_PAUSED);

        // Specific statuses
        STATUS_PRINTER_ADD("BLOCKED-DEVQ", JOB_STATUS_BLOCKED_DEVQ);
        STATUS_PRINTER_ADD("DELETED", JOB_STATUS_DELETED);
        STATUS_PRINTER_ADD("DELETING", JOB_STATUS_DELETING);
        STATUS_PRINTER_ADD("ERROR", JOB_STATUS_ERROR);
        STATUS_PRINTER_ADD("OFFLINE", JOB_STATUS_OFFLINE);
        STATUS_PRINTER_ADD("PAPEROUT", JOB_STATUS_PAPEROUT);
        STATUS_PRINTER_ADD("RESTART", JOB_STATUS_RESTART);
        STATUS_PRINTER_ADD("SPOOLING", JOB_STATUS_SPOOLING);
        STATUS_PRINTER_ADD("USER-INTERVENTION", JOB_STATUS_USER_INTERVENTION);
        // XP and later
#ifdef JOB_STATUS_COMPLETE
        STATUS_PRINTER_ADD("COMPLETE", JOB_STATUS_COMPLETE);
#endif
#ifdef JOB_STATUS_RETAINED
        STATUS_PRINTER_ADD("RETAINED", JOB_STATUS_RETAINED);
#endif

#undef STATUS_PRINTER_ADD
        return result;
    }

    const StatusMapType& getAttributeMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define ATTRIBUTE_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        ATTRIBUTE_PRINTER_ADD("DIRECT", PRINTER_ATTRIBUTE_DIRECT);
        ATTRIBUTE_PRINTER_ADD("DO-COMPLETE-FIRST", PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST);
        ATTRIBUTE_PRINTER_ADD("ENABLE-DEVQ", PRINTER_ATTRIBUTE_ENABLE_DEVQ);
        ATTRIBUTE_PRINTER_ADD("HIDDEN", PRINTER_ATTRIBUTE_HIDDEN);
        ATTRIBUTE_PRINTER_ADD("KEEPPRINTEDJOBS", PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS);
        ATTRIBUTE_PRINTER_ADD("LOCAL", PRINTER_ATTRIBUTE_LOCAL);
        ATTRIBUTE_PRINTER_ADD("NETWORK", PRINTER_ATTRIBUTE_NETWORK);
        ATTRIBUTE_PRINTER_ADD("PUBLISHED", PRINTER_ATTRIBUTE_PUBLISHED);
        ATTRIBUTE_PRINTER_ADD("QUEUED", PRINTER_ATTRIBUTE_QUEUED);
        ATTRIBUTE_PRINTER_ADD("RAW-ONLY", PRINTER_ATTRIBUTE_RAW_ONLY);
        ATTRIBUTE_PRINTER_ADD("SHARED", PRINTER_ATTRIBUTE_SHARED);
        ATTRIBUTE_PRINTER_ADD("OFFLINE", PRINTER_ATTRIBUTE_WORK_OFFLINE);
        // XP
#ifdef PRINTER_ATTRIBUTE_FAX
        ATTRIBUTE_PRINTER_ADD("FAX", PRINTER_ATTRIBUTE_FAX);
#endif
        // vista
#ifdef PRINTER_ATTRIBUTE_FRIENDLY_NAME
        ATTRIBUTE_PRINTER_ADD("FRIENDLY-NAME", PRINTER_ATTRIBUTE_FRIENDLY_NAME);
        ATTRIBUTE_PRINTER_ADD("MACHINE", PRINTER_ATTRIBUTE_MACHINE);
        ATTRIBUTE_PRINTER_ADD("PUSHED-USER", PRINTER_ATTRIBUTE_PUSHED_USER);
        ATTRIBUTE_PRINTER_ADD("PUSHED-MACHINE", PRINTER_ATTRIBUTE_PUSHED_MACHINE);
#endif
        // server 2003
#ifdef PRINTER_ATTRIBUTE_TS
        ATTRIBUTE_PRINTER_ADD("TS", PRINTER_ATTRIBUTE_TS);
#endif
#undef ATTRIBUTE_PRINTER_ADD
        return result;
    }

    const StatusMapType& getJobCommandMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define COMMAND_JOB_ADD(value, type) result.insert(std::make_pair(value, type))
        COMMAND_JOB_ADD("CANCEL", JOB_CONTROL_CANCEL);
        COMMAND_JOB_ADD("PAUSE", JOB_CONTROL_PAUSE);
        COMMAND_JOB_ADD("RESTART", JOB_CONTROL_RESTART);
        COMMAND_JOB_ADD("RESUME", JOB_CONTROL_RESUME);
        COMMAND_JOB_ADD("DELETE", JOB_CONTROL_DELETE);
        COMMAND_JOB_ADD("SENT-TO-PRINTER", JOB_CONTROL_SENT_TO_PRINTER);
        COMMAND_JOB_ADD("LAST-PAGE-EJECTED", JOB_CONTROL_LAST_PAGE_EJECTED);
#ifdef JOB_CONTROL_RETAIN
        COMMAND_JOB_ADD("RETAIN", JOB_CONTROL_RETAIN);
#endif
#ifdef JOB_CONTROL_RELEASE
        COMMAND_JOB_ADD("RELEASE", JOB_CONTROL_RELEASE);
#endif
#undef COMMAND_JOB_ADD
        return result;
    }

    void parseJobObject(JOB_INFO_2W *job, v8::Local<v8::Object> result_printer_job)
    {
        MY_NODE_MODULE_ISOLATE_DECL
        //Common fields
        //DWORD                JobId;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "id", V8_VALUE_NEW(Number, job->JobId));
#define ADD_V8_STRING_PROPERTY(name, key) if((job->##key != NULL) && (*job->##key != L'\0'))    \
        {                                   \
            MY_NODE_SET_OBJECT_PROP(result_printer_job, #name, V8_STRING_NEW_2BYTES((uint16_t*)job->##key)); \
        }
        //LPTSTR               pPrinterName;
        ADD_V8_STRING_PROPERTY(name, pPrinterName)
        //LPTSTR               pPrinterName;
        ADD_V8_STRING_PROPERTY(printerName, pPrinterName);
        //LPTSTR               pUserName;
        ADD_V8_STRING_PROPERTY(user, pUserName);
        //LPTSTR               pDatatype;
        ADD_V8_STRING_PROPERTY(format, pDatatype);
        //DWORD                Priority;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "priority", V8_VALUE_NEW(Number, job->Priority));
        //DWORD                Size;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "size", V8_VALUE_NEW(Number, job->Size));
        //DWORD                Status;
        v8::Local<v8::Array> result_printer_job_status = V8_VALUE_NEW_DEFAULT(Array);
        int i_status = 0;
        for(StatusMapType::const_iterator itStatus = getJobStatusMap().begin(); itStatus != getJobStatusMap().end(); ++itStatus)
        {
            if(job->Status & itStatus->second)
            {
                MY_NODE_SET_OBJECT(result_printer_job_status, i_status++, V8_STRING_NEW_UTF8(itStatus->first.c_str()));
            }
        }
        //LPTSTR               pStatus;
        if((job->pStatus != NULL) && (*job->pStatus != L'\0'))
        {
            MY_NODE_SET_OBJECT(result_printer_job_status, i_status++, V8_STRING_NEW_2BYTES((uint16_t*)job->pStatus));
        }
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "status", result_printer_job_status);

        // Specific fields
        //LPTSTR               pMachineName;
        ADD_V8_STRING_PROPERTY(machineName, pMachineName);
        //LPTSTR               pDocument;
        ADD_V8_STRING_PROPERTY(document, pDocument);
        //LPTSTR               pNotifyName;
        ADD_V8_STRING_PROPERTY(notifyName, pNotifyName);
        //LPTSTR               pPrintProcessor;
        ADD_V8_STRING_PROPERTY(printProcessor, pPrintProcessor);
        //LPTSTR               pParameters;
        ADD_V8_STRING_PROPERTY(parameters, pParameters);
        //LPTSTR               pDriverName;
        ADD_V8_STRING_PROPERTY(driverName, pDriverName);
#undef ADD_V8_STRING_PROPERTY
        //LPDEVMODE            pDevMode;
        //PSECURITY_DESCRIPTOR pSecurityDescriptor;
        //DWORD                Position;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "position", V8_VALUE_NEW(Number, job->Position));
        //DWORD                StartTime;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "startTime", V8_VALUE_NEW(Number, job->StartTime));
        //DWORD                UntilTime;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "untilTime", V8_VALUE_NEW(Number, job->UntilTime));
        //DWORD                TotalPages;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "totalPages", V8_VALUE_NEW(Number, job->TotalPages));
        //SYSTEMTIME           Submitted;
        //DWORD                Time;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "time", V8_VALUE_NEW(Number, job->Time));
        //DWORD                PagesPrinted;
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "pagesPrinted", V8_VALUE_NEW(Number, job->PagesPrinted));
    }

    /**
     * Returns last error code and message string
     */
    std::string getLastErrorCodeAndMessage() {
    	std::ostringstream s;
    	DWORD erroCode = GetLastError();
    	s << "code: " << erroCode;
    	DWORD retSize;
    	LPTSTR pTemp = NULL;
    	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                FORMAT_MESSAGE_FROM_SYSTEM|
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL,
                                erroCode,
                                LANG_NEUTRAL,
                                (LPTSTR)&pTemp,
                                0,
                                NULL );
        if (retSize && pTemp != NULL) {
	    //pTemp[strlen(pTemp)-2]='\0'; //remove cr and newline character
	    //TODO: check if it is needed to convert c string to std::string
	    std::string stringMessage(pTemp);
	    s << ", message: " << stringMessage;
	    LocalFree((HLOCAL)pTemp);
	}

    	return s.str();
    }

    std::string retrieveAndParseJobs(const LPWSTR iPrinterName,
                                     const DWORD& iTotalJobs,
                                     v8::Local<v8::Object> result_printer_jobs,
                                     PrinterHandle& iPrinterHandle)
    {
        MY_NODE_MODULE_ISOLATE_DECL
        DWORD bytes_needed = 0, totalJobs = 0;
        BOOL bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, NULL, bytes_needed, &bytes_needed, &totalJobs);
        MemValue<JOB_INFO_2W> jobs(bytes_needed);
        if(!jobs)
        {
            std::string error_str("Error on allocating memory for jobs: ");
            error_str += getLastErrorCodeAndMessage();
            v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
            MY_NODE_SET_OBJECT_PROP(result_printer_job, "error", V8_STRING_NEW_UTF8(error_str.c_str()));
            MY_NODE_SET_OBJECT(result_printer_jobs, 0, result_printer_job);
            return std::string("");
        }
        DWORD dummy_bytes = 0;
        bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, (LPBYTE)jobs.get(), bytes_needed, &dummy_bytes, &totalJobs);
        if(!bError)
        {
            std::string error_str("Error on EnumJobsW: ");
            error_str += getLastErrorCodeAndMessage();
            v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
            MY_NODE_SET_OBJECT_PROP(result_printer_job, "error", V8_STRING_NEW_UTF8(error_str.c_str()));
            MY_NODE_SET_OBJECT(result_printer_jobs, 0, result_printer_job);
            return std::string("");
        }
        JOB_INFO_2W *job = jobs.get();
        for(DWORD i = 0; i < totalJobs; ++i, ++job)
        {
            v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
            parseJobObject(job, result_printer_job);
            MY_NODE_SET_OBJECT(result_printer_jobs, i, result_printer_job);
        }
        return std::string("");
    }

    std::string parsePrinterInfo(const PRINTER_INFO_2W *printer, v8::Local<v8::Object> result_printer, PrinterHandle& iPrinterHandle)
    {
        MY_NODE_MODULE_ISOLATE_DECL
    #define ADD_V8_STRING_PROPERTY(name, key) if((printer->##key != NULL) && (*printer->##key != L'\0'))    \
        {                                   \
            MY_NODE_SET_OBJECT_PROP(result_printer, #name, V8_STRING_NEW_2BYTES((uint16_t*)printer->##key)); \
        }
        //LPTSTR               pPrinterName;
        ADD_V8_STRING_PROPERTY(name, pPrinterName)
        //LPTSTR               pServerName;
        ADD_V8_STRING_PROPERTY(serverName, pServerName)
        //LPTSTR               pShareName;
        ADD_V8_STRING_PROPERTY(shareName, pShareName)
        //LPTSTR               pPortName;
        ADD_V8_STRING_PROPERTY(portName, pPortName)
        //LPTSTR               pDriverName;
        ADD_V8_STRING_PROPERTY(driverName, pDriverName)
        //LPTSTR               pComment;
        ADD_V8_STRING_PROPERTY(comment, pComment)
        //LPTSTR               pLocation;
        ADD_V8_STRING_PROPERTY(location, pLocation)
        //LPTSTR               pSepFile;
        ADD_V8_STRING_PROPERTY(sepFile, pSepFile)
        //LPTSTR               pPrintProcessor;
        ADD_V8_STRING_PROPERTY(printProcessor, pPrintProcessor)
        //LPTSTR               pDatatype;
        ADD_V8_STRING_PROPERTY(datatype, pDatatype)
        //LPTSTR               pParameters;
        ADD_V8_STRING_PROPERTY(parameters, pParameters)
    #undef ADD_V8_STRING_PROPERTY
        //DWORD                Status;
        // statuses from:
        // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
        v8::Local<v8::Array> result_printer_status = V8_VALUE_NEW_DEFAULT(Array);
        int i_status = 0;
        for(StatusMapType::const_iterator itStatus = getStatusMap().begin(); itStatus != getStatusMap().end(); ++itStatus)
        {
            if(printer->Status & itStatus->second)
            {
                MY_NODE_SET_OBJECT(result_printer_status, i_status++, V8_STRING_NEW_UTF8(itStatus->first.c_str()));
            }
        }
        MY_NODE_SET_OBJECT_PROP(result_printer, "status", result_printer_status);
        MY_NODE_SET_OBJECT_PROP(result_printer, "statusNumber", V8_VALUE_NEW(Number, printer->Status));
        //DWORD                Attributes;
        v8::Local<v8::Array> result_printer_attributes = V8_VALUE_NEW_DEFAULT(Array);
        int i_attribute = 0;
        for(StatusMapType::const_iterator itAttribute = getAttributeMap().begin(); itAttribute != getAttributeMap().end(); ++itAttribute)
        {
            if(printer->Attributes & itAttribute->second)
            {
                MY_NODE_SET_OBJECT(result_printer_attributes, i_attribute++, V8_STRING_NEW_UTF8(itAttribute->first.c_str()));
            }
        }
        MY_NODE_SET_OBJECT_PROP(result_printer, "attributes", result_printer_attributes);
        //DWORD                Priority;
        MY_NODE_SET_OBJECT_PROP(result_printer, "priority", V8_VALUE_NEW(Number, printer->Priority));
        //DWORD                DefaultPriority;
        MY_NODE_SET_OBJECT_PROP(result_printer, "defaultPriority", V8_VALUE_NEW(Number, printer->DefaultPriority));
        //DWORD                cJobs;
        //MY_NODE_SET_OBJECT_PROP(result_printer, "jobs", V8_VALUE_NEW(Number, printer->cJobs));
        //DWORD                AveragePPM;
        MY_NODE_SET_OBJECT_PROP(result_printer, "averagePPM", V8_VALUE_NEW(Number, printer->AveragePPM));

        //DWORD                StartTime;
        if(printer->StartTime > 0)
        {
            MY_NODE_SET_OBJECT_PROP(result_printer, "startTime", V8_VALUE_NEW(Number, printer->StartTime));
        }
        //DWORD                UntilTime;
        if(printer->UntilTime > 0)
        {
            MY_NODE_SET_OBJECT_PROP(result_printer, "untilTime", V8_VALUE_NEW(Number, printer->UntilTime));
        }

        //TODO: to finish to extract all data
        //LPDEVMODE            pDevMode;
        //PSECURITY_DESCRIPTOR pSecurityDescriptor;

        if(printer->cJobs > 0)
        {
            v8::Local<v8::Array> result_printer_jobs = V8_VALUE_NEW(Array, printer->cJobs);
            // get jobs
            std::string error_str = retrieveAndParseJobs(printer->pPrinterName, printer->cJobs, result_printer_jobs, iPrinterHandle);
            if(!error_str.empty())
            {
                return error_str;
            }
            MY_NODE_SET_OBJECT_PROP(result_printer, "jobs", result_printer_jobs);
        }
        return "";
    }
}

MY_NODE_MODULE_CALLBACK(getPrinters)
{
    MY_NODE_MODULE_HANDLESCOPE;
    DWORD printers_size = 0;
    DWORD printers_size_bytes = 0, dummyBytes = 0;
    DWORD Level = 2;
    DWORD flags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS;// https://msdn.microsoft.com/en-us/library/cc244669.aspx
    // First try to retrieve the number of printers
    BOOL bError = EnumPrintersW(flags, NULL, 2, NULL, 0, &printers_size_bytes, &printers_size);
    // allocate the required memmory
    MemValue<PRINTER_INFO_2W> printers(printers_size_bytes);
    if(!printers)
    {
        RETURN_EXCEPTION_STR("Error on allocating memory for printers");
    }

    bError = EnumPrintersW(flags, NULL, 2, (LPBYTE)(printers.get()), printers_size_bytes, &dummyBytes, &printers_size);
    if(!bError)
    {
        std::string error_str("Error on EnumPrinters: ");
	error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    v8::Local<v8::Array> result = V8_VALUE_NEW(Array, printers_size);
    // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
	PRINTER_INFO_2W *printer = printers.get();
	DWORD i = 0;
    for(; i < printers_size; ++i, ++printer)
    {
        v8::Local<v8::Object> result_printer = V8_VALUE_NEW_DEFAULT(Object);
        PrinterHandle printerHandle((LPWSTR)(printer->pPrinterName));
        std::string error_str = parsePrinterInfo(printer, result_printer, printerHandle);
        if(!error_str.empty())
        {
            RETURN_EXCEPTION_STR(error_str.c_str());
        }
        MY_NODE_SET_OBJECT(result, i, result_printer);
    }
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(getDefaultPrinterName)
{
    MY_NODE_MODULE_HANDLESCOPE;
    // size in chars of the printer name: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144876(v=vs.85).aspx
    DWORD cSize = 0;
    GetDefaultPrinterW(NULL, &cSize);

    if(cSize == 0) {
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(""));
    }

    MemValue<uint16_t> bPrinterName(cSize*sizeof(uint16_t));
    BOOL res = GetDefaultPrinterW((LPWSTR)(bPrinterName.get()), &cSize);

    if(!res) {
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(""));
    }

    MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_2BYTES((uint16_t*)bPrinterName.get()));
}

MY_NODE_MODULE_CALLBACK(getPrinter)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 1);
    REQUIRE_ARGUMENT_STRINGW(iArgs, 0, printername);

    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(*printername));
    if(!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    DWORD printers_size_bytes = 0, dummyBytes = 0;
    GetPrinterW(*printerHandle, 2, NULL, printers_size_bytes, &printers_size_bytes);
    MemValue<PRINTER_INFO_2W> printer(printers_size_bytes);
    if(!printer)
    {
        RETURN_EXCEPTION_STR("Error on allocating memory for printers");
    }
    BOOL bOK = GetPrinterW(*printerHandle, 2, (LPBYTE)(printer.get()), printers_size_bytes, &printers_size_bytes);
    if(!bOK)
    {
        std::string error_str("Error on GetPrinter: ");
	error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    v8::Local<v8::Object> result_printer = V8_VALUE_NEW_DEFAULT(Object);
    std::string error_str = parsePrinterInfo(printer.get(), result_printer, printerHandle);
    if(!error_str.empty())
    {
        RETURN_EXCEPTION_STR(error_str.c_str());
    }

    MY_NODE_MODULE_RETURN_VALUE(result_printer);
}

MY_NODE_MODULE_CALLBACK(getPrinterDriverOptions)
{
    MY_NODE_MODULE_HANDLESCOPE;
    RETURN_EXCEPTION_STR("not supported on windows");
}

MY_NODE_MODULE_CALLBACK(getJob)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 2);
    REQUIRE_ARGUMENT_STRINGW(iArgs, 0, printername);
    REQUIRE_ARGUMENT_INTEGER(iArgs, 1, jobId);
    if(jobId < 0)
    {
        RETURN_EXCEPTION_STR("Wrong job number");
    }
    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(*printername));
    if(!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
	error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    DWORD size_bytes = 0, dummyBytes = 0;
    GetJobW(*printerHandle, static_cast<DWORD>(jobId), 2, NULL, size_bytes, &size_bytes);
    MemValue<JOB_INFO_2W> job(size_bytes);
    if(!job)
    {
        RETURN_EXCEPTION_STR("Error on allocating memory for printers");
    }
    BOOL bOK = GetJobW(*printerHandle, static_cast<DWORD>(jobId), 2, (LPBYTE)job.get(), size_bytes, &dummyBytes);
    if(!bOK)
    {
        std::string error_str("Error on GetJob. Wrong job id or it was deleted: ");
	error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
    parseJobObject(job.get(), result_printer_job);
    MY_NODE_MODULE_RETURN_VALUE(result_printer_job);
}

MY_NODE_MODULE_CALLBACK(setJob)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 3);
    REQUIRE_ARGUMENT_STRINGW(iArgs, 0, printername);
    REQUIRE_ARGUMENT_INTEGER(iArgs, 1, jobId);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, jobCommandV8);
    if(jobId < 0)
    {
        RETURN_EXCEPTION_STR("Wrong job number");
    }
    std::string jobCommandStr(*jobCommandV8);
    StatusMapType::const_iterator itJobCommand = getJobCommandMap().find(jobCommandStr);
    if(itJobCommand == getJobCommandMap().end())
    {
        RETURN_EXCEPTION_STR("wrong job command. use getSupportedJobCommands to see the possible commands");
    }
    DWORD jobCommand = itJobCommand->second;
    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(*printername));
    if(!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    // TODO: add the possibility to set job properties
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd162978(v=vs.85).aspx
    BOOL ok = SetJobW(*printerHandle, (DWORD)jobId, 0, NULL, jobCommand);
    MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Boolean, ok == TRUE));
}

MY_NODE_MODULE_CALLBACK(getSupportedJobCommands)
{
    MY_NODE_MODULE_HANDLESCOPE;
    v8::Local<v8::Array> result = V8_VALUE_NEW_DEFAULT(Array);
    int i = 0;
    for(StatusMapType::const_iterator itJob = getJobCommandMap().begin(); itJob != getJobCommandMap().end(); ++itJob)
    {
        MY_NODE_SET_OBJECT(result, i++, V8_STRING_NEW_UTF8(itJob->first.c_str()));
    }
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(getSupportedPrintFormats)
{
    MY_NODE_MODULE_HANDLESCOPE;
    v8::Local<v8::Array> result = V8_VALUE_NEW_DEFAULT(Array);
    int format_i = 0;

    LPTSTR name = NULL;
    DWORD numBytes = 0, processorsNum = 0;

    // Check the amount of bytes required
    LPWSTR nullVal = NULL;
    EnumPrintProcessorsW(nullVal, nullVal, 1, (LPBYTE)(NULL), numBytes, &numBytes, &processorsNum);
    MemValue<_PRINTPROCESSOR_INFO_1W> processors(numBytes);
    // Retrieve processors
    BOOL isOK = EnumPrintProcessorsW(nullVal, nullVal, 1, (LPBYTE)(processors.get()), numBytes, &numBytes, &processorsNum);

    if(!isOK) {
        std::string error_str("error on EnumPrintProcessorsW: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }

    _PRINTPROCESSOR_INFO_1W *pProcessor = processors.get();

    for(DWORD processor_i = 0; processor_i < processorsNum; ++processor_i, ++pProcessor) {
        numBytes = 0;
        DWORD dataTypesNum = 0;
        EnumPrintProcessorDatatypesW(nullVal, pProcessor->pName, 1, (LPBYTE)(NULL), numBytes, &numBytes, &dataTypesNum);
        MemValue<_DATATYPES_INFO_1W> dataTypes(numBytes);
        isOK = EnumPrintProcessorDatatypesW(nullVal, pProcessor->pName, 1, (LPBYTE)(dataTypes.get()), numBytes, &numBytes, &dataTypesNum);

        if(!isOK) {
            std::string error_str("error on EnumPrintProcessorDatatypesW: ");
            error_str += getLastErrorCodeAndMessage();
            RETURN_EXCEPTION_STR(error_str.c_str());
        }

        _DATATYPES_INFO_1W *pDataType = dataTypes.get();
        for(DWORD j = 0; j < dataTypesNum; ++j, ++pDataType) {
            MY_NODE_SET_OBJECT(result, format_i++, V8_STRING_NEW_2BYTES((uint16_t*)(pDataType->pName)));
        }
    }

    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(PrintDirect)
{
    MY_NODE_MODULE_HANDLESCOPE;
    //TODO: to move in an unique place win and posix input parameters processing
    REQUIRE_ARGUMENTS(iArgs, 5);

    // can be string or buffer
    if(iArgs.Length()<=0)
    {
        RETURN_EXCEPTION_STR("Argument 0 missing");
    }

    std::string data;
    v8::Local<v8::Value> arg0(iArgs[0]);
    if (!getStringOrBufferFromV8Value(arg0, data))
    {
        RETURN_EXCEPTION_STR("Argument 0 must be a string or Buffer");
    }

    REQUIRE_ARGUMENT_STRINGW(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRINGW(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRINGW(iArgs, 3, type);

    BOOL     bStatus = true;
    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(*printername));
    DOC_INFO_1W DocInfo;
    DWORD      dwJob = 0L;
    DWORD      dwBytesWritten = 0L;

    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }

    // Fill in the structure with info about this "document."
    DocInfo.pDocName = (LPWSTR)(*docname);
    DocInfo.pOutputFile =  NULL;
    DocInfo.pDatatype = (LPWSTR)(*type);

    // Inform the spooler the document is beginning.
    dwJob = StartDocPrinterW(*printerHandle, 1, (LPBYTE)&DocInfo );
    if (dwJob > 0) {
        // Start a page.
        bStatus = StartPagePrinter(*printerHandle);
        if (bStatus) {
            // Send the data to the printer.
            //TODO: check with sizeof(LPTSTR) is the same as sizeof(char)
            bStatus = WritePrinter( *printerHandle, (LPVOID)(data.c_str()), (DWORD)data.size(), &dwBytesWritten);
            EndPagePrinter(*printerHandle);
        }else{
            std::string error_str("StartPagePrinter error: ");
    	    error_str += getLastErrorCodeAndMessage();
            RETURN_EXCEPTION_STR(error_str.c_str());
        }
        // Inform the spooler that the document is ending.
        EndDocPrinter(*printerHandle);
    }else{
    	std::string error_str("StartDocPrinterW error: ");
    	error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    // Check to see if correct number of bytes were written.
    if (dwBytesWritten != data.size()) {
        RETURN_EXCEPTION_STR("not sent all bytes");
    }
    MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Number, dwJob));
}

MY_NODE_MODULE_CALLBACK(PrintFile)
{
    MY_NODE_MODULE_HANDLESCOPE;
    RETURN_EXCEPTION_STR("Not yet implemented on Windows");
}
