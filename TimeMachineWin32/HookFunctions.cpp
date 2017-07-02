// Copyright (C) 2017 Hideaki Narita


#include "stdafx.h"
#include "DebugPut.h"
#include "TimeMachine.h"


using namespace hnrt;


HMODULE WINAPI TimeMachine::hookLoadLibraryA(LPCSTR lpFileName)
{
    DebugPut(L"hookLoadLibraryA(%hs)", lpFileName);
    HMODULE hModule = LoadLibraryA(lpFileName);
    WCHAR szName[MAX_PATH];
    _snwprintf_s(szName, _TRUNCATE, L"%hs", lpFileName);
    TimeMachine::getInstance().installHook(reinterpret_cast<PBYTE>(hModule), szName);
    return hModule;
}


HMODULE WINAPI TimeMachine::hookLoadLibraryW(LPCWSTR lpFileName)
{
    DebugPut(L"hookLoadLibraryW(%s)", lpFileName);
    HMODULE hModule = LoadLibraryW(lpFileName);
    TimeMachine::getInstance().installHook(reinterpret_cast<PBYTE>(hModule), lpFileName);
    return hModule;
}


HMODULE WINAPI TimeMachine::hookLoadLibraryExA(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    DebugPut(L"hookLoadLibraryExA(%hs,%p,%xh)", lpFileName, hFile, dwFlags);
    HMODULE hModule = LoadLibraryExA(lpFileName, hFile, dwFlags);
    WCHAR szName[MAX_PATH];
    _snwprintf_s(szName, _TRUNCATE, L"%hs", lpFileName);
    TimeMachine::getInstance().installHook(reinterpret_cast<PBYTE>(hModule), szName);
    return hModule;
}


HMODULE WINAPI TimeMachine::hookLoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    DebugPut(L"hookLoadLibraryExW(%s,%p,%xh)", lpFileName, hFile, dwFlags);
    HMODULE hModule = LoadLibraryExW(lpFileName, hFile, dwFlags);
    TimeMachine::getInstance().installHook(reinterpret_cast<PBYTE>(hModule), lpFileName);
    return hModule;
}


void WINAPI TimeMachine::hookGetSystemTimeAsFileTime(LPFILETIME pFT)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    TimeMachine::getInstance().adjust(ft);
    *pFT = ft;
    SYSTEMTIME lt = { 0 };
    TimeMachine::getInstance().fileTimeToLocalTime(ft, lt);
    DebugPut(L"hookGetSystemTimeAsFileTime: %d-%02d-%02d %02d:%02d:%02d.%03d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}


void WINAPI TimeMachine::hookGetSystemTime(LPSYSTEMTIME pST)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    TimeMachine::getInstance().adjust(ft);
    FileTimeToSystemTime(&ft, pST);
    DebugPut(L"hookGetSystemTime: %d-%02d-%02d %02d:%02d:%02d.%03d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond, pST->wMilliseconds);
}


void WINAPI TimeMachine::hookGetLocalTime(LPSYSTEMTIME pST)
{
    FILETIME ft = { 0 };
    GetSystemTimeAsFileTime(&ft);
    TimeMachine::getInstance().adjust(ft);
    TimeMachine::getInstance().fileTimeToLocalTime(ft, *pST);
    DebugPut(L"hookGetLocalTime: %d-%02d-%02d %02d:%02d:%02d.%03d", pST->wYear, pST->wMonth, pST->wDay, pST->wHour, pST->wMinute, pST->wSecond, pST->wMilliseconds);
}


void TimeMachine::initMap()
{
    _excludeSet.insert(L"TimeMachine.DLL");
    _excludeSet.insert(L"KERNEL32.DLL");
    _map.insert(HookMapEntry(std::string("LoadLibraryA"), &TimeMachine::hookLoadLibraryA));
    _map.insert(HookMapEntry(std::string("LoadLibraryW"), &TimeMachine::hookLoadLibraryW));
    _map.insert(HookMapEntry(std::string("LoadLibraryExA"), &TimeMachine::hookLoadLibraryExA));
    _map.insert(HookMapEntry(std::string("LoadLibraryExW"), &TimeMachine::hookLoadLibraryExW));
    _map.insert(HookMapEntry(std::string("GetSystemTimeAsFileTime"), &TimeMachine::hookGetSystemTimeAsFileTime));
    _map.insert(HookMapEntry(std::string("GetSystemTime"), &TimeMachine::hookGetSystemTime));
    _map.insert(HookMapEntry(std::string("GetLocalTime"), &TimeMachine::hookGetLocalTime));
}
