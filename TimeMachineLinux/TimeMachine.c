/* Copyright (C) 2017 Hideaki Narita */


/* Remember to set LD_PRELOAD environment variable to the path to libTimeMachine.so */


#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <dlfcn.h>
#include "com_hideakin_lib_time_TimeMachine.h"


static long s_delta; // in microseconds


JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_setDate(JNIEnv* env, jobject obj, jint year, jint month, jint day)
{
    int (*pfnGetTimeOfDay)(struct timeval*, struct timezone*);
    struct timeval tv;
    struct tm tm1;
    struct tm tm2;
    time_t t1;
    time_t t2;

    pfnGetTimeOfDay = dlsym(RTLD_NEXT, "gettimeofday");
    (*pfnGetTimeOfDay)(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm1);
    tm1.tm_hour = 0;
    tm1.tm_min = 0;
    tm1.tm_sec = 0;
    tm1.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t1 = mktime(&tm1);

    tm2.tm_year = year - 1900;
    tm2.tm_mon = month - 1; // Month, 0 - jan
    tm2.tm_mday = day;
    tm2.tm_hour = 0;
    tm2.tm_min = 0;
    tm2.tm_sec = 0;
    tm2.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t2 = mktime(&tm2);

    s_delta = (t2 - t1) * 1000000L;
}


JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_setDelta(JNIEnv* env, jobject obj, jlong delta)
{
    s_delta = delta * 1000L;
}


JNIEXPORT void JNICALL Java_com_hideakin_lib_time_TimeMachine_reset(JNIEnv* env, jobject obj)
{
    s_delta = 0;
}


int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    int (*pfnGetTimeOfDay)(struct timeval*, struct timezone*);
    int rc;

    pfnGetTimeOfDay = dlsym(RTLD_NEXT, "gettimeofday");

    rc = (*pfnGetTimeOfDay)(tv, tz);

    long usec = tv->tv_usec + s_delta;
    if (usec >= 0L)
    {
        tv->tv_sec += usec / 1000000L;
        tv->tv_usec = usec % 1000000L;
    }
    else //if (usec < 0)
    {
        tv->tv_sec -= (1000000L - 1 - usec) / 1000000L;
        tv->tv_usec = (1000000L - (-usec % 1000000L)) % 1000000L;
    }

    printf("gettimeofday: %ld %ld\n", (long)tv->tv_sec, (long)tv->tv_usec);

    return rc;
}
