// Copyright (C) 2017 Hideaki Narita


#include "stdafx.h"
#include <map>
#include <string>
#include "DebugPut.h"
#include "TimeMachine.h"


#pragma comment(lib, "Dbghelp.lib")


typedef std::map<std::string, void*> HookMap;
typedef std::pair<std::string, void*> HookMapEntry;


using namespace hnrt;


static long long s_delta = 0;
static long s_installed = 0;
static HookMap s_map;


static void InstallHook(HMODULE);
static void InstallHook(PBYTE, PCWSTR);
static void InstallHook(PBYTE, PIMAGE_IMPORT_DESCRIPTOR);
static void InstallHook(PBYTE, PIMAGE_THUNK_DATA, PIMAGE_THUNK_DATA);


TimeMachine::TimeMachine(HMODULE hModule)
    : _hModule(hModule)
{
}


TimeMachine::~TimeMachine()
{
    reset();
}


void TimeMachine::setDate(int year, int month, int day)
{
    TIME_ZONE_INFORMATION tz = { 0 };
    GetTimeZoneInformation(&tz);
    SYSTEMTIME t1 = { 0 };
    GetLocalTime(&t1);
    t1.wHour = 0;
    t1.wMinute = 0;
    t1.wSecond = 0;
    t1.wMilliseconds = 0;
    SYSTEMTIME u1 = { 0 };
    TzSpecificLocalTimeToSystemTime(&tz, &t1, &u1);
    FILETIME f1;
    SystemTimeToFileTime(&u1, &f1);
    LARGE_INTEGER g1;
    g1.HighPart = f1.dwHighDateTime;
    g1.LowPart = f1.dwLowDateTime;

    SYSTEMTIME t2 = { 0 };
    t2.wYear = (WORD)year;
    t2.wMonth = (WORD)month;
    t2.wDay = (WORD)day;
    SYSTEMTIME u2 = { 0 };
    TzSpecificLocalTimeToSystemTime(&tz, &t2, &u2);
    FILETIME f2;
    SystemTimeToFileTime(&u2, &f2);
    LARGE_INTEGER g2;
    g2.HighPart = f2.dwHighDateTime;
    g2.LowPart = f2.dwLowDateTime;

    _InterlockedExchange((unsigned long long*)&s_delta, g2.QuadPart - g1.QuadPart);

    if (!_InterlockedExchange((unsigned long*)&s_installed, 1L))
    {
        InstallHook(_hModule);
    }
}


void TimeMachine::setDelta(long long delta)
{
    _InterlockedExchange((unsigned long long*)&s_delta, delta * 10000LL);

    if (!_InterlockedExchange((unsigned long*)&s_installed, 1L))
    {
        InstallHook(_hModule);
    }
}


void TimeMachine::reset()
{
    _InterlockedExchange((unsigned long long*)&s_delta, 0LL);
}


static HMODULE WINAPI HookLoadLibraryA(LPCSTR lpFileName)
{
    DebugPut(L"HookLoadLibraryA(%hs)", lpFileName);
    HMODULE hModule = LoadLibraryA(lpFileName);
    WCHAR name[MAX_PATH];
    _snwprintf_s(name, _TRUNCATE, L"%hs", lpFileName);
    InstallHook((PBYTE)hModule, name);
    return hModule;
}


HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpFileName)
{
    DebugPut(L"HookLoadLibraryW(%s)", lpFileName);
    HMODULE hModule = LoadLibraryW(lpFileName);
    InstallHook((PBYTE)hModule, lpFileName);
    return hModule;
}


static HMODULE WINAPI HookLoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    DebugPut(L"HookLoadLibraryExA(%hs,%p,%xh)", lpFileName, hFile, dwFlags);
    HMODULE hModule = LoadLibraryExA(lpFileName, hFile, dwFlags);
    WCHAR name[MAX_PATH];
    _snwprintf_s(name, _TRUNCATE, L"%hs", lpFileName);
    InstallHook((PBYTE)hModule, name);
    return hModule;
}


HMODULE WINAPI HookLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    DebugPut(L"HookLoadLibraryExW(%s,%p,%xh)", lpFileName, hFile, dwFlags);
    HMODULE hModule = LoadLibraryExW(lpFileName, hFile, dwFlags);
    InstallHook((PBYTE)hModule, lpFileName);
    return hModule;
}


inline static void Adjust(FILETIME& ft)
{
    LARGE_INTEGER li;
    li.HighPart = ft.dwHighDateTime;
    li.LowPart = ft.dwLowDateTime;
    li.QuadPart += s_delta;
    ft.dwHighDateTime = li.HighPart;
    ft.dwLowDateTime = li.LowPart;
}


void WINAPI HookGetSystemTimeAsFileTime(LPFILETIME pFT)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    Adjust(ft);
    *pFT = ft;
    TIME_ZONE_INFORMATION tz = { 0 };
    GetTimeZoneInformation(&tz);
    SYSTEMTIME ut = { 0 };
    FileTimeToSystemTime(&ft, &ut);
    SYSTEMTIME lt = { 0 };
    SystemTimeToTzSpecificLocalTime(&tz, &ut, &lt);
    DebugPut(L"HookGetSystemTimeAsFileTime: %d-%02d-%02d %02d:%02d:%02d.%03d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}


void WINAPI HookGetSystemTime(LPSYSTEMTIME pST)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    Adjust(ft);
    FileTimeToSystemTime(&ft, pST);
    DebugPut(L"HookGetSystemTime: %d-%02d-%02d %02d:%02d:%02d.%03d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond, pST->wMilliseconds);
}


void WINAPI HookGetLocalTime(LPSYSTEMTIME pST)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    Adjust(ft);
    TIME_ZONE_INFORMATION tz = { 0 };
    GetTimeZoneInformation(&tz);
    SYSTEMTIME ut = { 0 };
    FileTimeToSystemTime(&ft, &ut);
    SystemTimeToTzSpecificLocalTime(&tz, &ut, pST);
    DebugPut(L"HookGetLocalTime: %d-%02d-%02d %02d:%02d:%02d.%03d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond, pST->wMilliseconds);
}


static void InstallHook(HMODULE hModule)
{
    DWORD dwError;
    s_map.insert(HookMapEntry(std::string("LoadLibraryA"), &HookLoadLibraryA));
    s_map.insert(HookMapEntry(std::string("LoadLibraryW"), &HookLoadLibraryW));
    s_map.insert(HookMapEntry(std::string("LoadLibraryExA"), &HookLoadLibraryExA));
    s_map.insert(HookMapEntry(std::string("LoadLibraryExW"), &HookLoadLibraryExW));
    s_map.insert(HookMapEntry(std::string("GetSystemTimeAsFileTime"), &HookGetSystemTimeAsFileTime));
    s_map.insert(HookMapEntry(std::string("GetSystemTime"), &HookGetSystemTime));
    s_map.insert(HookMapEntry(std::string("GetLocalTime"), &HookGetLocalTime));
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCWSTR)hModule, &hModule))
    {
        dwError = GetLastError();
        DebugPut(L"Module pinning failed. Error %u", dwError);
        return;
    }
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32W entry;
        entry.dwSize = static_cast<DWORD>(sizeof(entry));
        if (Module32FirstW(hSnapshot, &entry))
        {
            do
            {
                InstallHook(entry.modBaseAddr, entry.szModule);
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


static void InstallHook(PBYTE pBase, PCWSTR pszName)
{
    if (!_wcsicmp(pszName, L"TimeMachine.dll") ||
        !_wcsicmp(pszName, L"Kernel32.dll"))
    {
        DebugPut(L"[%s] SKIPPED", pszName);
        return;
    }
    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR pDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(pBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);
    if (!pDesc)
    {
        DWORD dwError = GetLastError();
        DebugPut(L"[%s] ERROR %u", pszName, dwError);
        return;
    }
    DebugPut(L"[%s]", pszName);
    while (pDesc->Name)
    {
        InstallHook(pBase, pDesc);
        pDesc++;
    }
    DebugPut(L"+---(end)");
}


static void InstallHook(PBYTE pBase, PIMAGE_IMPORT_DESCRIPTOR pDesc)
{
    const char* szModuleName = (const char*)(pBase + pDesc->Name);
    DebugPut(L"+---%hs", szModuleName);
    PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pBase + pDesc->FirstThunk);
    PIMAGE_THUNK_DATA pOrgThunk = (PIMAGE_THUNK_DATA)(pBase + pDesc->OriginalFirstThunk);
    while (pThunk->u1.Function)
    {
        InstallHook(pBase, pThunk, pOrgThunk);
        pThunk++;
        pOrgThunk++;
    }
}


static void InstallHook(PBYTE pBase, PIMAGE_THUNK_DATA pThunk, PIMAGE_THUNK_DATA pOrgThunk)
{
    if (IMAGE_SNAP_BY_ORDINAL(pOrgThunk->u1.Ordinal))
    {
        return;
    }
    PIMAGE_IMPORT_BY_NAME pImg = (PIMAGE_IMPORT_BY_NAME)(pBase + pOrgThunk->u1.AddressOfData);
    std::string name(pImg->Name);
    HookMap::iterator iter = s_map.find(name);
    if (iter == s_map.end())
    {
        DebugPut(L"|   +---%hs", pImg->Name);
        return;
    }
    DebugPut(L"|   +---%hs ******************************", pImg->Name);
    void* pfnHook = iter->second;
    DWORD dwOrgProtect;
    if (!VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), PAGE_READWRITE, &dwOrgProtect))
    {
        DWORD dwError = GetLastError();
        DebugPut(L"VirtualProtect(PAGE_READWRITE) failed. Error %u", dwError);
        return;
    }
    pThunk->u1.Function = (ULONGLONG)pfnHook;
    DWORD dwNewProtect;
    if (!VirtualProtect(&pThunk->u1.Function, sizeof(pThunk->u1.Function), dwOrgProtect, &dwNewProtect))
    {
        DWORD dwError = GetLastError();
        DebugPut(L"VirtualProtect(%xh) failed. Error %u", dwOrgProtect, dwError);
    }
}
