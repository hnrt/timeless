// Copyright (C) 2017 Hideaki Narita


#include "stdafx.h"
#include "DebugPut.h"
#include "TimeMachine.h"


#pragma comment(lib, "Dbghelp.lib")


using namespace hnrt;


static TimeMachine* s_pTimeMachine;


TimeMachine& TimeMachine::getInstance(HMODULE hModule)
{
    if (!s_pTimeMachine)
    {
        TimeMachine* pTimeMachine = new TimeMachine(hModule);
        if (_InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&s_pTimeMachine), pTimeMachine, 0))
        {
            delete pTimeMachine;
        }
    }
    return *s_pTimeMachine;
}


TimeMachine& TimeMachine::getInstance()
{
    return *s_pTimeMachine;
}


TimeMachine::TimeMachine(HMODULE hModule)
    : _hModule(hModule)
    , _delta(0)
    , _state(HOOK_INSTALL_PENDING)
{
    GetTimeZoneInformation(&_timeZone);
}


TimeMachine::~TimeMachine()
{
    _InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&s_pTimeMachine), 0, reinterpret_cast<PVOID>(this));
}


void TimeMachine::setDate(int year, int month, int day)
{
    SYSTEMTIME t1 = { 0 };
    GetLocalTime(&t1);
    t1.wHour = 0;
    t1.wMinute = 0;
    t1.wSecond = 0;
    t1.wMilliseconds = 0;
    long long g1 = localTimeToFileTime(t1);

    SYSTEMTIME t2 = { 0 };
    t2.wYear = (WORD)year;
    t2.wMonth = (WORD)month;
    t2.wDay = (WORD)day;
    long long g2 = localTimeToFileTime(t2);

    set(g2 - g1);
}


// The delta is passed as a value in milliseconds.
void TimeMachine::setDelta(long long delta)
{
    set(delta * 10000LL);
}


// The delta is passed as a value in the number of 100-nanosecond intervals.
void TimeMachine::set(long long delta)
{
    _InterlockedExchange(reinterpret_cast<unsigned long long*>(&_delta), delta);

    switch (_InterlockedCompareExchange(&_state, HOOK_INSTALLING, HOOK_INSTALL_PENDING))
    {
    case HOOK_INSTALL_PENDING:
        initMap();
        pinModule();
        installHook();
        _InterlockedExchange(&_state, HOOK_INSTALLED);
        break;
    case HOOK_INSTALLING:
        // wait until the hook installation finished or 1 second is passed
        for (int c = 1000; c && _InterlockedCompareExchange(&_state, HOOK_INSTALLING, HOOK_INSTALLING) == HOOK_INSTALLING; c--)
        {
            Sleep(1);
        }
        break;
    default:
        // the hook already installed
        break;
    }
}


void TimeMachine::reset()
{
    _InterlockedExchange(reinterpret_cast<unsigned long long*>(&_delta), 0LL);
}


void TimeMachine::pinModule()
{
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, reinterpret_cast<LPWSTR>(_hModule), &_hModule))
    {
        DWORD dwError = GetLastError();
        DebugPut(L"Module pinning failed. Error %u", dwError);
    }
}


void TimeMachine::installHook()
{
    DWORD dwError;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32W entry;
        entry.dwSize = static_cast<DWORD>(sizeof(entry));
        if (Module32FirstW(hSnapshot, &entry))
        {
            do
            {
                installHook(entry.modBaseAddr, entry.szModule);
            } while (Module32NextW(hSnapshot, &entry));
            dwError = GetLastError();
            if (dwError != ERROR_NO_MORE_FILES)
            {
                DebugPut(L"Module32Next failed. Error %u", dwError);
            }
        }
        else
        {
            dwError = GetLastError();
            DebugPut(L"Module32First failed. Error %u", dwError);
        }
        CloseHandle(hSnapshot);
    }
    else
    {
        dwError = GetLastError();
        DebugPut(L"CreateToolhelp32Snapshot(MODULE) failed. Error %u", dwError);
    }
}


void TimeMachine::installHook(PBYTE pBase, PCWSTR pszName)
{
    if (needToExclude(pszName))
    {
        DebugPut(L"[%s] SKIPPED", pszName);
        return;
    }
    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR pDesc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToData(pBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size));
    if (!pDesc)
    {
        DWORD dwError = GetLastError();
        DebugPut(L"[%s] ERROR %u", pszName, dwError);
        return;
    }
    DebugPut(L"[%s]", pszName);
    while (pDesc->Name)
    {
        installHook(pBase, pDesc);
        pDesc++;
    }
    DebugPut(L"+---(end)");
}


void TimeMachine::installHook(PBYTE pBase, PIMAGE_IMPORT_DESCRIPTOR pDesc)
{
    char* szModuleName = reinterpret_cast<char*>(pBase + pDesc->Name);
    DebugPut(L"+---%hs", szModuleName);
    PIMAGE_THUNK_DATA pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(pBase + pDesc->FirstThunk);
    PIMAGE_THUNK_DATA pOrgThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(pBase + pDesc->OriginalFirstThunk);
    while (pThunk->u1.Function)
    {
        installHook(pBase, pThunk, pOrgThunk);
        pThunk++;
        pOrgThunk++;
    }
}


void TimeMachine::installHook(PBYTE pBase, PIMAGE_THUNK_DATA pThunk, PIMAGE_THUNK_DATA pOrgThunk)
{
    if (IMAGE_SNAP_BY_ORDINAL(pOrgThunk->u1.Ordinal))
    {
        return;
    }
    PIMAGE_IMPORT_BY_NAME pImg = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(pBase + pOrgThunk->u1.AddressOfData);
    void* pfnHook = getHook(pImg->Name);
    if (!pfnHook)
    {
        DebugPut(L"|   +---%hs", pImg->Name);
        return;
    }
    DebugPut(L"|   +---%hs ******************************", pImg->Name);
    DWORD dwOrgProtect;
    if (!VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), PAGE_READWRITE, &dwOrgProtect))
    {
        DWORD dwError = GetLastError();
        DebugPut(L"VirtualProtect(PAGE_READWRITE) failed. Error %u", dwError);
        return;
    }
    pThunk->u1.Function = reinterpret_cast<ULONGLONG>(pfnHook);
    DWORD dwNewProtect;
    if (!VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), dwOrgProtect, &dwNewProtect))
    {
        DWORD dwError = GetLastError();
        DebugPut(L"VirtualProtect(%xh) failed. Error %u", dwOrgProtect, dwError);
    }
}


bool TimeMachine::CaseInsensitiveStringComparator::operator()(PCWSTR s1, PCWSTR s2) const
{
    return _wcsicmp(s1, s2) < 0;
}
