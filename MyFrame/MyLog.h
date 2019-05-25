#ifndef __MYLOG_H__
#define __MYLOG_H__

#include <mutex>
#include <string.h>
#include <errno.h>

enum ENUM_MYLOG_LEVEL{
    MYLL_NONE    = -1,
    MYLL_ERROR   =  0,
    MYLL_WARN    =  1,
    MYLL_INFO    =  2,
    MYLL_DEBUG   =  3,
    MYLL_MIN     = -2,
    MYLL_MAX     =  4,
};

class MyLog
{
public:  
    ~MyLog();

    static MyLog* Inst();

    void SetLogLV(enum ENUM_MYLOG_LEVEL lv);

    void SetLogFile(FILE* fp);

    bool LogPrefix(enum ENUM_MYLOG_LEVEL level, const char *fname, int line);

    void Log(const char* fmt, ...);
private:
    MyLog();

    enum ENUM_MYLOG_LEVEL  m_ll;
    FILE*                  m_log_file;

    static MyLog* s_inst;
};

#define MYLOG(l, x)                                            \
    do {                                                       \
        if (MyLog::Inst()->LogPrefix(l, __FILE__, __LINE__)) { \
            MyLog::Inst()->Log x;                              \
        }                                                      \
    } while (0)

#endif // __MYLOG_H__
