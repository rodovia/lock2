#pragma once

#include <stdarg.h>

void _abortwrite(const char* msg, ...);
void _abortwrite(const char* msg, va_list ls);

[[gnu::noreturn]]
void abort(const char* msg = nullptr);

[[gnu::noreturn]]
void abort(const char* msg, ...);
