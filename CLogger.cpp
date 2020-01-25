#ifdef _DEBUG

#ifdef _WIN32
#include <windows.h>
#else //_WIN32
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#endif //_WIN32

#include <cstdarg>
#include <chrono>
#include <stdio.h>
#include <ctype.h>

#include "CLogger.h"

bool CheckLevelMask(uint32_t mask)
{
    return CLogger::m_level_mask & mask;
}

void LogInitConsole(LOG_FORMATTER formatter)
{
    CLogger::GetInstance().AddWriter(dynamic_cast<CLogWriter*>(new CConsoleWriter));
    CLogger::GetInstance().SetLevelMask(LOG_ALL_LEVELS);
    CLogger::GetInstance().SetFormatMask(LOG_FUNC_MAME | LOG_LINE_NUM | LOG_LOG_NUM);
    CLogger::GetInstance().SetFormatter(formatter);
}

void LogInitColorConsole()
{
    CLogger::GetInstance().AddWriter(dynamic_cast<CLogWriter*>(new CConsoleWriter));
    CLogger::GetInstance().SetFormatMask(LOG_FUNC_MAME | LOG_LINE_NUM | LOG_LOG_NUM);
    CLogger::GetInstance().SetFormatMask(LOG_ALL_COLUMNS);
    CLogger::GetInstance().SetFormatter(LOG_FORMATTER::COLORTEXT);
}

void LogInitTextFile(const string& name)
{
    CLogger::GetInstance().AddWriter(dynamic_cast<CLogWriter*>(new CFileWriter(name)));
    CLogger::GetInstance().SetLevelMask(LOG_ALL_LEVELS);
    CLogger::GetInstance().SetFormatMask(LOG_FUNC_MAME | LOG_LINE_NUM | LOG_TIME_STAMP | LOG_PROC_ID | LOG_THREAD_ID | LOG_LOG_NUM);
    CLogger::GetInstance().SetFormatter(LOG_FORMATTER::COLORTEXT);
}

void CLogger::AddWriter(CLogWriter* lw)
{
    m_Wr.push_back(shared_ptr<CLogWriter>(lw));
}

CLogger::CLogger()
{
#ifdef _WIN32
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdout, &consoleMode);
    SetConsoleMode(hStdout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif //_WIN32
}
CLogger::~CLogger()
{
#ifdef _WIN32
    SetConsoleMode(hStdout, consoleMode);
#endif //_WIN32
}

void CLogger::SetFormatter(LOG_FORMATTER val)
{
    if (val > LOG_FORMATTER::EXCEL) {
        return;
    }
    m_formatter = val;
}

void CLogger::Log(int level, const char* file, const char* function, int line, const char* format, ...)
{
    lock_guard lock(m_Mutex);
    char buffer[LOG_STR_LEN];
    int len = 0;
    va_list args;
    va_start(args, format);
    switch (m_formatter) {
    case LOG_FORMATTER::TEXT:
        len = TextFormatter(buffer, level, file, function, line);
        len += vsnprintf(buffer + len, LOG_STR_LEN, format, args);
        break;
    case LOG_FORMATTER::COLORTEXT:
        len = ColorTextFormatter(buffer, level, file, function, line);
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_L_YELLOW_TEXT);
        len += vsnprintf(buffer + len, LOG_STR_LEN, format, args);
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_END);
        break;
    case LOG_FORMATTER::EXCEL:
        len = ExcelFormatter(buffer, level, file, function, line);
        len += vsnprintf(buffer + len, LOG_STR_LEN, format, args);
        break;
    default:
        return;
    }
    for (auto const& Wr : m_Wr) {
        Wr->Write(buffer, len);
    }
    va_end(args);
}

int CLogger::printTime(char buffer[LOG_STR_LEN], int len)
{
#ifdef _WIN32
	SYSTEMTIME time;
	GetSystemTime(&time);
	len += snprintf(buffer + len, LOG_STR_LEN, "%02d:%02d:%02d:%04d ",
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
#else //_WIN32
	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tm=localtime(&tv.tv_sec);
	int32_t ms = (tv.tv_usec / 1000);
	len += snprintf(buffer + len, LOG_STR_LEN, "%02d:%02d:%02d:%04d ",
			tm->tm_hour, tm->tm_min, tm->tm_sec, ms);
#endif //_WIN32
    return len;
}

int CLogger::printThreadID(char buffer[LOG_STR_LEN], int len)
{
#ifdef _WIN32
    DWORD tid = GetCurrentThreadId();
#else //_WIN32
    pid_t tid = syscall(SYS_gettid);
#endif //_WIN32
    len += snprintf(buffer + len, LOG_STR_LEN, "tid:%d ", tid);
    return len;
}

int CLogger::printProcessID(char buffer[LOG_STR_LEN], int len)
{
#ifdef _WIN32
    DWORD pid = GetProcessId(GetCurrentProcess());
#else //_WIN32
    pid_t pid = getpid();
#endif //_WIN32
    len += snprintf(buffer + len, LOG_STR_LEN, "pid:%d ", pid);
    return len;
}

int CLogger::TextFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line)
{
    int len = 0;
    if (CLogger::m_format_mask & LOG_LOG_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%d ", CLogger::m_line_num++);
    }
    if (CLogger::m_format_mask & LOG_TIME_STAMP) {
    	len = printTime(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_PROC_ID) {
    	len = printProcessID(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_THREAD_ID) {
    	len = printThreadID(buffer, len);
    }
    switch (level) {
    case LOG_TRACE:
        len += snprintf(buffer + len, LOG_STR_LEN, "(TRACE: ");
        break;
    case LOG_DEBUG:
        len += snprintf(buffer + len, LOG_STR_LEN, "(DEBUG: ");
        break;
    case LOG_INFO:
        len += snprintf(buffer + len, LOG_STR_LEN, "(INFO : ");
        break;
    case LOG_WARN:
        len += snprintf(buffer + len, LOG_STR_LEN, "(WARN : ");
        break;
    case LOG_ERROR:
        len += snprintf(buffer + len, LOG_STR_LEN, "(ERROR: ");
        break;
    case LOG_FATAL:
        len += snprintf(buffer + len, LOG_STR_LEN, "(FATAL: ");
        break;
    default:
        break;
    }
    if (CLogger::m_format_mask & LOG_FILE_NAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%s ", file);
    }
    if (CLogger::m_format_mask & LOG_FUNC_MAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%s", function);
    }
    if (CLogger::m_format_mask & LOG_LINE_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, ":%d ", line);
    }
    else {
        len += snprintf(buffer + len, LOG_STR_LEN, " ");
    }
    return len;
}

int CLogger::ColorTextFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line)
{
    int len = 0;
    if (CLogger::m_format_mask & LOG_LOG_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_BLUE_TEXT "%d " COLOR_END, CLogger::m_line_num++);
    }
    if (CLogger::m_format_mask & LOG_TIME_STAMP) {
    	len = printTime(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_PROC_ID) {
    	len = printProcessID(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_THREAD_ID) {
    	len = printThreadID(buffer, len);
    }
    switch (level) {
    case LOG_TRACE:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_WHITE_TEXT "(TRACE: " COLOR_END);
        break;
    case LOG_DEBUG:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_GREEN_TEXT "(DEBUG: " COLOR_END);
        break;
    case LOG_INFO:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_GREEN_TEXT "(INFO : " COLOR_END);
        break;
    case LOG_WARN:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_GREEN_TEXT "(WARN : " COLOR_END);
        break;
    case LOG_ERROR:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_RED_TEXT "(ERROR: " COLOR_END);
        break;
    case LOG_FATAL:
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_RED_TEXT "(FATAL: " COLOR_END);
        break;
    default:
        break;
    }
    if (CLogger::m_format_mask & LOG_FILE_NAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_L_BLUE_TEXT "%s " COLOR_END, file);
    }
    if (CLogger::m_format_mask & LOG_FUNC_MAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_L_BLUE_TEXT "%s" COLOR_END, function);
    }
    if (CLogger::m_format_mask & LOG_LINE_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, COLOR_L_BLUE_TEXT ":%d " COLOR_END, line);
    }
    else {
        len += snprintf(buffer + len, LOG_STR_LEN, " ");
    }

    return len;
}

int CLogger::ExcelFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line)
{
    int len = 0;
    if (CLogger::m_format_mask & LOG_LOG_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%d;", CLogger::m_line_num++);
    }
    if (CLogger::m_format_mask & LOG_TIME_STAMP) {
    	len = printTime(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_PROC_ID) {
    	len = printProcessID(buffer, len);
    }
    if (CLogger::m_format_mask & LOG_THREAD_ID) {
    	len = printThreadID(buffer, len);
    }
    switch (level) {
    case LOG_TRACE:
        len += snprintf(buffer + len, LOG_STR_LEN, "(TRACE:;");
        break;
    case LOG_DEBUG:
        len += snprintf(buffer + len, LOG_STR_LEN, "(DEBUG:;");
        break;
    case LOG_INFO:
        len += snprintf(buffer + len, LOG_STR_LEN, "(INFO :;");
        break;
    case LOG_WARN:
        len += snprintf(buffer + len, LOG_STR_LEN, "(WARN :;");
        break;
    case LOG_ERROR:
        len += snprintf(buffer + len, LOG_STR_LEN, "(ERROR:;");
        break;
    case LOG_FATAL:
        len += snprintf(buffer + len, LOG_STR_LEN, "(FATAL:;");
        break;
    default:
        break;
    }
    if (CLogger::m_format_mask & LOG_FILE_NAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%s;", file);
    }
    if (CLogger::m_format_mask & LOG_FUNC_MAME) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%s;", function);
    }
    if (CLogger::m_format_mask & LOG_LINE_NUM) {
        len += snprintf(buffer + len, LOG_STR_LEN, "%d;", line);
    }
    return len;
}

// Takes a pointer to an arbitrary chunk of data and prints the first-len bytes.
void CLogger::Dump(const void* data, unsigned int len)
{
    printf("Size:  %d\n", len);

    if (len > 0) {
        unsigned width = 16;
        unsigned char* str = (unsigned char*)data;
        unsigned int j, i = 0;

        while (i < len) {
            printf(" ");

            for (j = 0; j < width; j++) {
                if (i + j < len)
                    printf("%02x ", (unsigned char)str[j]);
                else
                    printf("   ");

                if ((j + 1) % (width / 2) == 0)
                    printf(" -  ");
            }

            for (j = 0; j < width; j++) {
                if (i + j < len)
                    if (isprint(str[j])) {
                        printf("%c", str[j]);
                    }
                    else {
                        printf("%c", '.');
                    }
                else
                    printf(" ");
            }

            str += width;
            i += j;

            printf("\n");
        }
    }
}

//Writer classes
void CConsoleWriter::Write(const char* buff, const int len)
{
    printf("%s", buff);
}

void CConsoleWriter::Write(const string& sMessage)
{
    cout << sMessage;
}

CFileWriter::CFileWriter(const string& name)
{
    m_file.open(name);
}

CFileWriter::~CFileWriter()
{
    m_file.close();
}

void CFileWriter::Write(const char* buff, const int len)
{
    string sMessage(buff, len);
    m_file << sMessage;
}

void CFileWriter::Write(const string& sMessage)
{
    m_file << sMessage;
}

#endif //_DEBUG
