#include "CLI.h"
#include "../../Evaluator.h"

#include <MBTUI/MBTUI.h>
#include <MBTUI/Stacker.h>

namespace MBLisp
{
    LispWindow::LispWindow(std::shared_ptr<Evaluator> Evaluator,Value Val)
    {
        m_Evaluator = Evaluator;
        m_ModuleScope = Evaluator->GetModuleScope("cli");
        m_Value = Val;
    }
    bool LispWindow::Updated()
    {
        auto Result = m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"updated") ,{m_Value});
        if(!Result.IsType<bool>())
        {
            throw std::runtime_error("Invalid Lisp window: Update does not return boolean");
        }
        return Result.GetType<bool>();
    }
    void LispWindow::HandleInput(MBCLI::ConsoleInput const& Input)
    {
        m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"handle-input"),{m_Value,MBLisp::Value::EmplaceExternal<MBCLI::ConsoleInput>(Input)});
    }
    MBCLI::Dimensions LispWindow::PreferedDimensions(MBCLI::Dimensions SuggestedDimensions)
    {
        auto Result = m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"prefered-dims"),{m_Value,MBLisp::Value::EmplaceExternal<MBCLI::Dimensions>(SuggestedDimensions)});  
        if(!Result.IsType<MBCLI::Dimensions>())
        {
            throw std::runtime_error("PreferedDimensions returned invalid type");
        }

        return Result.GetType<MBCLI::Dimensions>();
    }
    void LispWindow::SetFocus(bool IsFocused)
    {
        auto Result = m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"set-focus") ,{m_Value,IsFocused});
    }
    MBCLI::CursorInfo LispWindow::GetCursorInfo()
    {
        auto Result = m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"get-cursor-info") ,{m_Value});
        if(!Result.IsType<MBCLI::CursorInfo>())
        {
            throw std::runtime_error("get-cursor-info returned invalid type for lisp window");
        }
        return Result.GetType<MBCLI::CursorInfo>();
    }
    void LispWindow::WriteBuffer(MBCLI::BufferView View,bool Redraw)
    {
        TemporaryLock Lock(View);
        auto LockRef = MBLisp::Value::EmplaceExternal<Temporary<MBCLI::BufferView>>(Lock.GetTemporary());
        m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue("write") ,{m_Value,LockRef,Redraw});
    }

    bool CLIModule::p_WindowUpdated(MBCLI::Window& Window)
    {
        return Window.Updated();
    }
    void CLIModule::p_WindowHandleInput(MBCLI::Window& Window,MBCLI::ConsoleInput const& Input)
    {
        Window.HandleInput(Input);
    }
    void CLIModule::p_WindowSetFocus(MBCLI::Window& Window,bool Focused)
    {
        Window.SetFocus(Focused);
    }
    MBCLI::CursorInfo CLIModule::p_GetCursorInfo(MBCLI::Window& Window)
    {
        return Window.GetCursorInfo();
    }
    void CLIModule::p_WriteBuffer(MBCLI::Window& Window,Temporary<MBCLI::BufferView> View,bool Redraw)
    {
        Window.WriteBuffer(View.Get().SubView(0,0),Redraw);
    }
    //void CLIModule::p_WriteBufferValue(Value Val,Temporary<MBCLI::BufferView> View,bool Redraw)
    //{
    //    //m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"write-buffer") ,{LockRef,Redraw});  
    //}
    MBCLI::CursorInfo CLIModule::CursorInfoDefault()
    {
        return MBCLI::CursorInfo();
    }
    void CLIModule::p_WriteString(Temporary<MBCLI::BufferView>& View,String const& StringToWrite)
    {
        View.Get().WriteCharacters(0,0,StringToWrite);
    }
    void CLIModule::p_WriteStringOffset(Temporary<MBCLI::BufferView>& View,String const& StringToWrite,Int RowOffset,Int ColumnOffset)
    {
        View.Get().WriteCharacters(RowOffset,ColumnOffset,StringToWrite);
    }
    void CLIModule::p_Clear(Temporary<MBCLI::BufferView>& View)
    {
        View.Get().Clear();
    }
    void CLIModule::p_ClearTerm(MBCLI::MBTerminal& Term)
    {
        Term.Clear();
    }
    Temporary<MBCLI::BufferView> CLIModule::p_SubView(Temporary<MBCLI::BufferView>& View,Int RowOffset,Int ColumnOffset)
    {
        return View.CreateRelated(View.Get().SubView(RowOffset,ColumnOffset));
    }
    Temporary<MBCLI::BufferView> CLIModule::p_SubViewHeightWidth(Temporary<MBCLI::BufferView>& View,Int RowOffset,Int ColumnOffset,Int Width,Int Height)
    {
        return View.CreateRelated(View.Get().SubView(RowOffset,ColumnOffset,MBCLI::Dimensions(Width,Height)));
    }
    Int CLIModule::p_Width(Temporary<MBCLI::BufferView>& View)
    {
        return View.Get().GetDimensions().Width;
    }
    Int CLIModule::p_Height(Temporary<MBCLI::BufferView>& View)
    {
        return View.Get().GetDimensions().Height;
    }
    MBCLI::Dimensions CLIModule::p_Dims(Temporary<MBCLI::BufferView>& View)
    {
        return View.Get().GetDimensions();
    }
    MBCLI::Dimensions CLIModule::p_DimsCreate(Int Width,Int Height)
    {
        return MBCLI::Dimensions(Width,Height);
    }
    Int CLIModule::p_WidthDims(MBCLI::Dimensions Dims)
    {
        return Dims.Width;
    }
    Int CLIModule::p_HeightDims(MBCLI::Dimensions Dims)
    {
        return Dims.Height;
    }


    static void SetAtr_Stacker(MBTUI::Stacker& Stacker,String const& Attribute,Value const& Value)
    {
        if(Attribute == "overflow")
        {
            if(Value.IsType<bool>())
            {
                Stacker.EnableOverflow(Value.GetType<bool>());
            }
        }
        else if(Attribute == "reversed")
        {
            if(Value.IsType<bool>())
            {
                Stacker.SetReversed(Value.GetType<bool>());
            }
        }
        else if(Attribute == "overflowreversed")
        {
            if(Value.IsType<bool>())
            {
                Stacker.SetOverflowDirection(Value.GetType<bool>());
            }
        }
        else if(Attribute == "border")
        {
            if(Value.IsType<bool>())
            {
                Stacker.SetBorder(Value.GetType<bool>());
            }
        }
        else if(Attribute == "direction")
        {
            if(Value.IsType<String>())
            {
                if(Value.GetType<String>() == "right")
                {
                    Stacker.SetFlowDirection(false);
                }
            }
        }
        else if(Attribute == "bordercolor")
        {
            if(Value.IsType<String>())
            {
                if(Value.GetType<String>() == "green")
                {
                    Stacker.SetBorderColor(MBCLI::ANSITerminalColor::BrightGreen);
                }
                else if(Value.GetType<String>() == "white")
                {
                    Stacker.SetBorderColor(MBCLI::ANSITerminalColor::BrightWhite);
                }
            }
        }
    }

    Value CLIModule::p_Stacker(Evaluator& Evaluator,Dict& Attributes,List& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Stacker,MBCLI::Window>();
        auto& Stacker = ReturnValue.GetType<MBTUI::Stacker>();


        for(auto const& Attribute : Attributes)
        {
            if(Attribute.first.IsType<String>())
            {
                SetAtr_Stacker(Stacker,Attribute.first.GetType<String>(),Attribute.second);
            }
        }
       

        for(auto& Child : Children)
        {
            if(Child.IsType<MBCLI::Window>())
            {
                Stacker.AddElement(Child.GetSharedPtr<MBCLI::Window>());
            }
            else
            {
                Stacker.AddElement(std::unique_ptr<MBCLI::Window>(std::make_unique<LispWindow>(Evaluator.shared_from_this(),Child)));
            }
        }
        return ReturnValue;
    }
    void CLIModule::p_AddChildStacker(MBTUI::Stacker& Stacker,Ref<MBCLI::Window> Child)
    {
        Stacker.AddElement(MBUtility::SmartPtr<MBCLI::Window>(Child.GetSharedPtr()));
    }
    void CLIModule::p_AddValueChildStacker(Evaluator& Eval,MBTUI::Stacker& Stacker,Value Child)
    {
        Stacker.AddElement(std::unique_ptr<MBCLI::Window>(std::make_unique<LispWindow>(Eval.shared_from_this(),Child)));
    }
    Value CLIModule::p_Repl(Evaluator& Evaluator,Dict& Attributes,List& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::REPL,MBCLI::Window>();
        auto EnterFuncIt = Attributes.find(String("onenter"));
        if(EnterFuncIt != Attributes.end())
        {
            MBUtility::MOFunction<void(std::string const&)> EnterFunc = 
                [Evaluator=Evaluator.shared_from_this(),Func = EnterFuncIt->second](std::string const& Line)
                {
                    Evaluator->Eval(Func,{Line});
                };
            ReturnValue.GetType<MBTUI::REPL>().SetOnEnterFunc(std::move(EnterFunc));
        }
        return ReturnValue;
    }
    MBCLI::Dimensions CLIModule::p_PreferedDims(MBCLI::Window& Window,MBCLI::Dimensions SuggestedDims)
    {
        return Window.PreferedDimensions(SuggestedDims);
    }
    void CLIModule::p_WriteWindowBuiltin(MBCLI::MBTerminal& Terminal,MBCLI::Window& Window)
    {
        Terminal.WriteWindow(Window);
    }
    void CLIModule::p_WriteWindow(Evaluator& CurrentEvaluator,MBCLI::MBTerminal& Terminal,Value Window)
    {
        LispWindow WindowAdapter(CurrentEvaluator.shared_from_this(),Window);
        Terminal.WriteWindow(WindowAdapter);
    }
    MBCLI::Dimensions CLIModule::p_TermDims(MBCLI::MBTerminal& Terminal)
    {
        auto Info = Terminal.GetTerminalInfo();
        return MBCLI::Dimensions(Info.Width,Info.Height);
    }
    Int CLIModule::p_HeightTerm(MBCLI::MBTerminal& Terminal)
    {
        auto Info = Terminal.GetTerminalInfo();
        return Info.Height;
    }
    Int CLIModule::p_WidthTerm(MBCLI::MBTerminal& Terminal)
    {
        auto Info = Terminal.GetTerminalInfo();
        return Info.Width;
    }
    Value CLIModule::p_Terminal()
    {
        return Value::EmplaceExternal<MBCLI::MBTerminal>();
    }
    MBCLI::ConsoleInput CLIModule::p_GetInput(MBCLI::MBTerminal& Terminal)
    {
        return Terminal.ReadNextInput();
    }
    MBCLI::CursorInfo CLIModule::CursorInfoPos(Int X,Int Y)
    {
        MBCLI::CursorInfo ReturnValue;
        ReturnValue.Hidden = false;
        ReturnValue.Position.ColumnIndex = X;
        ReturnValue.Position.RowIndex = Y;
        return ReturnValue;
    }
    MBCLI::CursorInfo CLIModule::CursorInfoPosHidden(Int X,Int Y,bool Hidden)
    {
        MBCLI::CursorInfo ReturnValue;
        ReturnValue.Hidden = Hidden;
        ReturnValue.Position.ColumnIndex = X;
        ReturnValue.Position.RowIndex = Y;
        return ReturnValue;
    }
    //static Value p_Stacker(Dict& Attributes,List& Children);
    //static Value p_Repl(Dict& Attributes,List& Children);
    void CLIModule::SetChildren(MBLisp::Value,MBLisp::Ref<MBLisp::List> Children)
    {
    }
    static void SetChildren_Stacker(Evaluator& Eval,MBTUI::Stacker& Stacker,MBLisp::List Children)
    {
        for(auto& Child : Children)
        {
            if(Child.IsType<MBCLI::Window>())
            {
                Stacker.AddElement(MBUtility::SmartPtr<MBCLI::Window>(Child.GetSharedPtr<MBCLI::Window>()));
            }
            else
            {
                Stacker.AddElement(std::unique_ptr<MBCLI::Window>(std::make_unique<LispWindow>(Eval.shared_from_this(),Child)));
            }
        }
    }



    static void DrawBorder(Temporary<MBCLI::BufferView> TempView)
    {
        auto& View = TempView.Get();
        MBCLI::DrawBorder(View,0,0,View.GetDimensions().Width,View.GetDimensions().Height);
    }
    static void DrawBorder_Offset(Temporary<MBCLI::BufferView> TempView)
    {
        auto& View = TempView.Get();
        MBCLI::DrawBorder(View,0,0,View.GetDimensions().Width,View.GetDimensions().Height);
    }

    static bool Input_Eq(MBCLI::ConsoleInput const& Input,String const& Key)
    {
        return Input.CharacterInput == Key;
    }
    static bool Input_Eq_2(String const& Key,MBCLI::ConsoleInput const& Input)
    {
        return Input.CharacterInput == Key;
    }

    MBLisp::Ref<MBLisp::Scope> CLIModule::GetModuleScope(MBLisp::Evaluator& AssociatedEvaluator)
    {
        auto ReturnValue = MBLisp::MakeRef<MBLisp::Scope>();

        AssociatedEvaluator.AddGeneric<p_WindowUpdated>(ReturnValue,"updated");
        AssociatedEvaluator.AddGeneric<p_WindowHandleInput>(ReturnValue,"handle-input");
        AssociatedEvaluator.AddGeneric<p_WindowSetFocus>(ReturnValue,"set-focus");
        AssociatedEvaluator.AddGeneric<p_GetCursorInfo>(ReturnValue,"get-cursor-info");
        AssociatedEvaluator.AddGeneric<p_PreferedDims>(ReturnValue,"prefered-dims");
        AssociatedEvaluator.AddGeneric<p_Dims>(ReturnValue,"dims");
        AssociatedEvaluator.AddGeneric<p_DimsCreate>(ReturnValue,"dims");


        AssociatedEvaluator.AddGeneric<CursorInfoPosHidden>(ReturnValue,"cursor-info");
        AssociatedEvaluator.AddGeneric<CursorInfoDefault>(ReturnValue,"cursor-info");
        AssociatedEvaluator.AddGeneric<CursorInfoPos>(ReturnValue,"cursor-info");

        AssociatedEvaluator.AddGeneric<p_WriteString>("write");
        AssociatedEvaluator.AddGeneric<p_WriteStringOffset>("write");
        AssociatedEvaluator.AddGeneric<p_WriteBuffer>("write");


        AssociatedEvaluator.AddGeneric<p_SubView>(ReturnValue,"sub-view");
        AssociatedEvaluator.AddGeneric<p_SubViewHeightWidth>(ReturnValue,"sub-view");
        AssociatedEvaluator.AddGeneric<p_Width>(ReturnValue,"width");
        AssociatedEvaluator.AddGeneric<p_Height>(ReturnValue,"height");
        AssociatedEvaluator.AddGeneric<p_WidthDims>(ReturnValue,"width");
        AssociatedEvaluator.AddGeneric<p_HeightDims>(ReturnValue,"height");

        AssociatedEvaluator.AddGeneric<p_Stacker>(ReturnValue,"stacker");
        AssociatedEvaluator.AddGeneric<SetAtr_Stacker>(ReturnValue,"set-atr");
        AssociatedEvaluator.AddGeneric<p_AddValueChildStacker>(ReturnValue,"add-child");
        AssociatedEvaluator.AddGeneric<p_AddChildStacker>(ReturnValue,"add-child");
        AssociatedEvaluator.AddObjectMethod<&MBTUI::Stacker::ClearChildren>(ReturnValue,"clear");
        AssociatedEvaluator.AddGeneric<p_Repl>(ReturnValue,"repl");

        AssociatedEvaluator.AddGeneric<p_Terminal>(ReturnValue,"terminal");
        AssociatedEvaluator.AddGeneric<p_GetInput>(ReturnValue,"get-input");
        AssociatedEvaluator.AddGeneric<p_WriteWindow>(ReturnValue,"write-window");
        AssociatedEvaluator.AddGeneric<p_WriteWindowBuiltin>(ReturnValue,"write-window");
        AssociatedEvaluator.AddGeneric<p_TermDims>(ReturnValue,"dims");
        AssociatedEvaluator.AddGeneric<p_HeightTerm>(ReturnValue,"height");
        AssociatedEvaluator.AddGeneric<p_WidthTerm>(ReturnValue,"width");

        AssociatedEvaluator.AddGeneric<p_Clear>("clear");
        AssociatedEvaluator.AddGeneric<p_ClearTerm>("clear");

        AssociatedEvaluator.AddGeneric<Input_Eq>("eq");
        AssociatedEvaluator.AddGeneric<Input_Eq_2>("eq");

        //AssociatedEvaluator.AddObjectMethod<&MBTUI::Stacker::EnableOverlow>(ReturnValue,"enable-overflow");

        return ReturnValue;
    }
}
