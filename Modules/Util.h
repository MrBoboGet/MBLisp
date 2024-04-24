#pragma once
#include "../Value.h"

namespace MBLisp
{
    template<typename T>
    class ItAdaptor
    {
        bool First = true;
        T Begin;
        T End;
    public:
        ItAdaptor(T&& nBegin) : Begin(std::forward<T>(nBegin)),End() { }
        ItAdaptor(T&& nBegin,T&& nEnd) : Begin(std::forward<T>(nBegin)),End(std::forward<T>(nEnd)) { }

        bool HasCurrent()
        {
            return Begin != End;
        }
        void Next()
        {
            if(First)
            {
                First = false;
                return;
            }
            if(Begin != End)
            {
                ++Begin;   
            }
        }
        auto operator->()
        {
            return Begin.operator->();
        }
        auto operator->() const
        {
            return Begin.operator->();
        }
        auto operator*()
        {
            return Begin.operator*();  
        }
        auto operator*() const
        {
            return Begin.operator*();  
        }




    };
}

