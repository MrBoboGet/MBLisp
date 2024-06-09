#pragma once
#include "../../Evaluator.h"

#include "../Util.h"
namespace MBLisp
{
    class IOModule : public Module
    {
        struct DirItDelegate
        {
            std::string Path;
            bool Recursive = false;
        };


        static std::string NormalizePath(std::filesystem::path const& Path);

       
        static String FileName(std::string const& FileName);
        static String Extension(std::string const& Path);
        static String Relative(std::string const& Base,std::string const& Path);
        static String RelativeCWD(std::string const& Path);
        static Int ModTime(std::string const& Path);
        
        static List Ls(std::string const& Dir);
        static DirItDelegate DirIt(std::string const& Dir);
        static DirItDelegate DirItRec(std::string const& Dir,bool IsRecursive);

        static Value Iterator(DirItDelegate const& Delegate);

        static std::string Current(ItAdaptor<std::filesystem::directory_iterator>& Iterator);
        static bool Next(ItAdaptor<std::filesystem::directory_iterator>& Iterator);
        
        static std::string CurrentRec(ItAdaptor<std::filesystem::recursive_directory_iterator>& Iterator);
        static bool NextRec(ItAdaptor<std::filesystem::recursive_directory_iterator>& Iterator);
    public:
        virtual Ref<Scope> GetModuleScope(Evaluator& AssociatedEvaluator) override;
    };
}
