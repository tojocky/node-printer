#include "node_printer.hpp"

#include <string>
#include <map>
#include <utility>

#include <cups/cups.h>

namespace
{
    typedef std::map<std::string, int> StatusMapType;
    typedef std::map<std::string, std::string> FormatMapType;

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
        STATUS_PRINTER_ADD("PRINTING", IPP_JOB_PROCESSING);
        STATUS_PRINTER_ADD("PRINTED", IPP_JOB_COMPLETED);
        STATUS_PRINTER_ADD("PAUSED", IPP_JOB_HELD);
        // Specific statuses
        STATUS_PRINTER_ADD("PENDING", IPP_JOB_PENDING);
        STATUS_PRINTER_ADD("PAUSED", IPP_JOB_STOPPED);
        STATUS_PRINTER_ADD("CANCELLED", IPP_JOB_CANCELLED);
        STATUS_PRINTER_ADD("ABORTED", IPP_JOB_ABORTED);

#undef STATUS_PRINTER_ADD
        return result;
    }

    const FormatMapType& getPrinterFormatMap()
    {
        static FormatMapType result;
        if(!result.empty())
        {
            return result;
        }
        result.insert(std::make_pair("RAW", CUPS_FORMAT_RAW));
        result.insert(std::make_pair("TEXT", CUPS_FORMAT_TEXT));
        result.insert(std::make_pair("PDF", CUPS_FORMAT_PDF));
        result.insert(std::make_pair("JPEG", CUPS_FORMAT_JPEG));
        result.insert(std::make_pair("POSTSCRIPT", CUPS_FORMAT_POSTSCRIPT));
        result.insert(std::make_pair("COMMAND", CUPS_FORMAT_COMMAND));
        result.insert(std::make_pair("AUTO", CUPS_FORMAT_AUTO));
        return result;
    }

    /** Parse job info object.
     * @return error string. if empty, then no error
     */
    std::string parseJobObject(const cups_job_t *job, v8::Handle<v8::Object> result_printer_job)
    {
        //Common fields
        result_printer_job->Set(v8::String::NewSymbol("id"), v8::Number::New(job->id));
        result_printer_job->Set(v8::String::NewSymbol("name"), v8::String::New(job->title));
        result_printer_job->Set(v8::String::NewSymbol("printerName"), v8::String::New(job->dest));
        result_printer_job->Set(v8::String::NewSymbol("user"), v8::String::New(job->user));
        std::string job_format(job->format);

        // Try to parse the data format, otherwise will write the unformatted one
        for(FormatMapType::const_iterator itFormat = getPrinterFormatMap().begin(); itFormat != getPrinterFormatMap().end(); ++itFormat)
        {
            if(itFormat->second == job_format)
            {
                job_format = itFormat->first;
                break;
            }
        }
        
        result_printer_job->Set(v8::String::NewSymbol("format"), v8::String::New(job_format.c_str()));
        result_printer_job->Set(v8::String::NewSymbol("priority"), v8::Number::New(job->priority));
        result_printer_job->Set(v8::String::NewSymbol("size"), v8::Number::New(job->size));
        v8::Local<v8::Array> result_printer_job_status = v8::Array::New();
        int i_status = 0;
        for(StatusMapType::const_iterator itStatus = getJobStatusMap().begin(); itStatus != getJobStatusMap().end(); ++itStatus)
        {
            if(job->state == itStatus->second)
            {
                result_printer_job_status->Set(i_status++, v8::String::New(itStatus->first.c_str()));
                // only one status could be on posix
                break;
            }
        }
        if(i_status == 0)
        {
            // A new status? return error then
            std::string error_str("wrong job status: ");
            error_str += job->state;
            return error_str;
        }
        
        result_printer_job->Set(v8::String::NewSymbol("status"), result_printer_job_status);

        //Specific fields
        // Ecmascript store time in milliseconds, but time_t in seconds
        result_printer_job->Set(v8::String::NewSymbol("completedTime"), v8::Date::New(job->completed_time*1000));
        result_printer_job->Set(v8::String::NewSymbol("creationTime"), v8::Date::New(job->creation_time*1000));
        result_printer_job->Set(v8::String::NewSymbol("processingTime"), v8::Date::New(job->processing_time*1000));

        // No error. return an empty string
        return "";
    }
    
    /** Parse printer info object
     * @return error string.
     */
    std::string parsePrinterInfo(const cups_dest_t * printer, v8::Handle<v8::Object> result_printer)
    {
        result_printer->Set(v8::String::NewSymbol("name"), v8::String::New(printer->name));
        result_printer->Set(v8::String::NewSymbol("isDefault"), v8::Boolean::New(static_cast<bool>(printer->is_default)));

        if(printer->instance)
        {
            result_printer->Set(v8::String::NewSymbol("instance"), v8::String::New(printer->instance));
        }
        v8::Local<v8::Object> result_printer_options = v8::Object::New();
        cups_option_t *dest_option = printer->options; 
        for(int j = 0; j < printer->num_options; ++j, ++dest_option)
        {
            result_printer_options->Set(v8::String::NewSymbol(dest_option->name), v8::String::New(dest_option->value));
        }
        result_printer->Set(v8::String::NewSymbol("options"), result_printer_options);
        // Get printer jobs
        cups_job_t * jobs;
        int totalJobs = cupsGetJobs(&jobs, printer->name, 0 /*0 means all users*/, CUPS_WHICHJOBS_ACTIVE);
        std::string error_str;
        if(totalJobs > 0)
        {
            v8::Local<v8::Array> result_priner_jobs = v8::Array::New(totalJobs);
            int jobi =0;
            cups_job_t * job = jobs;
            for(; jobi < totalJobs; ++jobi, ++job)
            {
                v8::Local<v8::Object> result_printer_job = v8::Object::New();
                error_str = parseJobObject(job, result_printer_job);
                if(!error_str.empty())
                {
                    // got an error? break then.
                    break;
                }
                result_priner_jobs->Set(jobi, result_printer_job);
            }
            result_printer->Set(v8::String::NewSymbol("jobs"), result_priner_jobs);
        }
        cupsFreeJobs(totalJobs, jobs);
        return error_str;
    }
}

v8::Handle<v8::Value> getPrinters(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;

    cups_dest_t *printers = NULL;
    int printers_size = cupsGetDests(&printers);
    v8::Local<v8::Array> result = v8::Array::New(printers_size);
    cups_dest_t *printer = printers;
    std::string error_str;
    for(int i = 0; i < printers_size; ++i, ++printer)
    {
        v8::Local<v8::Object> result_printer = v8::Object::New();
        error_str = parsePrinterInfo(printer, result_printer);
        if(!error_str.empty())
        {
            // got an error? break then
            break;
        }
        result->Set(i, result_printer);
    }
    cupsFreeDests(printers_size, printers);
    if(!error_str.empty())
    {
        // got an error? return the error then
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    return scope.Close(result);
}

v8::Handle<v8::Value> getPrinter(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;
    REQUIRE_ARGUMENTS(iArgs, 1);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);

    cups_dest_t *printers = NULL, *printer = NULL;
    int printers_size = cupsGetDests(&printers);
    printer = cupsGetDest(*printername, NULL, printers_size, printers);
    v8::Local<v8::Object> result_printer = v8::Object::New();
    if(printer != NULL)
    {
        parsePrinterInfo(printer, result_printer);
    }
    cupsFreeDests(printers_size, printers);
    if(printer == NULL)
    {
        // printer not found
        RETURN_EXCEPTION_STR("Printer not found");
    }
    return scope.Close(result_printer);
}

v8::Handle<v8::Value> getJob(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;
    REQUIRE_ARGUMENTS(iArgs, 2);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);
    REQUIRE_ARGUMENT_INTEGER(iArgs, 1, jobId);

    v8::Local<v8::Object> result_printer_job = v8::Object::New();
    // Get printer jobs
    cups_job_t *jobs = NULL, *jobFound = NULL;
    int totalJobs = cupsGetJobs(&jobs, *printername, 0 /*0 means all users*/, CUPS_WHICHJOBS_ALL);
    if(totalJobs > 0)
    {
        int jobi =0;
        cups_job_t * job = jobs;
        for(; jobi < totalJobs; ++jobi, ++job)
        {
            if(job->id != jobId)
            {
                continue;
            }
            // Job Found
            jobFound = job;
            parseJobObject(job, result_printer_job);
            break;
        }
    }
    cupsFreeJobs(totalJobs, jobs);
    if(jobFound == NULL)
    {
        // printer not found
        RETURN_EXCEPTION_STR("Printer job not found");
    }
    return scope.Close(result_printer_job);
}

v8::Handle<v8::Value> setJob(const v8::Arguments& iArgs)
{
    v8::HandleScope scope;
    REQUIRE_ARGUMENTS(iArgs, 3);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);
    REQUIRE_ARGUMENT_INTEGER(iArgs, 1, jobId);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, jobCommandV8);
    if(jobId < 0)
    {
        RETURN_EXCEPTION_STR("Wrong job number");
    }
    std::string jobCommandStr(*jobCommandV8);
    bool result_ok = false;
    if(jobCommandStr == "CANCEL")
    {
        result_ok = (cupsCancelJob(*printername, jobId) == 1);
    }
    else
    {
        RETURN_EXCEPTION_STR("wrong job command. use getSupportedJobCommands to see the possible commands");
    }
    return scope.Close(v8::Boolean::New(result_ok));
}

v8::Handle<v8::Value> getSupportedJobCommands(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    v8::Local<v8::Array> result = v8::Array::New();
    int i = 0;
    result->Set(i++, v8::String::New("CANCEL"));
    return scope.Close(result);
}

v8::Handle<v8::Value> getSupportedPrintFormats(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    v8::Local<v8::Array> result = v8::Array::New();
    int i = 0;
    for(FormatMapType::const_iterator itFormat = getPrinterFormatMap().begin(); itFormat != getPrinterFormatMap().end(); ++itFormat)
    {
        result->Set(i++, v8::String::New(itFormat->first.c_str()));
    }
    return scope.Close(result);
}

v8::Handle<v8::Value> PrintDirect(const v8::Arguments& iArgs) {
    v8::HandleScope scope;
    REQUIRE_ARGUMENTS(iArgs, 4);

    // can be string or buffer
    if(iArgs.Length() <= 0)
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 missing")));
    }

    std::string data;
    v8::Handle<v8::Value> arg0(iArgs[0]);

    if(arg0->IsString())
    {
        v8::String::Utf8Value data_str_v8(arg0->ToString());
        data.assign(*data_str_v8, data_str_v8.length());
    }
    else if(arg0->IsObject() && arg0.As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())
    {
        data.assign(static_cast<char*>(arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayData()),
                    arg0.As<v8::Object>()->GetIndexedPropertiesExternalArrayDataLength());
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string or Buffer")));
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 3, type);
    std::string type_str(*type);
    FormatMapType::const_iterator itFormat = getPrinterFormatMap().find(type_str);
    if(itFormat == getPrinterFormatMap().end())
    {
        type_str = itFormat->second;
    }
    else
    {
        return v8::ThrowException(v8::Exception::TypeError(v8::String::New("unsupported format type")));
    }
    int num_options = 0;
    cups_option_t *options = NULL;
    int job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, *printername, *docname, num_options, options);
    if(job_id > 0)
    {
        cupsStartDocument(CUPS_HTTP_DEFAULT, *printername, job_id, *docname, type_str.c_str(), 1 /*last document*/);
        /* cupsWriteRequestData can be called as many times as needed */
        //TODO: to split big buffer
        cupsWriteRequestData(CUPS_HTTP_DEFAULT, data.c_str(), data.size());
        cupsFinishDocument(CUPS_HTTP_DEFAULT, *printername);
    }
    return scope.Close(v8::Number::New(job_id));
}
