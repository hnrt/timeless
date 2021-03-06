// Copyright (C) 2017 Hideaki Narita


#pragma once


#include <Windows.h>
#include <Dbghelp.h>
#include <map>
#include <set>
#include <string>


namespace hnrt
{

    class TimeMachine
    {

    public:

        static TimeMachine& getInstance(HMODULE);
        static TimeMachine& getInstance();

        ~TimeMachine();
        void setDate(int, int, int);
        void setDelta(long long);
        void reset();

    private:

        enum PrivateConstants
        {
            HOOK_INSTALL_PENDING = 0,
            HOOK_INSTALLING = 1,
            HOOK_INSTALLED = 2,
        };

        typedef std::map<std::string, void*> HookMap;
        typedef std::pair<std::string, void*> HookMapEntry;

        class CaseInsensitiveStringComparator
        {

        public:

            bool operator()(PCWSTR, PCWSTR) const;

        };

        typedef std::set<PCWSTR, CaseInsensitiveStringComparator> CaseInsensitiveStringSet;

        static HMODULE WINAPI hookLoadLibraryA(LPCSTR lpFileName);
        static HMODULE WINAPI hookLoadLibraryW(LPCWSTR lpFileName);
        static HMODULE WINAPI hookLoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
        static HMODULE WINAPI hookLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
        static void WINAPI hookGetSystemTimeAsFileTime(LPFILETIME pFT);
        static void WINAPI hookGetSystemTime(LPSYSTEMTIME pST);
        static void WINAPI hookGetLocalTime(LPSYSTEMTIME pST);

        TimeMachine(HMODULE);
        void set(long long);
        void pinModule();
        void installHook();
        void installHook(PBYTE, PCWSTR);
        void installHook(PBYTE, PIMAGE_IMPORT_DESCRIPTOR);
        void installHook(PBYTE, PIMAGE_THUNK_DATA, PIMAGE_THUNK_DATA);
        void initMap();
        inline long long localTimeToFileTime(const SYSTEMTIME&);
        inline void adjust(FILETIME&);
        inline void fileTimeToLocalTime(const FILETIME&, SYSTEMTIME&);
        inline bool needToExclude(PCWSTR) const;
        inline void* getHook(const char*) const;

        HMODULE _hModule;
        long long _delta; // number of 100-nanosecond intervals
        unsigned long _state;
        TIME_ZONE_INFORMATION _timeZone;
        CaseInsensitiveStringSet _excludeSet;
        HookMap _map;

    };

    inline long long TimeMachine::localTimeToFileTime(const SYSTEMTIME& localTime)
    {
        SYSTEMTIME utcTime = { 0 };
        TzSpecificLocalTimeToSystemTime(&_timeZone, &localTime, &utcTime);
        FILETIME fileTime;
        SystemTimeToFileTime(&utcTime, &fileTime);
        LARGE_INTEGER li;
        li.HighPart = fileTime.dwHighDateTime;
        li.LowPart = fileTime.dwLowDateTime;
        return li.QuadPart;
    }

    inline void TimeMachine::adjust(FILETIME& fileTime)
    {
        LARGE_INTEGER li;
        li.HighPart = fileTime.dwHighDateTime;
        li.LowPart = fileTime.dwLowDateTime;
        li.QuadPart += _delta;
        fileTime.dwHighDateTime = li.HighPart;
        fileTime.dwLowDateTime = li.LowPart;
    }

    inline void TimeMachine::fileTimeToLocalTime(const FILETIME& fileTime, SYSTEMTIME& localTime)
    {
        SYSTEMTIME utcTime = { 0 };
        FileTimeToSystemTime(&fileTime, &utcTime);
        SystemTimeToTzSpecificLocalTime(&_timeZone, &utcTime, &localTime);
    }

    inline bool TimeMachine::needToExclude(PCWSTR pszKey) const
    {
        CaseInsensitiveStringSet::const_iterator iter = _excludeSet.find(pszKey);
        return iter != _excludeSet.end();
    }

    inline void* TimeMachine::getHook(const char* pszKey) const
    {
        std::string sKey(pszKey);
        HookMap::const_iterator iter = _map.find(sKey);
        return iter != _map.end() ? iter->second : 0;
    }

}
