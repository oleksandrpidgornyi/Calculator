#pragma once

#define COLOR_END           "\033[0m"
#define COLOR_RED_TEXT      "\x1B[31m"
#define COLOR_GREEN_TEXT    "\x1B[32m"
#define COLOR_YELLOW_TEXT   "\x1B[33m"
#define COLOR_BLUE_TEXT     "\x1B[34m"
#define COLOR_MAGENTA_TEXT  "\x1B[35m"
#define COLOR_L_BLUE_TEXT   "\x1B[36m"
#define COLOR_WHITE_TEXT    "\x1B[37m"
#define COLOR_L_YELLOW_TEXT "\x1B[93m"

#define COLOR_RED_BKG     "\033[3;41;30m"
#define COLOR_GREEN_BKG   "\033[3;42;30m"
#define COLOR_BLUE_BKG    "\033[3;44;30m"
#define COLOR_L_BLUE_BKG  "\033[3;104;30m"

#define COLOR_WHITE_BKG_MAGENTA_TEXT  "\033[3;47;35m"
#define COLOR_WHITE_BKG_L_MGNTA_TEXT  "\033[2;47;35m"
#define COLOR_WHITE_BKG_RED_TEXT      "\033[1;47;35m"

#ifdef _DEBUG

#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include "TSingletone.hpp"

using namespace std;

#define LOG_STR_LEN 1024

//log levels
#define LOG_NONE   0x00
#define LOG_TRACE  0x01 //verbose
#define LOG_DEBUG  0x02
#define LOG_INFO   0x04
#define LOG_WARN   0x08
#define LOG_ERROR  0x10
#define LOG_FATAL  0x20
#define LOG_ALL_LEVELS  (LOG_TRACE | LOG_DEBUG | LOG_INFO | LOG_WARN | LOG_ERROR | LOG_FATAL)
//log columns
#define LOG_FILE_NAME  0x01
#define LOG_FUNC_MAME  0x02
#define LOG_LINE_NUM   0x04
#define LOG_TIME_STAMP 0x08
#define LOG_PROC_ID    0x10
#define LOG_THREAD_ID  0x20
#define LOG_LOG_NUM    0x40
#define LOG_ALL_COLUMNS (LOG_FILE_NAME | LOG_FUNC_MAME | LOG_LINE_NUM | LOG_TIME_STAMP | LOG_PROC_ID | LOG_THREAD_ID | LOG_LOG_NUM)

enum class LOG_FORMATTER {
    TEXT,
    COLORTEXT,
    EXCEL
};

class CLogWriter {
public:
    CLogWriter() {}
    virtual ~CLogWriter() {}
    virtual void Write(const char* buff, const int len) = 0;
    virtual void Write(const string& sMessage) = 0;
};

class CConsoleWriter : public CLogWriter {
public:
    CConsoleWriter() : CLogWriter() {}
    virtual ~CConsoleWriter() {}
    void Write(const char* buff, const int len) override;
    void Write(const string& sMessage) override;
};

class CFileWriter : public CLogWriter {
public:
    CFileWriter() = delete;
    CFileWriter(const string& name);
    virtual ~CFileWriter();
    void Write(const char* buff, const int len) override;
    void Write(const string& sMessage) override;
private:
    ofstream m_file;
};

class CLogger final : public TSingleton<CLogger> {
    friend class TSingleton<CLogger>;
public:
    CLogger(CLogger const&) = delete;            // Copy construct
    CLogger(CLogger&&) = delete;                 // Move construct
    CLogger& operator=(CLogger const&) = delete; // Copy assign
    CLogger& operator=(CLogger&&) = delete;      // Move assign

    void AddWriter(CLogWriter* lw);

    void Dump(const void* data, unsigned int len);
    void Log(int level, const char* file, const char* function, int line, const char* format, ...);

    void SetLevelMask(uint32_t mask) { m_level_mask = mask; }
    void SetFormatMask(uint32_t mask) { m_format_mask = mask; }
    void SetFormatter(LOG_FORMATTER val);

    friend bool CheckLevelMask(uint32_t mask);
private:
#ifdef _WIN32
    void *hStdout;
    unsigned long consoleMode;
#endif //_WIN32
    mutex m_Mutex;
    vector<shared_ptr<CLogWriter>> m_Wr;
    LOG_FORMATTER m_formatter = LOG_FORMATTER::TEXT;
    inline static uint32_t m_level_mask  = LOG_ERROR | LOG_FATAL;
    inline static uint32_t m_format_mask = LOG_FILE_NAME | LOG_FUNC_MAME | LOG_LINE_NUM;
    inline static uint32_t m_line_num    = 0;
    CLogger();
    ~CLogger();
    int TextFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line);
    int ColorTextFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line);
    int ExcelFormatter(char buffer[LOG_STR_LEN], int level, const char* file, const char* function, int line);
    int printTime(char buffer[LOG_STR_LEN], int len);
    int printThreadID(char buffer[LOG_STR_LEN], int len);
    int printProcessID(char buffer[LOG_STR_LEN], int len);
};

bool CheckLevelMask(uint32_t mask);
void LogInitConsole(LOG_FORMATTER formatter);
void LogInitColorConsole();
void LogInitTextFile(const string& name);

//The value of __FILE__ is the file path as specified on the compiler's command line. 
#define LOGT(...) if (CheckLevelMask(LOG_TRACE)) CLogger::GetInstance().Log(LOG_TRACE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGD(...) if (CheckLevelMask(LOG_DEBUG)) CLogger::GetInstance().Log(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGI(...) if (CheckLevelMask(LOG_INFO))  CLogger::GetInstance().Log(LOG_INFO,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGW(...) if (CheckLevelMask(LOG_WARN))  CLogger::GetInstance().Log(LOG_WARN,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGE(...) if (CheckLevelMask(LOG_ERROR)) CLogger::GetInstance().Log(LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGF(...) if (CheckLevelMask(LOG_FATAL)) CLogger::GetInstance().Log(LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define LOG_INIT_COLORCONSOLE LogInitColorConsole()

#else  //_DEBUG

#define LOGT(...)
#define LOGD(...)
#define LOGI(...)
#define LOGW(...)
#define LOGE(...)
#define LOGF(...)

#define LOG_INIT_COLORCONSOLE

#endif //_DEBUG
