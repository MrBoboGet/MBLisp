#include "DB.h"
#include "../../Evaluator.h"

#include <MrBoboDatabase/MrBoboDatabase.h>
namespace MBLisp
{

    static MBDB::SQLStatement GetStatement(MBDB::MrBoboDatabase& DB,String& SQL)
    {
        return DB.GetSQLStatement(SQL);
    }
    static void Bind_String_Int(MBDB::SQLStatement& Statement,String& ParameterName,Int Value)
    {
        Statement.BindValue(ParameterName,Value);
    }
    static void Bind_String_String(MBDB::SQLStatement& Statement,String& ParameterName,String& Value)
    {
        Statement.BindValue(ParameterName,Value);
    }
    static void Bind_String_Null(MBDB::SQLStatement& Statement,String& ParameterName,Null& Value)
    {
        Statement.BindNull(ParameterName);
    }
    static void Bind_Int_Int(MBDB::SQLStatement& Statement,Int ParameterIndex,Int Value)
    {
        Statement.BindInt(Value,ParameterIndex);
    }
    static void Bind_Int_String(MBDB::SQLStatement& Statement,Int ParameterIndex,String& Value)
    {
        Statement.BindString(Value,ParameterIndex);
    }
    static void Bind_Int_Null(MBDB::SQLStatement& Statement,Int ParameterIndex,Null Value)
    {
        Statement.BindNull(ParameterIndex);
    }


    struct StatementIterator
    {
        Ref<MBDB::SQLStatement> Statement;
        Value CurrentValue;
    };
    static Value Iterator(Ref<MBDB::SQLStatement> Stmt)
    {
        StatementIterator ReturnValue;
        ReturnValue.Statement = Stmt;
        return Value::MakeExternal<StatementIterator>(std::move(ReturnValue));
    }
    static bool Next(StatementIterator& It)
    {
        bool ReturnValue = false;
        if(It.Statement->HasMore())
        {
            ReturnValue = true;
            It.CurrentValue = Value::MakeExternal<MBDB::MBDB_RowData>(It.Statement->Next());
        }
        return ReturnValue;
    }
    static Value Current(StatementIterator& It)
    {
        return It.CurrentValue;
    }


    Value CellToValue(MBDB::MBDB_RowData::VariantType const& Column)
    {
        if(std::holds_alternative<std::monostate>(Column))
        {
            return Null();
        }
        else if(std::holds_alternative<MBDB::IntType>(Column))
        {
            return Value(std::get<MBDB::IntType>(Column));
        }
        else if(std::holds_alternative<std::string>(Column))
        {
            return Value(std::get<std::string>(Column));
        }
        else if(std::holds_alternative<MBDB::FloatType>(Column))
        {
            return Value(std::get<MBDB::FloatType>(Column));
        }
        return Null();
    }
    static Value IndexRow(MBDB::MBDB_RowData& Row,String& Value)
    {
        return CellToValue(Row[Value]);
    }

    static Value OpenDatabase(String const& Value)
    {
        return Value::EmplaceExternal<MBDB::MrBoboDatabase>(Value,MBDB::DBOpenOptions::ReadWrite);
    }

    static void Execute_String(MBDB::MrBoboDatabase& DB,String const& SQL)
    {
        auto Stmt = DB.GetSQLStatement(SQL);
        while(Stmt.HasMore())
        {
            Stmt.Next();   
        }
    }

    Ref<Scope> DBModule::GetModuleScope(Evaluator& AssociatedEvaluator)
    {
        Ref<Scope> ReturnValue = MakeRef<Scope>();

        AssociatedEvaluator.AddGeneric<GetStatement>(ReturnValue,"statement");

        AssociatedEvaluator.AddGeneric<Bind_String_Int>(ReturnValue,"bind");
        AssociatedEvaluator.AddGeneric<Bind_String_String>(ReturnValue,"bind");
        AssociatedEvaluator.AddGeneric<Bind_String_Null>(ReturnValue,"bind");
        AssociatedEvaluator.AddGeneric<Bind_Int_Int>(ReturnValue,"bind");
        AssociatedEvaluator.AddGeneric<Bind_Int_String>(ReturnValue,"bind");
        AssociatedEvaluator.AddGeneric<Bind_Int_Null>(ReturnValue,"bind");

        AssociatedEvaluator.AddGeneric<OpenDatabase>(ReturnValue,"open-db");
        AssociatedEvaluator.AddGeneric<Execute_String>(ReturnValue,"execute");


        //AssociatedEvaluator.AddObjectMethod<&MBDB::SQLStatement::HasMore>("more");
        //AssociatedEvaluator.AddObjectMethod<&MBDB::SQLStatement::Next>("next");
        //
        AssociatedEvaluator.AddGeneric<Iterator>("iterator");
        AssociatedEvaluator.AddGeneric<Next>("next");
        AssociatedEvaluator.AddGeneric<Current>("current");

        AssociatedEvaluator.AddGeneric<IndexRow>("index");

        return ReturnValue;
    }
}
