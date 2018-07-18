#include "node_printer_napi.hpp"

#include <string>
#include <map>
#include <utility>
#include <sstream>

#include <cups/cups.h>
#include <cups/ppd.h>

namespace
{
typedef std::map<std::string, int> StatusMapType;
typedef std::map<std::string, std::string> FormatMapType;

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

const FormatMapType &getPrinterFormatMap()
{
    static FormatMapType result;
    if (!result.empty())
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
std::string parseJobObject(const cups_job_t *job, Napi::Object &result_printer_job, Napi::Env env)
{
    //Common fields
    result_printer_job.Set(Napi::String::New(env, "id"), Napi::Number::New(env, job->id));
    result_printer_job.Set(Napi::String::New(env, "name"), Napi::String::New(env, job->title));
    result_printer_job.Set(Napi::String::New(env, "printerName"), Napi::String::New(env, job->dest));
    result_printer_job.Set(Napi::String::New(env, "user"), Napi::String::New(env, job->user));
    std::string job_format(job->format);

    // Try to parse the data format, otherwise will write the unformatted one
    for (FormatMapType::const_iterator itFormat = getPrinterFormatMap().begin(); itFormat != getPrinterFormatMap().end(); ++itFormat)
    {
        if (itFormat->second == job_format)
        {
            job_format = itFormat->first;
            break;
        }
    }

    result_printer_job.Set(Napi::String::New(env, "format"), Napi::String::New(env, job_format.c_str()));
    result_printer_job.Set(Napi::String::New(env, "priority"), Napi::Number::New(env, job->priority));
    result_printer_job.Set(Napi::String::New(env, "size"), Napi::Number::New(env, job->size));
    Napi::Array result_printer_job_status = Napi::Array::New(env);
    int i_status = 0;
    for (StatusMapType::const_iterator itStatus = getJobStatusMap().begin(); itStatus != getJobStatusMap().end(); ++itStatus)
    {
        if (job->state == itStatus->second)
        {
            result_printer_job_status.Set(i_status++, Napi::String::New(env, itStatus->first.c_str()));
            // only one status could be on posix
            break;
        }
    }
    if (i_status == 0)
    {
        // A new status? report as unsupported
        std::ostringstream s;
        s << "unsupported job status: " << job->state;
        result_printer_job_status.Set(i_status++, Napi::String::New(env, s.str().c_str()));
    }

    result_printer_job.Set(Napi::String::New(env, "status"), result_printer_job_status);

    //Specific fields
    // Ecmascript store time in milliseconds, but time_t in seconds

    double creationTime = ((double)job->creation_time) * 1000;
    double completedTime = ((double)job->completed_time) * 1000;
    double processingTime = ((double)job->processing_time) * 1000;

    result_printer_job.Set(Napi::String::New(env, "completedTime"), Napi::Number::New(env, completedTime));
    result_printer_job.Set(Napi::String::New(env, "creationTime"), Napi::Number::New(env, creationTime));
    result_printer_job.Set(Napi::String::New(env, "processingTime"), Napi::Number::New(env, processingTime));

    // No error. return an empty string
    return "";
}

/** Parse printer info object
     * @return error string.
     */
std::string parsePrinterInfo(const cups_dest_t *printer, Napi::Object &result_printer, Napi::Env env)
{
    result_printer.Set(Napi::String::New(env, "name"),
                       Napi::String::New(env, printer->name));
    result_printer.Set(Napi::String::New(env, "isDefault"), Napi::Boolean::New(env, static_cast<bool>(printer->is_default)));
    if (printer->instance)
    {
        result_printer.Set(Napi::String::New(env, "instance"), Napi::String::New(env, printer->instance));
    }

    Napi::Object result_printer_options = Napi::Object::New(env);
    cups_option_t *dest_option = printer->options;
    for (int j = 0; j < printer->num_options; ++j, ++dest_option)
    {
        result_printer_options.Set(Napi::String::New(env, dest_option->name), Napi::String::New(env, dest_option->value));
    }
    result_printer.Set(Napi::String::New(env, "options"), result_printer_options);
    // Get printer jobs
    cups_job_t *jobs;
    int totalJobs = cupsGetJobs(&jobs, printer->name, 0 /*0 means all users*/, CUPS_WHICHJOBS_ACTIVE);
    std::string error_str;
    if (totalJobs > 0)
    {
        Napi::Array result_priner_jobs = Napi::Array::New(env);
        int jobi = 0;
        cups_job_t *job = jobs;
        for (; jobi < totalJobs; ++jobi, ++job)
        {
            Napi::Object result_printer_job = Napi::Object::New(env);
            error_str = parseJobObject(job, result_printer_job, env);

            if (!error_str.empty())
            {
                // got an error? break then.
                break;
            }
            result_priner_jobs.Set(jobi, result_printer_job);
        }
        result_printer.Set(Napi::String::New(env, "jobs"), result_priner_jobs);
    }
    cupsFreeJobs(totalJobs, jobs);
    return error_str;
}

class CupsOptions : public MemValueBase<cups_option_t>
{
  protected:
    int num_options;
    virtual void free()
    {
        if (_value != NULL)
        {
            cupsFreeOptions(num_options, get());
            _value = NULL;
            num_options = 0;
        }
    }

  public:
    CupsOptions() : num_options(0) {}
    ~CupsOptions() { free(); }

    /// Add options from v8 object
    CupsOptions(Napi::Object iOptions) : num_options(0)
    {
        Napi::Array props = iOptions.GetPropertyNames();

        for (unsigned int i = 0; i < props.Length(); ++i)
        {
            Napi::Value key(props.Get(i));
            std::string keyStr(key.ToString());
            std::string valStr(iOptions.Get(key).ToString());

            num_options = cupsAddOption(keyStr.c_str(), valStr.c_str(), num_options, &_value);
        }
    }

    const int &getNumOptions() { return num_options; }
};
} // namespace

Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info)
{

    //This does not return default user printer name according to https://www.cups.org/documentation.php/doc-2.0/api-cups.html#cupsGetDefault2
    //so leave as undefined and JS implementation will loop in all printers
    /*
    const char * printerName = cupsGetDefault();

    // return default printer name only if defined
    if(printerName != NULL) {
        MY_NODE_MODULE_RETURN_VALUE(V8_STRING_NEW_UTF8(printerName));
    }
    */
    return info.Env().Null();
}

Napi::Value getPrinters(const Napi::CallbackInfo &info)
{

    cups_dest_t *printers = NULL;
    int printers_size = cupsGetDests(&printers);
    Napi::Array result = Napi::Array::New(info.Env());
    cups_dest_t *printer = printers;
    std::string error_str;
    for (int i = 0; i < printers_size; ++i, ++printer)
    {
        Napi::Object result_printer = Napi::Object::New(info.Env());
        error_str = parsePrinterInfo(printer, result_printer, info.Env());

        if (!error_str.empty())
        {
            // got an error? break then
            break;
        }
        result.Set(i, result_printer);
    }
    cupsFreeDests(printers_size, printers);
    if (!error_str.empty())
    {
        // got an error? return the error then
        RETURN_EXCEPTION_STR(info.Env(), error_str.c_str());
    }
    return result;
}

Napi::Value PrintDirect(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(info, 5, info.Env());

    // can be string or buffer
    if (info.Length() <= 0)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Argument 0 missing");
    }

    std::string data;
    Napi::Value arg0(info.Env(), info[0]);
    if (!getStringOrBufferFromNapiValue(arg0, data))
    {
        RETURN_EXCEPTION_STR(info.Env(), "Argument 0 must be a string or Buffer");
    }
    REQUIRE_ARGUMENT_STRING(info, 1, printername);
    REQUIRE_ARGUMENT_STRING(info, 2, docname);
    REQUIRE_ARGUMENT_STRING(info, 3, type);
    REQUIRE_ARGUMENT_OBJECT(info, 4, print_options);

    std::string type_str(type);
    FormatMapType::const_iterator itFormat = getPrinterFormatMap().find(type_str);
    if (itFormat == getPrinterFormatMap().end())
    {
        RETURN_EXCEPTION_STR(info.Env(), "unsupported format type");
    }
    type_str = itFormat->second;

    CupsOptions options(print_options);

    int job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, printername.c_str(), docname.c_str(), options.getNumOptions(), options.get());
    if (job_id == 0)
    {
        RETURN_EXCEPTION_STR(info.Env(), cupsLastErrorString());
    }

    if (HTTP_CONTINUE != cupsStartDocument(CUPS_HTTP_DEFAULT, printername.c_str(), job_id, docname.c_str(), type_str.c_str(), 1 /*last document*/))
    {
        RETURN_EXCEPTION_STR(info.Env(), cupsLastErrorString());
    }

    /* cupsWriteRequestData can be called as many times as needed */
    //TODO: to split big buffer
    if (HTTP_CONTINUE != cupsWriteRequestData(CUPS_HTTP_DEFAULT, data.c_str(), data.size()))
    {
        cupsFinishDocument(CUPS_HTTP_DEFAULT, printername.c_str());
        RETURN_EXCEPTION_STR(info.Env(), cupsLastErrorString());
    }

    cupsFinishDocument(CUPS_HTTP_DEFAULT, printername.c_str());
    return Napi::Number::New(info.Env(), job_id);
}

Napi::Value PrintFile(const Napi::CallbackInfo &info)
{

    REQUIRE_ARGUMENTS(info, 3, info.Env());

    // can be string or buffer
    if (info.Length() <= 0)
    {
        RETURN_EXCEPTION_STR(info.Env(), "Argument 0 missing");
    }

    REQUIRE_ARGUMENT_STRING(info, 0, filename);
    REQUIRE_ARGUMENT_STRING(info, 1, docname);
    REQUIRE_ARGUMENT_STRING(info, 2, printer);
    REQUIRE_ARGUMENT_OBJECT(info, 3, print_options);

    CupsOptions options(print_options);

    int job_id = cupsPrintFile(printer.c_str(), filename.c_str(), docname.c_str(), options.getNumOptions(), options.get());

    if (job_id == 0)
    {
        return Napi::String::New(info.Env(), cupsLastErrorString());
    }
    else
    {
        return Napi::Number::New(info.Env(), job_id);
    }
}

Napi::Value getSupportedJobCommands(const Napi::CallbackInfo &info)
{
    Napi::Array result = Napi::Array::New(info.Env());
    int i = 0;
    result.Set(i++, Napi::String::New(info.Env(), "CANCEL"));
    return result;
}
