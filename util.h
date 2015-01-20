#ifndef __UTIL_H__
#define	__UTIL_H__

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class Util
{
public:

    static size_t FileSize(const char * path)
    {
        struct stat st;
        return stat(path, &st) ? 0 : st.st_size;
    }

    static size_t FileSize(const std::string & path)
    {
        return FileSize(path.c_str());
    }

    static std::string itoa(int i)
    {
        char buf[16] = "";
        sprintf(buf, "%d", i);
        return buf;
    }

    static std::string itoa(uint i)
    {
        char buf[16] = "";
        sprintf(buf, "%u", i);
        return buf;
    }
};

#endif	/* __UTIL_H__ */
