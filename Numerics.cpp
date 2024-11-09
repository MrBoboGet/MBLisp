#include "Numerics.h"
namespace MBLisp
{
    static bool AnyFunc(List& List)
    {
        bool ReturnValue = false;
        for(auto const& Value : List)
        {
            if(Value.IsType<bool>() && Value.GetType<bool>())
            {
                return true;   
            }
        }
        return ReturnValue;
    }
    static bool All(List& List)
    {
        bool ReturnValue = true;
        for(auto const& Value : List)
        {
            if(!Value.IsType<bool>() || !Value.GetType<bool>())
            {
                return false;   
            }
        }
        return ReturnValue;
    }

    void AddNumericFunctions(Evaluator& Evaluator)
    {
        Evaluator.AddGeneric<AnyFunc>("any");
        Evaluator.AddGeneric<All>("all");
    }
}
