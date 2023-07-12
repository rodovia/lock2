#include "string.h"

int strlen(const char* string)
{
    const char* s;
    for (s = string; *s; s++);
    return (s - string);
}

int strncmp(const char* n1, const char* n2, int size)
{
    if (strlen(n1) == 0 || strlen(n2) == 0)
    {
        return 1;
    }

    for (int i = 0; i < size; i++)
    {
        if (n1[i] != n2[i])
        {
            return 1;
        }
    }

    return 0;
}
