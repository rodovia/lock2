#include "dllogic/ustar.h"
#include "klibc/string.h"

static int OctalToInteger(const char* str, int size)
{
    int c = 0;
    for (int i = 0; i < size; i++)
    {
        char w = str[i];
        c *= 8;
        c += w - '0';
    }
    return c;
}

int driver::LoadUstarFile(unsigned char *archive, const char *file, char*& buffer)
{
    const char* ptr = reinterpret_cast<char*>(archive);
    if (archive == nullptr)
    {
        return -1;
    }

    while(!strncmp(ptr + 257, "ustar", 5))
    {
        int size = OctalToInteger(ptr, strlen(ptr) + 1);
        if (!strncmp(ptr, file, strlen(file) + 1))
        {
            buffer = const_cast<char*>(ptr + 512);
            return size;
        }
        ptr += (((size + 511) / 512) + 1) * 512;
    }
    return 0;
}
