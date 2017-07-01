// Copyright (C) 2017 Hideaki Narita


#pragma once


#include <Windows.h>


namespace hnrt
{

    class TimeMachine
    {

    public:

        TimeMachine(HMODULE);
        ~TimeMachine();
        void setDate(int, int, int);
        void setDelta(long long);
        void reset();

    private:

        HMODULE _hModule;

    };

}
