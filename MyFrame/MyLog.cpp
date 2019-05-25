#include "MyLog.h"
#include <stdarg.h>

MyLog* MyLog::s_inst = nullptr;

MyLog::MyLog() :
    m_ll(MYLL_INFO),
    m_log_file(stdout)
{
}

MyLog::~MyLog()
{}

MyLog* MyLog::Inst()
{
    if(s_inst == nullptr)
        s_inst = new MyLog();
    return s_inst;
}

bool MyLog::LogPrefix(enum ENUM_MYLOG_LEVEL level, const char *fname, int line)
{
    if (level > m_ll) return false;
    if (m_log_file == NULL) m_log_file = stderr;
    return true;
}

void MyLog::Log(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(m_log_file, fmt, ap);
    va_end(ap);
    fputc('\n', m_log_file);
    fflush(m_log_file);
}
