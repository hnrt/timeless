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
        inline bool needToExclude(PCWSTR);
        inline void* getHook(const char*);

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
        FILETIME ft;
        SystemTimeToFileTime(&utcTime, &ft);
        LARGE_INTEGER gt;
        gt.HighPart = ft.dwHighDateTime;
        gt.LowPart = ft.dwLowDateTime;
        return gt.QuadPart;
    }

    inline void TimeMachine::adjust(FILETIME& ft)
    {
        LARGE_INTEGER li;
        li.HighPart = ft.dwHighDateTime;
        li.LowPart = ft.dwLowDateTime;
        li.QuadPart += _delta;
        ft.dwHighDateTime = li.HighPart;
        ft.dwLowDateTime = li.LowPart;
    }

    inline void TimeMachine::fileTimeToLocalTime(const FILETIME& ft, SYSTEMTIME& st)
    {
        SYSTEMTIME ut = { 0 };
        FileTimeToSystemTime(&ft, &ut);
        SystemTimeToTzSpecificLocalTime(&_timeZone, &ut, &st);
    }

    inline bool TimeMachine::needToExclude(PCWSTR pszKey)
    {
        CaseInsensitiveStringSet::const_iterator iter = _excludeSet.find(pszKey);
        return iter != _excludeSet.end();
    }

    inline void* TimeMachine::getHook(const char* pszKey)
    {
        std::string sKey(pszKey);
        HookMap::iterator iter = _map.find(sKey);
        if (iter != _map.end())
        {
            return iter->second;
        }
        else
        {
            return 0;
        }
    }

}
