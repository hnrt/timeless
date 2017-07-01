// Copyright (C) 2017 Hideaki Narita


#include "stdafx.h"
#include "com_hideakin_lib_time_TimeMachine.h"
#include "TimeMachine.h"


using namespace hnrt;


static TimeMachine* g_pTimeMachine;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_pTimeMachine = new TimeMachine(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


extern "C" JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_setDate(JNIEnv *, jobject, jint year, jint month, jint day)
{
    g_pTimeMachine->setDate(year, month, day);
}


extern "C" JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_setDelta(JNIEnv *, jobject, jlong delta)
{
    g_pTimeMachine->setDelta(delta);
}


extern "C" JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_reset(JNIEnv *, jobject)
{
    g_pTimeMachine->reset();
}
