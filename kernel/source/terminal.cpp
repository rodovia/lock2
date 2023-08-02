#include "terminal.h"
#include "alloc/physical.h"

#define SSFN_CONSOLEBITMAP_TRUECOLOR 1
#include "ssfn.h"
#include "requests.h"
#include "limine.h"
#include <cstdarg>
#include <string.h>

#define SEVERITY_INFO_COLOR 0x0048BA
#define SEVERITY_WARN_COLOR 0xF4CA16
#define SEVERITY_ERROR_COLOR 0xFE2712
#define ABORT_BG_COLOR 0xDC143C

#define MAXIMUM_BUFFER 64

extern const unsigned char terminalFont[];

static void ClearScreen(uint32_t clr)
{
    uint32_t height = ssfn_dst.h, 
             width = ssfn_dst.w;
    uint32_t* final;
    for (uint64_t i = 0; i < height; i++)
    {
        final = reinterpret_cast<uint32_t*>((uint64_t)ssfn_dst.ptr + (i * ssfn_dst.p));
        for (uint64_t c = 0; c < width; c++)
        {
            final[c] = clr;
        }
    }
}

static void ReverseString(char* str)
{
	int len;
	int index;
	char* start, *end, temp;

	len = strlen(str);

	start = str;
	end = str+len-1;

    for (index = 0; index < len / 2; index++) 
    { 
		temp = *end;
		*end = *start;
		*start = temp;

		start++; 
		end--;
	}
}

static char* NumberToString(uint64_t number, char* str, int base)
{
    int i = 0;
    bool neg = false;

    if (number == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (number != 0)
    {
        int rem = number % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        number = number / base;
    }

    if (neg)
    {
        str[i++] = '-';
    }
    str[i] = '\0';
    ReverseString(str);
    return str;
}

static void WriteNumber(unsigned long number, unsigned short base)
{
    char bf[MAXIMUM_BUFFER];
    NumberToString(number, bf, base);
    CTerminal::Write(bf);
}

CTerminal::CTerminal()
{
    struct limine_framebuffer_response* fbs = rqs::GetFramebuffer();
    struct limine_framebuffer* fb = fbs->framebuffers[0];
    ssfn_src = reinterpret_cast<ssfn_font_t*>(const_cast<unsigned char*>(terminalFont));
    ssfn_dst.bg = 0x0;
    ssfn_dst.fg = 0xAAAAAA;
    ssfn_dst.h = fb->height;
    ssfn_dst.w = fb->width;
    ssfn_dst.p = fb->pitch;
    ssfn_dst.ptr = reinterpret_cast<uint8_t*>(fb->address);
}

void CTerminal::Write(const char* st, int length)
{
    CTerminal inst = CTerminal::GetInstance();
    inst.WriteString(st, length);
}

void CTerminal::Write(const char* st)
{
    CTerminal::Write(st, strlen(st));
}

void CTerminal::WriteChar(char c)
{
    char cbyte[2];
    (*cbyte) = c;
    cbyte[1] = '\0';
    this->WriteString(cbyte, 1);
}

void CTerminal::WriteString(const char *st, size_t length)
{
    int finalw = ssfn_dst.w - ssfn_src->width;
    for (size_t i = 0; i < length; i++)
    {
        char c = st[i];
        if (c == '\n' || ssfn_dst.x >= finalw)
        {
            ssfn_dst.y += ssfn_src->height;
        }

        if (c == '\n' || c == '\r' || ssfn_dst.x >= finalw)
        {
            ssfn_dst.x = 0;
            continue;
        }

        if (c == '\t')
        {
            ssfn_dst.x = ssfn_src->width * 4;
            continue;
        }

        if (ssfn_dst.h - ssfn_dst.y < ssfn_src->height)
        {
            uint32_t* fb = reinterpret_cast<uint32_t*>(ssfn_dst.ptr);
            void* dest;
            void* src;
            for (int i = ssfn_src->height; i != ssfn_dst.h; ++i)
            {
                dest = PaAdd(fb, (i - ssfn_src->height) * ssfn_dst.p);
                src = PaAdd(fb, i * ssfn_dst.p);
                memcpy(dest, src, ssfn_dst.w * sizeof(uint32_t));
                ssfn_dst.x = 0;
                ssfn_dst.y = ssfn_dst.h - ssfn_src->height;
            }

            /* Erase old line */
            for (int i = ssfn_dst.y; i != ssfn_dst.h; ++i)
            {
                dest = PaAdd(fb, i * ssfn_dst.p);
                memset(dest, 0, ssfn_dst.w * sizeof(uint32_t));
            }
        }
        
        ssfn_putc(c);
    }
}

bool CTerminal::ThinkEscape(char c)
{
    switch(c)
    {
    case '\e':
        if (m_EscState == escape_state::UNKNOWN)
        {
            m_EscState = escape_state::FOUND_ESCAPE;
            return false; /* Do not print */
        }
        return true;
    case '$':
        if (m_EscState == escape_state::FOUND_ESCAPE)
        {
            m_EscState = escape_state::FOUND_DSIGN;
            return false;
        }
        return true;
    default:
        break;
    }

    if (m_HexPointer > 5)
    {
        m_EscState = escape_state::FOUND_LPAREN;
        return false; /* Expression is too big. Truncate. */
    }

    if (c < 'f' || c < 'F')
    {
        m_EscState = escape_state::FOUND_CHAR;
        m_Hex[m_HexPointer] = c;
    }

    if (c == ')' && 
        m_EscState == escape_state::FOUND_CHAR)
    {
        m_EscState = escape_state::SWITCH_COLORS;
        return false;
    }

    return true;
}

void CTerminal::Severity(int sev, const char* st, int length)
{
    CTerminal& term = CTerminal::GetInstance();
    unsigned int color = ssfn_dst.fg;
    switch (sev)
    {
    case SEVERITY_INFO:
        color = SEVERITY_INFO_COLOR;
        CTerminal::Write("info: ");
        break;
    case SEVERITY_WARN:
        color = SEVERITY_WARN_COLOR;
        CTerminal::Write("warning! ");
        break;
    case SEVERITY_ERROR:
        color = SEVERITY_ERROR_COLOR;
        CTerminal::Write("ERROR: ");
        break;
    }

    ssfn_dst.fg = color;
    term.WriteString(st, length);
    ssfn_dst.fg = 0xAAAAAA;
}

void CTerminal::Severity(int sev, const char* st)
{
    CTerminal::Severity(sev, st, strlen(st));
}

void CTerminal::SeverityFormatted(int sev, const char* st, ...)
{
    unsigned int color = ssfn_dst.fg;
    switch (sev)
    {
    case SEVERITY_INFO:
        color = SEVERITY_INFO_COLOR;
        CTerminal::Write("info: ");
        break;
    case SEVERITY_WARN:
        color = SEVERITY_WARN_COLOR;
        CTerminal::Write("warning! ");
        break;
    case SEVERITY_ERROR:
        color = SEVERITY_ERROR_COLOR;
        CTerminal::Write("ERROR: ");
        break;
    }

    va_list ls;
    va_start(ls, st);
    ssfn_dst.fg = color;
    CTerminal::WriteFormatted(st, ls);
    ssfn_dst.fg = 0xAAAAAA;
    va_end(ls);
    
    CTerminal::Write("\n");
}

void CTerminal::WriteFormatted(const char* st, ...)
{
    va_list v;
    va_start(v, st);
    CTerminal::WriteFormatted(st, v);
    va_end(v);
}

void CTerminal::WriteFormatted(const char* st, va_list args)
{
    char c;
    CTerminal& term = CTerminal::GetInstance();
    int l = strlen(st);
    for (int i = 0; i < l; i++)
    {
        c = st[i];
        if (c == '%')
        {
            if (st[i + 1] == '%')
            {
                term.WriteChar('%');
                continue;
            }

            term.m_FormatState = kTerminalFmtStateSpecifParse;
            continue;
        }

        if (term.m_FormatState == kTerminalFmtStateNormal)
        {
            term.WriteChar(c);
            continue;
        }

        switch (c)
        {
        case 'i':
        {
            int arg = va_arg(args, int);
            WriteNumber(arg, 10);
            break;
        }
        case 'u':
        {
            uint64_t arg = va_arg(args, uint64_t);
            WriteNumber(arg, 10);
            break;
        }
        case 'p':
        case 'X':
        {
            void* arg = va_arg(args, void*);
            WriteNumber(reinterpret_cast<uint64_t>(arg), 16);
            break;
        }
        case 's':
        {
            char* str = va_arg(args, char*);
            CTerminal::Write(str);
            break;
        }

        /* ACPICA compatibility. It expects a full-blown,
           standards-compliant printf implementation, which
           we do not have.  */
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            continue;
        }

        term.m_FormatState = kTerminalFmtStateNormal;
    }
}

void CTerminal::WriteFormat(const char *st, ...)
{
    va_list va;
    va_start(va, st);
    CTerminal::WriteFormatted(st, va);
    va_end(va);
}

void CTerminal::PrepareForAbort()
{
    ssfn_dst.x = 0;
    ssfn_dst.y = 0;
    ssfn_dst.fg = 0xFFFFFF;
    ssfn_dst.bg = ABORT_BG_COLOR;
    ClearScreen(ABORT_BG_COLOR);
}
