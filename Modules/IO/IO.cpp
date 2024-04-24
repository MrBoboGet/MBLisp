#include "IO.h"

#include <MBUnicode/MBUnicode.h>
namespace MBLisp
{
    std::string IOModule::NormalizePath(std::filesystem::path const& Path)
    {
        std::string ReturnValue = MBUnicode::PathToUTF8(std::filesystem::weakly_canonical(Path));
        return ReturnValue;
    }

    std::string IOModule::FileName(std::string const& FileName)
    {
        return MBUnicode::PathToUTF8( std::filesystem::path(FileName).filename());
    }
    std::string IOModule::Extension(std::string const& Path)
    {
        return MBUnicode::PathToUTF8(std::filesystem::path(Path).extension());
    }

    std::string IOModule::Relative(std::string const& Base,std::string const& Path)
    {
        return MBUnicode::PathToUTF8(std::filesystem::relative(Path,Base));
    }
    std::string IOModule::RelativeCWD(std::string const& Path)
    {
        return MBUnicode::PathToUTF8(std::filesystem::relative(Path));
    }
    List IOModule::Ls(std::string const& Dir)
    {
        List ReturnValue;
        for(auto& Entry : std::filesystem::directory_iterator(Dir))
        {
            ReturnValue.push_back(NormalizePath(Entry.path()));
        }
        return ReturnValue;
    }
    IOModule::DirItDelegate IOModule::DirIt(std::string const& Dir)
    {
        DirItDelegate ReturnValue;
        ReturnValue.Path = Dir;
        return ReturnValue;
    }
    IOModule::DirItDelegate IOModule::DirItRec(std::string const& Dir,bool IsRecursive)
    {
        DirItDelegate ReturnValue;
        ReturnValue.Path = Dir;
        ReturnValue.Recursive = IsRecursive;
        return ReturnValue;
    }
    Value IOModule::Iterator(DirItDelegate const& Delegate)
    {
        Value ReturnValue;
        std::string PathToSearch = Delegate.Path != "" ? Delegate.Path : ".";
        if(Delegate.Recursive)
        {
            return Value::EmplaceExternal<ItAdaptor<std::filesystem::recursive_directory_iterator>>(std::filesystem::recursive_directory_iterator(Delegate.Path));
        }
        else 
        {
            return Value::EmplaceExternal<ItAdaptor<std::filesystem::directory_iterator>>( std::filesystem::directory_iterator(Delegate.Path));
        }
        return ReturnValue;
    }


    std::string IOModule::Current(ItAdaptor<std::filesystem::directory_iterator>& Iterator)
    {
        if(!Iterator.HasCurrent())
        {
            throw std::runtime_error("Accessing directory iterator after it's finished");
        }
        return NormalizePath(Iterator->path()); 
    }

    bool IOModule::Next(ItAdaptor<std::filesystem::directory_iterator>& Iterator)
    {
        Iterator.Next();
        return Iterator.HasCurrent();
    }
    std::string IOModule::CurrentRec(ItAdaptor<std::filesystem::recursive_directory_iterator>& Iterator)
    {
        if(!Iterator.HasCurrent())
        {
            throw std::runtime_error("Accessing directory iterator after it's finished");
        }
        return NormalizePath(Iterator->path()); 
    }
    bool IOModule::NextRec(ItAdaptor<std::filesystem::recursive_directory_iterator>& Iterator)
    {
        Iterator.Next();
        return Iterator.HasCurrent();
    }




    Ref<Scope> IOModule::GetModuleScope(Evaluator& AssociatedEvaluator) 
    {
        Ref<Scope> ReturnValue = AssociatedEvaluator.CreateDefaultScope();
        AssociatedEvaluator.AddGeneric<Next>("next");
        AssociatedEvaluator.AddGeneric<Current>("current");
        AssociatedEvaluator.AddGeneric<NextRec>("next");
        AssociatedEvaluator.AddGeneric<CurrentRec>("current");

        AssociatedEvaluator.AddGeneric<Relative>("relative");
        AssociatedEvaluator.AddGeneric<FileName>("file-name");
        AssociatedEvaluator.AddGeneric<Extension>("extension");
        AssociatedEvaluator.AddGeneric<RelativeCWD>("relative");

        AssociatedEvaluator.AddGeneric<Ls>("ls");
        AssociatedEvaluator.AddGeneric<Iterator>("iterator");
        AssociatedEvaluator.AddGeneric<DirIt>("dir-it");
        AssociatedEvaluator.AddGeneric<DirItRec>("dir-it");

        return ReturnValue;
    }
}
