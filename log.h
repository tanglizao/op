#ifndef __LOG_H__
#define __LOG_H__

#include <sys/types.h>
#include <time.h>
#include <string>
#include <stdio.h>
#include <dirent.h>
#include "noncopyable.h"
#include "util.h"

class Log : private NonCopyable
{
private:
    uint _level;
    bool _flush;
    bool _print;
    bool _add_time;

    FILE * _handle;
    std::string _prefix;
    int _today;
    size_t _max_fsize;
    size_t _fsize;
    uint _sequence;
    Mutex _mutex;

private:
    void close(FILE *& f);
    int maxsequence(const std::string & prefix);
    std::string init(const std::string & prefix, size_t max_fsize);
    std::string rollPath(const std::string & prefix, uint sequence);
    void roll(FILE *& f, const std::string & prefix, uint sequence, size_t * fsize);
    bool open(FILE *& f, const char * path);
    void output(const char * msg, size_t len);
    int addtime(char * str);
    std::string date();
    int today();

public:
    Log();
    ~Log();

public:
    bool create(const char * prefix
                , uint level = 1
                , bool flush = true
                , bool print = false
                , bool add_time = true
                , size_t max_fsize = (1 << 30));
    void out(uint level, const std::string & msg);
    void out(uint level, size_t len, const char * msg);
    void out(uint level, const char * format, ...);

    FILE * handle();
    void level(uint level);
    uint level();
};

/* -------------------------------------------------------------------------- */

#define MAX_LOG_LEN 16384
#define VASTR(buf, bufsize, fmtlen, format)	\
{ \
	va_list	va; \
	va_start(va, format); \
	fmtlen = vsnprintf(buf, bufsize, format, va); \
	va_end(va); \
	if( fmtlen >= bufsize ) { \
		*(buf + bufsize - 1) = '\0'; \
		fmtlen = bufsize - 1; \
	} \
}

Log::Log() : _level(1), _flush(true), _print(false), _add_time(true),
_handle(NULL), _today(0), _max_fsize(1 << 30), _fsize(0), _sequence(0)
{
}

Log::~Log()
{
    close(_handle);
}

bool Log::create(const char * prefix
                 , uint level
                 , bool flush
                 , bool print
                 , bool add_time
                 , size_t max_fsize)
{
    _level = level;
    _flush = flush;
    _print = print;
    _add_time = add_time;
    _prefix = prefix;
    _max_fsize = max_fsize;
    _today = today();
    std::string path = init(_prefix, _max_fsize);

    if (path.empty()) {
        return false;
    }

    if (!open(_handle, path.c_str())) {
        return false;
    }

    _fsize = Util::FileSize(path);
    return true;
}

bool Log::open(FILE *& f, const char * path)
{
    return (NULL != (f = fopen(path, "a")));
}

std::string Log::init(const std::string & prefix, size_t max_fsize)
{
    int sequence = maxsequence(prefix);

    if (-2 == sequence)
        return std::string();

    if (-1 == sequence) {
        sequence = 0;
        return prefix + date() + "_0.log";
    }

    std::string path = prefix + date() + "_" + Util::itoa(sequence) + ".log";
    size_t fsize = Util::FileSize(path.c_str());
    _sequence = (fsize < max_fsize ? sequence : sequence + 1);
    return prefix + date() + "_" + Util::itoa(_sequence) + ".log";
}

std::string Log::rollPath(const std::string & prefix, uint sequence)
{
    return prefix + date() + "_" + Util::itoa(sequence) + ".log";
}

int Log::maxsequence(const std::string & prefix)
{
    std::string prefix_dir = prefix.substr(0, prefix.find_last_of('/') + 1);
    std::string prefix_name = prefix.substr(prefix.find_last_of('/') + 1) + date() + "_";

    DIR * dir = opendir(prefix_dir.c_str());
    if (NULL == dir) return -2;

    int no = -1, maxno = -1;
    std::string name;
    struct dirent * ptr = NULL;

    while ((ptr = readdir(dir))) {
        if ((DT_REG == ptr->d_type) && !strncmp(ptr->d_name, prefix_name.c_str(), prefix_name.length())) {
            name = ptr->d_name + prefix_name.length();
            no = atoi(name.substr(0, name.find_first_of('.')).c_str());

            if (no > maxno) {
                maxno = no;
            }
        }
    }
    closedir(dir);

    return maxno;
}

void Log::close(FILE *& f)
{
    if (NULL != f) {
        fclose(f);
        f = NULL;
    }
}

void Log::roll(FILE *& f, const std::string & prefix, uint sequence, size_t * fsize)
{
    close(f);

    std::string path = rollPath(prefix, sequence);
    if (!open(f, path.c_str())) {
        fprintf(stderr, "err:%d Can't create new log file %s\n", errno, path.c_str());
        return;
    }

    *fsize = 0;
}

void Log::out(uint level, const std::string & msg)
{
    out(level, msg.length(), msg.c_str());
}

void Log::out(uint level, size_t len, const char * msg)
{
    if (_level < level) {
        return;
    }

    char tmp[MAX_LOG_LEN];
    int time_len = 0;
    if (_add_time) {
        time_len = addtime(tmp);
        tmp[time_len++] = ' ';
    }

    int fmtlen = 0;
    if (len < (sizeof (tmp) - time_len)) {
        memcpy(tmp + time_len, msg, len + 1);
        fmtlen = len + time_len;
    }
    else {
        memcpy(tmp + time_len, msg, sizeof (tmp) - time_len - 2);
        tmp[sizeof (tmp) - 2] = '\n';
        tmp[sizeof (tmp) - 1] = '\0';
        fmtlen = sizeof (tmp) - 1;
    }

    output(tmp, fmtlen);
}

void Log::out(uint level, const char * format, ...)
{
    if (_level < level) {
        return;
    }

    char tmp[MAX_LOG_LEN];
    int time_len = 0;
    if (_add_time) {
        time_len = addtime(tmp);
        tmp[time_len++] = ' ';
    }

    size_t fmtlen = 0;
    VASTR(tmp + time_len, sizeof (tmp) - time_len, fmtlen, format)
    output(tmp, fmtlen + time_len);
}

void Log::output(const char * msg, size_t len)
{
    MLOCK lock(_mutex);

    if (today() != _today) {
        _sequence = 0;
        roll(_handle, _prefix, _sequence, &_fsize);
        _today = today();
    }

    if (_fsize >= _max_fsize) {
        _sequence++;
        roll(_handle, _prefix, _sequence, &_fsize);
    }

    _fsize += len;

    if (_handle) fprintf(_handle, "%s", msg);
    if (_flush) fflush(_handle);
    if (_print) printf("%s", msg);
}

int Log::today()
{
    time_t now;
    time(&now);
    struct tm res;
    return localtime_r(&now, &res)->tm_mday;
}

int Log::addtime(char * str)
{
    time_t now;
    struct tm res;

    time(&now);
    localtime_r(&now, &res);

    return sprintf(str, "%02d:%02d:%02d",
                   res.tm_hour,
                   res.tm_min,
                   res.tm_sec);
}

std::string Log::date()
{
    time_t now;
    struct tm res;
    char str[20] = "";

    time(&now);
    localtime_r(&now, &res);
    sprintf(str, "%04d-%02d-%02d", res.tm_year + 1900, res.tm_mon + 1, res.tm_mday);
    return str;
}

FILE * Log::handle()
{
    return _handle;
}

void Log::level(uint level)
{
    _level = level;
}

uint Log::level()
{
    return _level;
}

#endif /* __LOG_H__ */
