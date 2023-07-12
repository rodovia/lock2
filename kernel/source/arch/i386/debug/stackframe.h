#pragma once

struct stackframe;

namespace dbg
{

void DumpStackFrame(const struct stackframe* rbp = nullptr);

}
