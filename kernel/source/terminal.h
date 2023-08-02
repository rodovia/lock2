#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#include "dllogic/api/dhelp.h"

#define SEVERITY_INFO 1
#define SEVERITY_WARN 2
#define SEVERITY_ERROR 3

#define Info(M, ...) CTerminal::SeverityFormatted(SEVERITY_INFO, M __VA_OPT__(,) __VA_ARGS__)
#define Warn(M, ...) CTerminal::SeverityFormatted(SEVERITY_WARN, M __VA_OPT__(,) __VA_ARGS__)
#define Error(M, ...) CTerminal::SeverityFormatted(SEVERITY_ERROR, M __VA_OPT__(,) __VA_ARGS__)

enum class escape_state
{
    UNKNOWN,
    FOUND_ESCAPE,
    FOUND_DSIGN,
    FOUND_CHAR,
    FOUND_LPAREN,
    SWITCH_COLORS
};

enum fmt_state
{
    kTerminalFmtStateSpecifParse,
    kTerminalFmtStateNormal,
};

class CTerminal : public IDHelpTerminal
{
public:
    static CTerminal& GetInstance() noexcept
    {
        static CTerminal t;
        return t;
    }

    ~CTerminal() override
    {

    }

    static void Write(const char* st, int length);
    static void Write(const char* st);
    static void Severity(int sev, const char* st, int length);
    static void Severity(int sev, const char* st);
    static void SeverityFormatted(int sev, const char* st, ...);
    static void WriteFormatted(const char* st, ...);
    static void WriteFormatted(const char* st, va_list args);

    void WriteString(const char* st, size_t length) override;
    void WriteFormat(const char* st, ...) override;
    static void PrepareForAbort();
private:
    CTerminal();

    
    void WriteChar(char c);
    bool ThinkEscape(char c);

    char m_Hex[6];
    uint8_t m_HexPointer;
    fmt_state m_FormatState = fmt_state::kTerminalFmtStateNormal;
    escape_state m_EscState = escape_state::UNKNOWN;
};

