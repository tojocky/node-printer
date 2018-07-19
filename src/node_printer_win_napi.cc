#include "node_printer_napi.hpp"

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#include <Wingdi.h>
#pragma comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <iostream>
#include <string>
#include <cstring>

#include <map>
#include <utility>
#include <sstream>
#include <windows.h>
namespace
{
typedef std::map<std::string, DWORD> StatusMapType;

/** Memory value class management to avoid memory leak
    */
template <typename Type>
class MemValue : public MemValueBase<Type>
{
  public:
    /** Constructor of allocating iSizeKbytes bytes memory;
        * @param iSizeKbytes size in bytes of required allocating memory
        */
    MemValue(const DWORD iSizeKbytes)
    {
        _value = (Type *)malloc(iSizeKbytes);
    }

    ~MemValue()
    {
        free();
    }

  protected:
    virtual void free()
    {
        if (_value != NULL)
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
        if (_ok)
        {
            ClosePrinter(_printer);
        }
    }
    operator HANDLE() { return _printer; }
    operator bool() { return (!!_ok); }
    HANDLE &operator*() { return _printer; }
    HANDLE *operator->() { return &_printer; }
    const HANDLE &operator->() const { return _printer; }
    HANDLE _printer;
    BOOL _ok;
};

const StatusMapType &getJobCommandMap()
{
    static StatusMapType result;
    if (!result.empty())
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

const StatusMapType &getStatusMap()
{
    static StatusMapType result;
    if (!result.empty())
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

const StatusMapType &getAttributeMap()
{
    static StatusMapType result;
    if (!result.empty())
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

const StatusMapType &getJobStatusMap()
{
    static StatusMapType result;
    if (!result.empty())
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

void parseJobObject(JOB_INFO_2W *job, Napi::Object &result_printer_job, Napi::Env env)
{
    //Common fields
    //DWORD                JobId;
    result_printer_job.Set(Napi::String::New(env, "id"), Napi::Number::New(env, job->JobId));
#define ADD_STRING_PROPERTY(name, key, env)                                                                    \
    if ((job->##key != NULL) && (*job->##key != L'\0'))                                                        \
    {                                                                                                          \
        result_printer_job.Set(Napi::String::New(env, #name), Napi::String::New(env, (char16_t *)job->##key)); \
    }
    //LPTSTR               pPrinterName;
    ADD_STRING_PROPERTY(name, pPrinterName, env)
    //LPTSTR               pPrinterName;
    ADD_STRING_PROPERTY(printerName, pPrinterName, env);
    //LPTSTR               pUserName;
    ADD_STRING_PROPERTY(user, pUserName, env);
    //LPTSTR               pDatatype;
    ADD_STRING_PROPERTY(format, pDatatype, env);
    //DWORD                Priority;
    result_printer_job.Set(Napi::String::New(env, "priority"), Napi::Number::New(env, job->Priority));
    //DWORD                Size;
    result_printer_job.Set(Napi::String::New(env, "size"), Napi::Number::New(env, job->Size));
    //DWORD                Status;
    Napi::Array result_printer_job_status = Napi::Array::New(env);
    int i_status = 0;
    for (StatusMapType::const_iterator itStatus = getJobStatusMap().begin(); itStatus != getJobStatusMap().end(); ++itStatus)
    {
        if (job->Status & itStatus->second)
        {
            result_printer_job_status.Set(i_status++, Napi::String::New(env, itStatus->first.c_str()));
        }
    }
    //LPTSTR               pStatus;
    if ((job->pStatus != NULL) && (*job->pStatus != L'\0'))
    {
        result_printer_job_status.Set(i_status++, Napi::String::New(env, (char16_t *)job->pStatus));
    }
    result_printer_job.Set(Napi::String::New(env, "status"), result_printer_job_status);

    // Specific fields
    //LPTSTR               pMachineName;
    ADD_STRING_PROPERTY(machineName, pMachineName, env);
    //LPTSTR               pDocument;
    ADD_STRING_PROPERTY(document, pDocument, env);
    //LPTSTR               pNotifyName;
    ADD_STRING_PROPERTY(notifyName, pNotifyName, env);
    //LPTSTR               pPrintProcessor;
    ADD_STRING_PROPERTY(printProcessor, pPrintProcessor, env);
    //LPTSTR               pParameters;
    ADD_STRING_PROPERTY(parameters, pParameters, env);
    //LPTSTR               pDriverName;
    ADD_STRING_PROPERTY(driverName, pDriverName, env);
#undef ADD_STRING_PROPERTY
    //LPDEVMODE            pDevMode;
    //PSECURITY_DESCRIPTOR pSecurityDescriptor;
    //DWORD                Position;
    result_printer_job.Set(Napi::String::New(env, "position"), Napi::Number::New(env, job->Position));
    //DWORD                StartTime;
    result_printer_job.Set(Napi::String::New(env, "startTime"), Napi::Number::New(env, job->StartTime));
    //DWORD                UntilTime;
    result_printer_job.Set(Napi::String::New(env, "untilTime"), Napi::Number::New(env, job->UntilTime));
    //DWORD                TotalPages;
    result_printer_job.Set(Napi::String::New(env, "totalPages"), Napi::Number::New(env, job->TotalPages));
    //SYSTEMTIME           Submitted;
    //DWORD                Time;
    result_printer_job.Set(Napi::String::New(env, "time"), Napi::Number::New(env, job->Time));
    //DWORD                PagesPrinted;
    result_printer_job.Set(Napi::String::New(env, "pagesPrinted"), Napi::Number::New(env, job->PagesPrinted));
}

/**
     * Returns last error code and message string
     */
std::string getLastErrorCodeAndMessage()
{
    std::ostringstream s;
    DWORD erroCode = GetLastError();
    s << "code: " << erroCode;
    DWORD retSize;
    LPTSTR pTemp = NULL;
    retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            NULL,
                            erroCode,
                            LANG_NEUTRAL,
                            (LPTSTR)&pTemp,
                            0,
                            NULL);
    if (retSize && pTemp != NULL)
    {
        //pTemp[strlen(pTemp)-2]='\0'; //remove cr and newline character
        //TODO: check if it is needed to convert c string to std::string
        std::string stringMessage(pTemp);
        s << ", message: " << stringMessage;
        LocalFree((HLOCAL)pTemp);
    }

    return s.str();
}

std::string retrieveAndParseJobs(const LPWSTR iPrinterName,
                                 const DWORD &iTotalJobs,
                                 Napi::Object &result_printer_jobs,
                                 PrinterHandle &iPrinterHandle, Napi::Env env)
{
    DWORD bytes_needed = 0, totalJobs = 0;
    BOOL bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, NULL, bytes_needed, &bytes_needed, &totalJobs);
    MemValue<JOB_INFO_2W> jobs(bytes_needed);
    if (!jobs)
    {
        std::string error_str("Error on allocating memory for jobs: ");
        error_str += getLastErrorCodeAndMessage();
        return error_str;
    }
    DWORD dummy_bytes = 0;
    bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, (LPBYTE)jobs.get(), bytes_needed, &dummy_bytes, &totalJobs);
    if (!bError)
    {
        std::string error_str("Error on EnumJobsW: ");
        error_str += getLastErrorCodeAndMessage();
        return error_str;
    }
    JOB_INFO_2W *job = jobs.get();
    for (DWORD i = 0; i < totalJobs; ++i, ++job)
    {
        Napi::Object result_printer_job = Napi::Object::New(env);
        parseJobObject(job, result_printer_job, env);
        result_printer_jobs.Set(i, result_printer_job);
    }
    return std::string("");
}

std::string parsePrinterInfo(const PRINTER_INFO_2W *printer, Napi::Object &result_printer, PrinterHandle &iPrinterHandle, Napi::Env env)
{

#define ADD_STRING_PROPERTY(name, key, env)                                                                    \
    if ((printer->##key != NULL) && (*printer->##key != L'\0'))                                                \
    {                                                                                                          \
        result_printer.Set(Napi::String::New(env, #name), Napi::String::New(env, (char16_t *)printer->##key)); \
    }
    //LPTSTR               pPrinterName;
    ADD_STRING_PROPERTY(name, pPrinterName, env)
    //LPTSTR               pServerName;
    ADD_STRING_PROPERTY(serverName, pServerName, env)
    //LPTSTR               pShareName;
    ADD_STRING_PROPERTY(shareName, pShareName, env)
    //LPTSTR               pPortName;
    ADD_STRING_PROPERTY(portName, pPortName, env)
    //LPTSTR               pDriverName;
    ADD_STRING_PROPERTY(driverName, pDriverName, env)
    //LPTSTR               pComment;
    ADD_STRING_PROPERTY(comment, pComment, env)
    //LPTSTR               pLocation;
    ADD_STRING_PROPERTY(location, pLocation, env)
    //LPTSTR               pSepFile;
    ADD_STRING_PROPERTY(sepFile, pSepFile, env)
    //LPTSTR               pPrintProcessor;
    ADD_STRING_PROPERTY(printProcessor, pPrintProcessor, env)
    //LPTSTR               pDatatype;
    ADD_STRING_PROPERTY(datatype, pDatatype, env)
    //LPTSTR               pParameters;
    ADD_STRING_PROPERTY(parameters, pParameters, env)
#undef ADD_STRING_PROPERTY
    //DWORD                Status;
    // statuses from:
    // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
    Napi::Array result_printer_status = Napi::Array::New(env);
    int i_status = 0;
    for (StatusMapType::const_iterator itStatus = getStatusMap().begin(); itStatus != getStatusMap().end(); ++itStatus)
    {
        if (printer->Status & itStatus->second)
        {
            result_printer_status.Set(i_status, Napi::String::New(env, itStatus->first.c_str()));
            ++i_status;
        }
    }
    result_printer.Set(Napi::String::New(env, "status"), result_printer_status);
    result_printer.Set(Napi::String::New(env, "statusNumber"), Napi::Number::New(env, printer->Status));
    //DWORD                Attributes;
    Napi::Array result_printer_attributes = Napi::Array::New(env);
    int i_attribute = 0;
    for (StatusMapType::const_iterator itAttribute = getAttributeMap().begin(); itAttribute != getAttributeMap().end(); ++itAttribute)
    {
        if (printer->Attributes & itAttribute->second)
        {
            result_printer_attributes.Set(i_attribute, Napi::String::New(env, itAttribute->first.c_str()));
            ++i_attribute;
        }
    }
    result_printer.Set(Napi::String::New(env, "attributes"), result_printer_attributes);
    //DWORD                Priority;
    result_printer.Set(Napi::String::New(env, "priority"), Napi::Number::New(env, printer->Priority));
    //DWORD                DefaultPriority;
    result_printer.Set(Napi::String::New(env, "defaultPriority"), Napi::Number::New(env, printer->DefaultPriority));
    //DWORD                cJobs;
    //result_printer->Set(V8_STRING_NEW_UTF8("jobs"), V8_VALUE_NEW(Number, printer->cJobs));
    //DWORD                AveragePPM;
    result_printer.Set(Napi::String::New(env, "averagePPM"), Napi::Number::New(env, printer->AveragePPM));

    //DWORD                StartTime;
    if (printer->StartTime > 0)
    {
        result_printer.Set(Napi::String::New(env, "startTime"), Napi::Number::New(env, printer->StartTime));
    }
    //DWORD                UntilTime;
    if (printer->UntilTime > 0)
    {
        result_printer.Set(Napi::String::New(env, "untilTime"), Napi::Number::New(env, printer->UntilTime));
    }

    //TODO: to finish to extract all data
    //LPDEVMODE            pDevMode;
    //PSECURITY_DESCRIPTOR pSecurityDescriptor;

    if (printer->cJobs > 0)
    {
        Napi::Array result_printer_jobs = Napi::Array::New(env);
        // get jobs
        std::string error_str = retrieveAndParseJobs(printer->pPrinterName, printer->cJobs, result_printer_jobs, iPrinterHandle, env);
        if (!error_str.empty())
        {
            return error_str;
        }
        result_printer.Set(Napi::String::New(env, "jobs"), result_printer_jobs);
    }
    return "";
}

} // namespace

Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info)
{
    // size in chars of the printer name: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144876(v=vs.85).aspx
    DWORD cSize = 0;
    GetDefaultPrinterW(NULL, &cSize);

    if (cSize == 0)
    {
        return Napi::String::New(info.Env(), "");
    }

    MemValue<char16_t>
        bPrinterName(cSize * sizeof(char16_t));
    BOOL res = GetDefaultPrinterW((LPWSTR)(bPrinterName.get()), &cSize);
    if (!res)
    {
        return Napi::String::New(info.Env(), "");
    }

    return Napi::String::New(info.Env(), (char16_t *)bPrinterName.get());
}

Napi::Value getPrinters(const Napi::CallbackInfo &info)
{
    DWORD printers_size = 0;
    DWORD printers_size_bytes = 0, dummyBytes = 0;
    DWORD Level = 2;
    DWORD flags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS; // https://msdn.microsoft.com/en-us/library/cc244669.aspx
    // First try to retrieve the number of printers
    BOOL bError = EnumPrintersW(flags, NULL, 2, NULL, 0, &printers_size_bytes, &printers_size);
    // allocate the required memmory
    MemValue<PRINTER_INFO_2W> printers(printers_size_bytes);
    if (!printers)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Error on allocating memory for printers");
    }

    bError = EnumPrintersW(flags, NULL, 2, (LPBYTE)(printers.get()), printers_size_bytes, &dummyBytes, &printers_size);
    if (!bError)
    {
        std::string error_str("Error on EnumPrinters: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    Napi::Array result = Napi::Array::New(info.Env());
    // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
    PRINTER_INFO_2W *printer = printers.get();
    DWORD i = 0;
    for (; i < printers_size; ++i, ++printer)
    {
        Napi::Object result_printer = Napi::Object::New(info.Env());
        PrinterHandle printerHandle((LPWSTR)(printer->pPrinterName));
        std::string error_str = parsePrinterInfo(printer, result_printer, printerHandle, info.Env());
        if (!error_str.empty())
        {
            RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
        }
        result.Set(i, result_printer);
    }
    return result;
}

Napi::Value PrintDirect(const Napi::CallbackInfo &info)
{
    //TODO: to move in an unique place win and posix input parameters processing
    REQUIRE_ARGUMENTS(info, 5, info.Env());

    // can be string or buffer
    if (info.Length() <= 0)
    {
        Napi::Error::New(info.Env(), "Argument 0 missing").ThrowAsJavaScriptException();
        return info.Env().Null();
    }

    std::string data;
    Napi::Value arg0(info.Env(), info[0]);
    if (!getStringOrBufferFromNapiValue(arg0, data))
    {
        Napi::Error::New(info.Env(), "Argument 0 must be a string or Buffer").ThrowAsJavaScriptException();
        return info.Env().Null();
    }
    std::string tempo;
    REQUIRE_ARGUMENT_STRINGW(info, 1, printername, tempo);
    REQUIRE_ARGUMENT_STRINGW(info, 2, docname, tempo);
    REQUIRE_ARGUMENT_STRINGW(info, 3, type, tempo);
    BOOL bStatus = true;

    // Open a handle to the printer.

    PrinterHandle printerHandle((LPWSTR)&printername[0]);
    DOC_INFO_1W DocInfo;
    DWORD dwJob = 0L;
    DWORD dwBytesWritten = 0L;
    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    // Fill in the structure with info about this "document."
    DocInfo.pDocName = (LPWSTR)(&docname[0]);
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = (LPWSTR)(&type[0]);

    // // Inform the spooler the document is beginning.
    dwJob = StartDocPrinterW(*printerHandle, 1, (LPBYTE)&DocInfo);
    if (dwJob > 0)
    {
        // Start a page.
        bStatus = StartPagePrinter(*printerHandle);
        if (bStatus)
        {
            // Send the data to the printer.
            //TODO: check with sizeof(LPTSTR) is the same as sizeof(char)
            bStatus = WritePrinter(*printerHandle, (LPVOID)(data.c_str()), (DWORD)data.size(), &dwBytesWritten);
            EndPagePrinter(*printerHandle);
        }
        else
        {
            std::string error_str("StartPagePrinter error: ");
            error_str += getLastErrorCodeAndMessage();
            RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
        }
        // Inform the spooler that the document is ending.
        EndDocPrinter(*printerHandle);
    }
    else
    {
        std::string error_str("StartDocPrinterW error: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    // Check to see if correct number of bytes were written.
    if (dwBytesWritten != data.size())
    {
        RETURN_EXCEPTION_STR(info.Env(), "not sent all bytes");
    }
    return Napi::Number::New(info.Env(), dwJob);
}

Napi::Value PrintFile(const Napi::CallbackInfo &info)
{
    RETURN_EXCEPTION_STR(info.Env(), "Not yet implemented on Windows");
}

Napi::Value getSupportedJobCommands(const Napi::CallbackInfo &info)
{
    Napi::Array result = Napi::Array::New(info.Env());
    int i = 0;
    for (StatusMapType::const_iterator itJob = getJobCommandMap().begin(); itJob != getJobCommandMap().end(); ++itJob)
    {
        result.Set(i++, Napi::String::New(info.Env(), itJob->first.c_str()));
    }
    return result;
}

Napi::Value getPrinter(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(info, 1, info.Env());
    std::string tempo;
    REQUIRE_ARGUMENT_STRINGW(info, 0, printername, tempo);

    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(&printername[0]));
    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    DWORD printers_size_bytes = 0, dummyBytes = 0;
    GetPrinterW(*printerHandle, 2, NULL, printers_size_bytes, &printers_size_bytes);
    MemValue<PRINTER_INFO_2W> printer(printers_size_bytes);
    if (!printer)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Error on allocating memory for printers");
    }
    BOOL bOK = GetPrinterW(*printerHandle, 2, (LPBYTE)(printer.get()), printers_size_bytes, &printers_size_bytes);
    if (!bOK)
    {
        std::string error_str("Error on GetPrinter: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    Napi::Object result_printer = Napi::Object::New(info.Env());
    std::string error_str = parsePrinterInfo(printer.get(), result_printer, printerHandle, info.Env());
    if (!error_str.empty())
    {
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }

    return result_printer;
}

Napi::Value getPrinterDriverOptions(const Napi::CallbackInfo &info)
{
    RETURN_EXCEPTION_STR(info.Env(), "not supported on windows");
}

Napi::Value getSupportedPrintFormats(const Napi::CallbackInfo &info)
{
    Napi::Array result = Napi::Array::New(info.Env());
    int format_i = 0;

    LPTSTR name = NULL;
    DWORD numBytes = 0, processorsNum = 0;

    // Check the amount of bytes required
    LPWSTR nullVal = NULL;
    EnumPrintProcessorsW(nullVal, nullVal, 1, (LPBYTE)(NULL), numBytes, &numBytes, &processorsNum);
    MemValue<_PRINTPROCESSOR_INFO_1W> processors(numBytes);
    // Retrieve processors
    BOOL isOK = EnumPrintProcessorsW(nullVal, nullVal, 1, (LPBYTE)(processors.get()), numBytes, &numBytes, &processorsNum);

    if (!isOK)
    {
        std::string error_str("error on EnumPrintProcessorsW: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }

    _PRINTPROCESSOR_INFO_1W *pProcessor = processors.get();

    for (DWORD processor_i = 0; processor_i < processorsNum; ++processor_i, ++pProcessor)
    {
        numBytes = 0;
        DWORD dataTypesNum = 0;
        EnumPrintProcessorDatatypesW(nullVal, pProcessor->pName, 1, (LPBYTE)(NULL), numBytes, &numBytes, &dataTypesNum);
        MemValue<_DATATYPES_INFO_1W> dataTypes(numBytes);
        isOK = EnumPrintProcessorDatatypesW(nullVal, pProcessor->pName, 1, (LPBYTE)(dataTypes.get()), numBytes, &numBytes, &dataTypesNum);

        if (!isOK)
        {
            std::string error_str("error on EnumPrintProcessorDatatypesW: ");
            error_str += getLastErrorCodeAndMessage();
            RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
        }

        _DATATYPES_INFO_1W *pDataType = dataTypes.get();
        for (DWORD j = 0; j < dataTypesNum; ++j, ++pDataType)
        {
            result.Set(format_i++, Napi::String::New(info.Env(), (char16_t *)(pDataType->pName)));
        }
    }

    return result;
}

Napi::Value getJob(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(info, 2, info.Env());
    std::string tempo;
    REQUIRE_ARGUMENT_STRINGW(info, 0, printername, tempo);
    REQUIRE_ARGUMENT_INTEGER(info, 1, jobId);
    if (jobId < 0)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Wrong job number");
    }
    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(&printername[0]));
    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    DWORD size_bytes = 0, dummyBytes = 0;
    GetJobW(*printerHandle, static_cast<DWORD>(jobId), 2, NULL, size_bytes, &size_bytes);
    MemValue<JOB_INFO_2W> job(size_bytes);
    if (!job)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Error on allocating memory for printers");
    }
    BOOL bOK = GetJobW(*printerHandle, static_cast<DWORD>(jobId), 2, (LPBYTE)job.get(), size_bytes, &dummyBytes);
    if (!bOK)
    {
        std::string error_str("Error on GetJob. Wrong job id or it was deleted: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    Napi::Object result_printer_job = Napi::Object::New(info.Env());
    parseJobObject(job.get(), result_printer_job, info.Env());
    return result_printer_job;
}

Napi::Value setJob(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(info, 3, info.Env());
    std::string tempo;
    REQUIRE_ARGUMENT_STRINGW(info, 0, printername, tempo);
    REQUIRE_ARGUMENT_INTEGER(info, 1, jobId);
    REQUIRE_ARGUMENT_STRING(info, 2, jobCommandPar);
    if (jobId < 0)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Wrong job number");
    }
    std::string jobCommandStr(jobCommandPar);
    StatusMapType::const_iterator itJobCommand = getJobCommandMap().find(jobCommandStr);
    if (itJobCommand == getJobCommandMap().end())
    {
        RETURN_EXCEPTION_STR(info.Env(), "wrong job command. use getSupportedJobCommands to see the possible commands");
    }
    DWORD jobCommand = itJobCommand->second;
    // Open a handle to the printer.
    PrinterHandle printerHandle((LPWSTR)(&printername[0]));
    if (!printerHandle)
    {
        std::string error_str("error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    // TODO: add the possibility to set job properties
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd162978(v=vs.85).aspx
    BOOL ok = SetJobW(*printerHandle, (DWORD)jobId, 0, NULL, jobCommand);
    return Napi::Boolean::New(info.Env(), ok == TRUE);
}