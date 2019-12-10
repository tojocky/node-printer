#include "node_printer.hpp"

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <node_version.h>

#include <cups/cups.h>
#include <cups/ppd.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

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
#ifdef CUPS_FORMAT_PDF
        result.insert(std::make_pair("PDF", CUPS_FORMAT_PDF));
#endif
#ifdef CUPS_FORMAT_JPEG
        result.insert(std::make_pair("JPEG", CUPS_FORMAT_JPEG));
#endif
#ifdef CUPS_FORMAT_POSTSCRIPT
        result.insert(std::make_pair("POSTSCRIPT", CUPS_FORMAT_POSTSCRIPT));
#endif
#ifdef CUPS_FORMAT_COMMAND
        result.insert(std::make_pair("COMMAND", CUPS_FORMAT_COMMAND));
#endif
#ifdef CUPS_FORMAT_AUTO
        result.insert(std::make_pair("AUTO", CUPS_FORMAT_AUTO));
#endif
        return result;
    }

    /** Parse job info object.
     * @return error string. if empty, then no error
     */
    std::string parseJobObject(const cups_job_t *job, v8::Local<v8::Object> result_printer_job)
    {
        MY_NODE_MODULE_ISOLATE_DECL
        //Common fields
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "id", V8_VALUE_NEW(Number, job->id));
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "name", V8_STRING_NEW_UTF8(job->title));
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "printerName", V8_STRING_NEW_UTF8(job->dest));
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "user", V8_STRING_NEW_UTF8(job->user));
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

        MY_NODE_SET_OBJECT_PROP(result_printer_job, "format", V8_STRING_NEW_UTF8(job_format.c_str()));
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "priority", V8_VALUE_NEW(Number, job->priority));
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "size", V8_VALUE_NEW(Number, job->size));
        v8::Local<v8::Array> result_printer_job_status = V8_VALUE_NEW_DEFAULT(Array);
        int i_status = 0;
        for(StatusMapType::const_iterator itStatus = getJobStatusMap().begin(); itStatus != getJobStatusMap().end(); ++itStatus)
        {
            if(job->state == itStatus->second)
            {
                MY_NODE_SET_OBJECT(result_printer_job_status, i_status++, V8_STRING_NEW_UTF8(itStatus->first.c_str()));
                // only one status could be on posix
                break;
            }
        }
        if(i_status == 0)
        {
            // A new status? report as unsupported
            std::ostringstream s;
            s << "unsupported job status: " << job->state;
            MY_NODE_SET_OBJECT(result_printer_job_status, i_status++, V8_STRING_NEW_UTF8(s.str().c_str()));
        }

        MY_NODE_SET_OBJECT_PROP(result_printer_job, "status", result_printer_job_status);

        //Specific fields
        // Ecmascript store time in milliseconds, but time_t in seconds

        double creationTime = ((double)job->creation_time) * 1000;
        double completedTime = ((double)job->completed_time) * 1000;
        double processingTime = ((double)job->processing_time) * 1000;

        MY_NODE_SET_OBJECT_PROP(result_printer_job, "completedTime", Nan::New<v8::Date>(completedTime).ToLocalChecked());
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "creationTime", Nan::New<v8::Date>(creationTime).ToLocalChecked());
        MY_NODE_SET_OBJECT_PROP(result_printer_job, "processingTime", Nan::New<v8::Date>(processingTime).ToLocalChecked());

        // No error. return an empty string
        return "";
    }

    /** Parses printer driver PPD options
     */
    void populatePpdOptions(v8::Local<v8::Object> ppd_options, ppd_file_t  *ppd, ppd_group_t *group)
    {
        int i, j;
        ppd_option_t *option;
        ppd_choice_t *choice;
        ppd_group_t *subgroup;

        for (i = group->num_options, option = group->options; i > 0; --i, ++option)
        {
            MY_NODE_MODULE_ISOLATE_DECL
            v8::Local<v8::Object> ppd_suboptions = V8_VALUE_NEW_DEFAULT(Object);
            for (j = option->num_choices, choice = option->choices;
                 j > 0;
                 --j, ++choice)
            {
                MY_NODE_SET_OBJECT_PROP(ppd_suboptions, choice->choice, V8_VALUE_NEW(Boolean, static_cast<bool>(choice->marked)));
            }

            MY_NODE_SET_OBJECT_PROP(ppd_options, option->keyword, ppd_suboptions);
        }

        for (i = group->num_subgroups, subgroup = group->subgroups; i > 0; --i, ++subgroup) {
            populatePpdOptions(ppd_options, ppd, subgroup);
        }
    }

    /** Parse printer driver options
     * @return error string.
     */
    std::string parseDriverOptions(const cups_dest_t * printer, v8::Local<v8::Object> ppd_options)
    {
        const char *filename;
        ppd_file_t *ppd;
        ppd_group_t *group;
        int i;

        std::ostringstream error_str; // error string

        if ((filename = cupsGetPPD(printer->name)) != NULL)
        {
            if ((ppd = ppdOpenFile(filename)) != NULL)
            {
                 ppdMarkDefaults(ppd);
                 cupsMarkOptions(ppd, printer->num_options, printer->options);

                 for (i = ppd->num_groups, group = ppd->groups; i > 0; --i, ++group)
                 {
                    populatePpdOptions(ppd_options, ppd, group);
                 }
                 ppdClose(ppd);
            }
            else
            {
                error_str << "Unable to open PPD filename " << filename << " ";
            }
            unlink(filename);
        }
        else
        {
            error_str << "Unable to get CUPS PPD driver file. ";
        }

        return error_str.str();
    }


    /** Parse printer info object
     * @return error string.
     */
    std::string parsePrinterInfo(const cups_dest_t * printer, v8::Local<v8::Object> result_printer)
    {
        MY_NODE_MODULE_ISOLATE_DECL
        MY_NODE_SET_OBJECT_PROP(result_printer, "name", V8_STRING_NEW_UTF8(printer->name));
        MY_NODE_SET_OBJECT_PROP(result_printer, "isDefault", V8_VALUE_NEW(Boolean, static_cast<bool>(printer->is_default)));

        if(printer->instance)
        {
            MY_NODE_SET_OBJECT_PROP(result_printer, "instance", V8_STRING_NEW_UTF8(printer->instance));
        }

        v8::Local<v8::Object> result_printer_options = V8_VALUE_NEW_DEFAULT(Object);
        cups_option_t *dest_option = printer->options;

        for(int j = 0; j < printer->num_options; ++j, ++dest_option)
        {
            MY_NODE_SET_OBJECT_PROP(result_printer_options, dest_option->name, V8_STRING_NEW_UTF8(dest_option->value));
        }

        MY_NODE_SET_OBJECT_PROP(result_printer, "options", result_printer_options);
        // Get printer jobs
        cups_job_t * jobs;
        int totalJobs = cupsGetJobs(&jobs, printer->name, 0 /*0 means all users*/, CUPS_WHICHJOBS_ACTIVE);
        std::string error_str;
        if(totalJobs > 0)
        {
            v8::Local<v8::Array> result_priner_jobs = V8_VALUE_NEW(Array, totalJobs);
            int jobi =0;
            cups_job_t * job = jobs;
            for(; jobi < totalJobs; ++jobi, ++job)
            {
                v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
                error_str = parseJobObject(job, result_printer_job);
                if(!error_str.empty())
                {
                    // got an error? break then.
                    break;
                }
                MY_NODE_SET_OBJECT(result_priner_jobs, jobi, result_printer_job);
            }
            MY_NODE_SET_OBJECT_PROP(result_printer, "jobs", result_priner_jobs);
        }
        cupsFreeJobs(totalJobs, jobs);
        return error_str;
    }

    /// cups option class to automatically free memory.
    class CupsOptions: public MemValueBase<cups_option_t> {
    protected:
        int num_options;
        virtual void free() {
            if(_value != NULL)
            {
                cupsFreeOptions(num_options, get());
                _value = NULL;
                num_options = 0;
            }
        }
    public:
        CupsOptions(): num_options(0) {}
        ~CupsOptions () { free(); }

        /// Add options from v8 object
        CupsOptions(v8::Local<v8::Object> iV8Options): num_options(0) {
            v8::Local<v8::Array> props = Nan::GetPropertyNames(iV8Options).ToLocalChecked();

            for(unsigned int i = 0; i < props->Length(); ++i) {
                v8::Local<v8::Value> key(MY_NODE_GET_OBJECT(props, i));
                Nan::Utf8String keyStr(V8_LOCAL_STRING_FROM_VALUE(key));
                Nan::Utf8String valStr(V8_LOCAL_STRING_FROM_VALUE(MY_NODE_GET_OBJECT(iV8Options, key)));

                num_options = cupsAddOption(*keyStr, *valStr, num_options, &_value);
            }
        }

        const int& getNumOptions() { return num_options; }
    };
}

MY_NODE_MODULE_CALLBACK(getPrinters)
{
    MY_NODE_MODULE_HANDLESCOPE;

    cups_dest_t *printers = NULL;
    int printers_size = cupsGetDests(&printers);
    v8::Local<v8::Array> result = V8_VALUE_NEW(Array, printers_size);
    cups_dest_t *printer = printers;
    std::string error_str;
    for(int i = 0; i < printers_size; ++i, ++printer)
    {
        v8::Local<v8::Object> result_printer = V8_VALUE_NEW_DEFAULT(Object);
        error_str = parsePrinterInfo(printer, result_printer);
        if(!error_str.empty())
        {
            // got an error? break then
            break;
        }
        MY_NODE_SET_OBJECT(result, i, result_printer);
    }
    cupsFreeDests(printers_size, printers);
    if(!error_str.empty())
    {
        // got an error? return the error then
        RETURN_EXCEPTION_STR(error_str.c_str());
    }
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(getDefaultPrinterName)
{
    MY_NODE_MODULE_HANDLESCOPE;
    //This does not return default user printer name according to https://www.cups.org/documentation.php/doc-2.0/api-cups.html#cupsGetDefault2
    //so leave as undefined and JS implementation will loop in all printers
    /*
    const char * printerName = cupsGetDefault();

    // return default printer name only if defined
    if(printerName != NULL) {
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(printerName));
    }
    */
    MY_NODE_MODULE_RETURN_UNDEFINED();
}

MY_NODE_MODULE_CALLBACK(getPrinter)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 1);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);

    cups_dest_t *printers = NULL, *printer = NULL;
    int printers_size = cupsGetDests(&printers);
    printer = cupsGetDest(*printername, NULL, printers_size, printers);
    v8::Local<v8::Object> result_printer = V8_VALUE_NEW_DEFAULT(Object);
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
    MY_NODE_MODULE_RETURN_VALUE(result_printer);
}

MY_NODE_MODULE_CALLBACK(getPrinterDriverOptions)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 1);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);

    cups_dest_t *printers = NULL, *printer = NULL;
    int printers_size = cupsGetDests(&printers);
    printer = cupsGetDest(*printername, NULL, printers_size, printers);
    v8::Local<v8::Object> driver_options = V8_VALUE_NEW_DEFAULT(Object);
    if(printer != NULL)
    {
        parseDriverOptions(printer, driver_options);
    }
    cupsFreeDests(printers_size, printers);
    if(printer == NULL)
    {
        // printer not found
        RETURN_EXCEPTION_STR("Printer not found");
    }
    MY_NODE_MODULE_RETURN_VALUE(driver_options);
}

MY_NODE_MODULE_CALLBACK(getJob)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 2);
    REQUIRE_ARGUMENT_STRING(iArgs, 0, printername);
    REQUIRE_ARGUMENT_INTEGER(iArgs, 1, jobId);

    v8::Local<v8::Object> result_printer_job = V8_VALUE_NEW_DEFAULT(Object);
    // Get printer jobs
    cups_job_t *jobs = NULL, *jobFound = NULL;
    int totalJobs = cupsGetJobs(&jobs, *printername, 0 /*0 means all users*/, CUPS_WHICHJOBS_ALL);
    if(totalJobs > 0)
    {
        int jobi = 0;
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
    MY_NODE_MODULE_RETURN_VALUE(result_printer_job);
}

MY_NODE_MODULE_CALLBACK(setJob)
{
    MY_NODE_MODULE_HANDLESCOPE;
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
    MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Boolean, result_ok));
}

MY_NODE_MODULE_CALLBACK(getSupportedJobCommands)
{
    MY_NODE_MODULE_HANDLESCOPE;
    v8::Local<v8::Array> result = V8_VALUE_NEW_DEFAULT(Array);
    int i = 0;
    MY_NODE_SET_OBJECT(result, i++, V8_STRING_NEW_UTF8("CANCEL"));
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(getSupportedPrintFormats)
{
    MY_NODE_MODULE_HANDLESCOPE;
    v8::Local<v8::Array> result = V8_VALUE_NEW_DEFAULT(Array);
    int i = 0;
    for(FormatMapType::const_iterator itFormat = getPrinterFormatMap().begin(); itFormat != getPrinterFormatMap().end(); ++itFormat)
    {
        MY_NODE_SET_OBJECT(result, i++, V8_STRING_NEW_UTF8(itFormat->first.c_str()));
    }
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(PrintDirect)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 5);

    // can be string or buffer
    if(iArgs.Length() <= 0)
    {
        RETURN_EXCEPTION_STR("Argument 0 missing");
    }

    std::string data;
    v8::Local<v8::Value> arg0(iArgs[0]);
    if (!getStringOrBufferFromV8Value(arg0, data))
    {
        RETURN_EXCEPTION_STR("Argument 0 must be a string or Buffer");
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 1, printername);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 3, type);
    REQUIRE_ARGUMENT_OBJECT(iArgs, 4, print_options);

    std::string type_str(*type);
    FormatMapType::const_iterator itFormat = getPrinterFormatMap().find(type_str);
    if(itFormat == getPrinterFormatMap().end())
    {
        RETURN_EXCEPTION_STR("unsupported format type");
    }
    type_str = itFormat->second;

    CupsOptions options(print_options);

    int job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, *printername, *docname, options.getNumOptions(), options.get());
    if(job_id == 0) {
        RETURN_EXCEPTION_STR(cupsLastErrorString());
    }

    if(HTTP_CONTINUE != cupsStartDocument(CUPS_HTTP_DEFAULT, *printername, job_id, *docname, type_str.c_str(), 1 /*last document*/)) {
        RETURN_EXCEPTION_STR(cupsLastErrorString());
    }

    /* cupsWriteRequestData can be called as many times as needed */
    //TODO: to split big buffer
    if (HTTP_CONTINUE != cupsWriteRequestData(CUPS_HTTP_DEFAULT, data.c_str(), data.size())) {
        cupsFinishDocument(CUPS_HTTP_DEFAULT, *printername);
        RETURN_EXCEPTION_STR(cupsLastErrorString());
    }

    cupsFinishDocument(CUPS_HTTP_DEFAULT, *printername);

    MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Number, job_id));
}

MY_NODE_MODULE_CALLBACK(PrintFile)
{
    MY_NODE_MODULE_HANDLESCOPE;
    REQUIRE_ARGUMENTS(iArgs, 3);

    // can be string or buffer
    if(iArgs.Length() <= 0)
    {
        RETURN_EXCEPTION_STR("Argument 0 missing");
    }

    REQUIRE_ARGUMENT_STRING(iArgs, 0, filename);
    REQUIRE_ARGUMENT_STRING(iArgs, 1, docname);
    REQUIRE_ARGUMENT_STRING(iArgs, 2, printer);
    REQUIRE_ARGUMENT_OBJECT(iArgs, 3, print_options);

    CupsOptions options(print_options);

    int job_id = cupsPrintFile(*printer, *filename, *docname, options.getNumOptions(), options.get());

    if(job_id == 0){
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(cupsLastErrorString()));
    } else {
        MY_NODE_MODULE_RETURN_VALUE(V8_VALUE_NEW(Number, job_id));
    }
}
