// Copyright (C) 2017 Hideaki Narita


#include "stdafx.h"
#include "DebugPut.h"


void hnrt::DebugPut(PCWSTR pszFormat, ...)
{
    WCHAR buf[512];
    va_list argList;
    va_start(argList, pszFormat);
    _vsnwprintf_s(buf, _TRUNCATE, pszFormat, argList);
    va_end(argList);
    size_t len = wcslen(buf);
    if (len > 0)
    {
        if (len + 2 <= sizeof(buf) && buf[len - 1] != L'\n')
        {
            buf[len + 0] = L'\n';
            buf[len + 1] = L'\0';
        }
        OutputDebugStringW(buf);
    }
}
