#include "CLI.h"
#include "../../Evaluator.h"

#include <MBTUI/MBTUI.h>
#include <MBTUI/Stacker.h>

#include <MBTUI/SizeSpecification.h>
#include <MBTUI/Absolute.h>
#include <MBTUI/Text.h>
#include <MBTUI/Hider.h>
#include <cstdint>

#include <MBUtility/MBStrings.h>
#include <MBTUI/Spin.h>
namespace MBLisp
{
    static MBCLI::TerminalColor ParseColor(std::string_view Content)
    {
        MBCLI::TerminalColor ReturnValue = MBCLI::ANSITerminalColor::Default;
        if(Content.size() > 0 && Content[0] == '#')
        {
            if(Content.size() == 7)
            {
                std::array<uint8_t,4> Value;
                bool Valid = true;
                Value[0] = MBUtility::HexValueToByte(Content[1],Content[2],&Valid);
                Value[1] = MBUtility::HexValueToByte(Content[3],Content[4],&Valid);
                Value[2] = MBUtility::HexValueToByte(Content[5],Content[6],&Valid);
                Value[3] = 255;
                ReturnValue = Value;
            }
        }
        else
        {
            if(Content == "red")
            {
                ReturnValue = MBCLI::ANSITerminalColor::BrightRed;
            }
            else if(Content == "black")
            {
                ReturnValue = MBCLI::ANSITerminalColor::Black;
            }
            else if(Content == "green")
            {
                ReturnValue = MBCLI::ANSITerminalColor::BrightGreen;
            }
            else if(Content == "yellow")
            {
                ReturnValue = MBCLI::ANSITerminalColor::BrightYellow;
            }
        }
        return ReturnValue;
    }









    LispWindow::LispWindow(std::shared_ptr<Evaluator> Evaluator,Value Val)
    {
        m_Evaluator = Evaluator;
        m_ModuleScope = Evaluator->GetModuleScope("cli");
        m_Value = Val;

        //auto NewParent = Value::EmplaceExternal<std::shared_ptr<MBCLI::UpdateInfo>>(std::make_shared<MBCLI::UpdateInfo>()).GetRef<std::shared_ptr<MBCLI::UpdateInfo>>();
        //(*NewParent)->Parent = GetParentInfo();
        m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"set-parent-info"),{m_Value, Value::EmplaceExternal<std::shared_ptr<MBCLI::UpdateInfo>>(GetParentInfo())  });
    }
    bool LispWindow::HandleInput(MBCLI::ConsoleInput const& Input)
    {
        auto Result = m_Evaluator->Eval(m_ModuleScope,m_Evaluator->GetValue(*m_ModuleScope,"handle-input"),{m_Value,MBLisp::Value::EmplaceExternal<MBCLI::ConsoleInput>(Input)});
        if(Result.IsType<bool>())
        {
            return Result.GetType<bool>();
        }
        return false;
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
        SetUpdated(false);
    }

    bool CLIModule::p_WindowUpdated(MBCLI::Window& Window)
    {
        return Window.Updated();
    }
    bool CLIModule::p_WindowHandleInput(MBCLI::Window& Window,MBCLI::ConsoleInput const& Input)
    {
        return Window.HandleInput(Input);
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



    //Statics

    MBTUI::SizeSpecification GetSizeSpecification(Dict const& Attributes)
    {
        MBTUI::SizeSpecification ReturnValue;
        if(auto HeightIt = Attributes.find(String("height")); HeightIt != Attributes.end())
        {
            if(HeightIt->second.IsType<Int>())
            {
                ReturnValue.SetHeight(HeightIt->second.GetType<Int>());
            }
            else if(HeightIt->second.IsType<String>())
            {
                ReturnValue.SetHeight(HeightIt->second.GetType<String>());
            }
        }
        if(auto WidthIt = Attributes.find(String("width")); WidthIt != Attributes.end())
        {
            if(WidthIt->second.IsType<Int>())
            {
                ReturnValue.SetHeight(WidthIt->second.GetType<Int>());
            }
            else if(WidthIt->second.IsType<String>())
            {
                ReturnValue.SetHeight(WidthIt->second.GetType<String>());
            }
        }
        return ReturnValue;
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
        else if(Attribute == "height")
        {
            if(Value.IsType<Int>())
            {
                auto CurrentSpec = Stacker.GetSizeSpec();
                CurrentSpec.SetHeight(Value.GetType<Int>());
                Stacker.SetSizeSpec(CurrentSpec);
            }
            else if(Value.IsType<String>())
            {
                auto CurrentSpec = Stacker.GetSizeSpec();
                CurrentSpec.SetHeight(Value.GetType<String>());
                Stacker.SetSizeSpec(CurrentSpec);
            }
        }
        else if(Attribute == "width")
        {
            if(Value.IsType<Int>())
            {
                auto CurrentSpec = Stacker.GetSizeSpec();
                CurrentSpec.SetWidth(Value.GetType<Int>());
                Stacker.SetSizeSpec(CurrentSpec);
            }
            else if(Value.IsType<String>())
            {
                auto CurrentSpec = Stacker.GetSizeSpec();
                CurrentSpec.SetWidth(Value.GetType<String>());
                Stacker.SetSizeSpec(CurrentSpec);
            }
        }
        else if(Attribute == "reversed")
        {
            if(Value.IsType<bool>())
            {
                Stacker.SetReversed(Value.GetType<bool>());
            }
        }
        else if(Attribute == "overflow-reversed")
        {
            if(Value.IsType<bool>())
            {
                Stacker.SetOverflowDirection(Value.GetType<bool>());
            }
        }
        else if(Attribute == "passthrough")
        {
            if(Value.IsType<String>())
            {
                std::vector<std::string> Characters;
                Characters = MBUtility::Split(Value.GetType<String>(),",");
                Stacker.SetInputPassthrough(Characters);
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
        else if(Attribute == "border-color")
        {
            if(Value.IsType<String>())
            {
                Stacker.SetBorderColor(ParseColor(Value.GetType<String>()));
            }
        }
        else if(Attribute == "bg-color")
        {
            if(Value.IsType<String>())
            {
                Stacker.SetBGColor(ParseColor(Value.GetType<String>()));
            }
        }
        else if(Attribute == "text-color")
        {
            if(Value.IsType<String>())
            {
                Stacker.SetTextColor(ParseColor(Value.GetType<String>()));
            }
        }
        else if(Attribute == "justification")
        {
            if(Value.IsType<String>())
            {
                MBTUI::Stacker::Justification Justification;
                auto Res = MBTUI::Stacker::ParseJustification(Value.GetType<String>(),Justification);
                if(Res)
                {
                    Stacker.SetJustification(Justification);
                }
            }
        }
    }
    static void SetChildren_Stacker(Evaluator& Eval,MBTUI::Stacker& Stacker,MBLisp::List& Children)
    {
        Stacker.ClearChildren();
        for(auto& Child : Children)
        {
            if(Child.IsType<MBCLI::Window>())
            {
                Stacker.AddElement(Child.GetSharedPtr<MBCLI::Window>());
            }
            else
            {
                Stacker.AddElement(std::shared_ptr<MBCLI::Window>(std::make_shared<LispWindow>(Eval.shared_from_this(),Child)));
            }
        }
    }
    static void ClearChildren_Stacker(MBTUI::Stacker& Stacker)
    {
        Stacker.ClearChildren();
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
        SetChildren_Stacker(Evaluator,Stacker,Children);

        return ReturnValue;
    }


    //void CLIModule::p_AddChildStacker(MBTUI::Stacker& Stacker,Ref<MBCLI::Window> Child)
    //{
    //    Stacker.AddElement(MBUtility::SmartPtr<MBCLI::Window>(Child.GetSharedPtr()));
    //}
    void CLIModule::p_AddValueChildStacker(Evaluator& Eval,MBTUI::Stacker& Stacker,Value Child)
    {
        if(Child.IsType<MBCLI::Window>())
        {
            Stacker.AddElement(std::shared_ptr<MBCLI::Window>(std::make_shared<BuiltinWindowAdapter>(std::move(Child))));
        }
        else
        {
            Stacker.AddElement(std::shared_ptr<MBCLI::Window>(std::make_shared<LispWindow>(Eval.shared_from_this(),std::move(Child))));
        }
    }

    Value i_Spin(Evaluator& Evaluator,Dict& Attributes,List& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::SpinWindow,MBCLI::Window>();
        return ReturnValue;
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
    Value GetLine_Repl(MBTUI::REPL& Repl)
    {
        return String(Repl.GetLineString());
    }
    void SetLine_Repl(MBTUI::REPL& Repl,String const& Content)
    {
        Repl.SetText(Content);
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
    static bool CompareInputString(MBCLI::ConsoleInput const& Input,String const& Key)
    {
        return Input == Key;
    }
    static bool Input_Eq(MBCLI::ConsoleInput const& Input,String const& Key)
    {
        return CompareInputString(Input,Key);
    }
    static bool Input_Eq_2(String const& Key,MBCLI::ConsoleInput const& Input)
    {
        return CompareInputString(Input,Key);
    }
    static bool Ctrl(MBCLI::ConsoleInput const& Input)
    {
        return (Input.KeyModifiers & uint64_t(MBCLI::KeyModifier::CTRL)) != 0;
    }
    static bool Ctrl_Input(MBCLI::ConsoleInput const& Input,String const& Character)
    {
        if(Character.size() != 1)
        {
            throw std::runtime_error("Can only compare control character for single char string");
        }
        return (Input.KeyModifiers & uint64_t(MBCLI::KeyModifier::CTRL)) != 0 &&  Input.CharacterInput == MBCLI::CTRL(Character[0]);
    }


    static void SetParentInfo(Value const& Value,std::shared_ptr<MBCLI::UpdateInfo> Parent)
    {
        int hej = 2;
    }
    static void SetParentInfo_Window(MBCLI::Window& Value,std::shared_ptr<MBCLI::UpdateInfo> Parent)
    {
        Value.GetParentInfo()->Parent = Parent;
    }
    static void SetParentInfo_UpdatedInfo(std::shared_ptr<MBCLI::UpdateInfo>& Value,std::shared_ptr<MBCLI::UpdateInfo> Parent)
    {
        Value->Parent = Parent;
    }
    static void SetUpdated(std::shared_ptr<MBCLI::UpdateInfo> Parent,bool Updated)
    {
        if(Updated)
        {
            Parent->Update();
        }
        else
        {
            Parent->Updated = false;
        }
    }

    static Value NewUpdatedInfo()
    {
        return Value::EmplaceExternal<std::shared_ptr<MBCLI::UpdateInfo>>(std::make_shared<MBCLI::UpdateInfo>());
    }
    static bool Updated(std::shared_ptr<MBCLI::UpdateInfo> Parent)
    {
        return Parent->Updated;
    }

    static Value GetSelected(MBTUI::Stacker& Stacker)
    {
        if(!Stacker.WindowSelected())
        {
            return Value();
        }
        auto& SelectedWindow = Stacker.GetSelectedWindow();
        auto Shared = SelectedWindow.GetShared();
        //TODO: would rather avoid dynamic_cast if possible...
        auto Builtin = dynamic_cast<BuiltinWindowAdapter*>(Shared.get());
        if(Builtin != nullptr)
        {
            return Builtin->GetLispValue();
        }
        auto Lisp = dynamic_cast<LispWindow*>(Shared.get());
        if(Lisp != nullptr)
        {
            return Lisp->GetUnderylingValue();
        }
        return FromShared(Shared);
    }
    static Value GetFirst_Stacker(MBTUI::Stacker& Stacker)
    {
        auto& SelectedWindow = Stacker.First();
        auto Shared = SelectedWindow.GetShared();
        //TODO: would rather avoid dynamic_cast if possible...
        auto Builtin = dynamic_cast<BuiltinWindowAdapter*>(Shared.get());
        if(Builtin != nullptr)
        {
            return Builtin->GetLispValue();
        }
        auto Lisp = dynamic_cast<LispWindow*>(Shared.get());
        if(Lisp != nullptr)
        {
            return Lisp->GetUnderylingValue();
        }
        return FromShared(Shared);
    }
    static Value ChildCount_Stacker(MBTUI::Stacker& Stacker)
    {
        return Int(Stacker.ChildCount());
    }

    void SetAttribute_Absolute(MBTUI::Absolute& Absolute,String const& Atr,Value const& Val)
    {
        if(Atr == "orientation")
        {
            if(Val.IsType<String>())
            {
                Absolute.SetOrientation(MBTUI::StringToOrientation(Val.GetType<String>()));
            }
        }
        else if(Atr == "relative")
        {
            if(Val.IsType<bool>())
            {
                Absolute.SetRelative(Val.GetType<bool>());
            }
        }
        else if(Atr == "row-offset")
        {
            if(Val.IsType<Int>())
            {
                auto Offset = Absolute.GetOffsets();
                Offset.Height = Val.GetType<Int>();
                Absolute.SetOffsets(Offset.Height,Offset.Width);
            }
        }
        else if(Atr == "col-offset")
        {
            if(Val.IsType<Int>())
            {
                auto Offset = Absolute.GetOffsets();
                Offset.Width = Val.GetType<Int>();
                Absolute.SetOffsets(Offset.Height,Offset.Width);
            }
        }
        else if(Atr == "visible")
        {
            if(Val.IsType<bool>())
            {
                Absolute.SetVisible(Val.GetType<bool>());   
            }
        }
    }

    Value CreateAbsolute(Evaluator& Evaluator,Dict& Attributes,List& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Absolute,MBCLI::Window>();
        auto& Stacker = ReturnValue.GetType<MBTUI::Absolute>();
        for(auto const& Attribute : Attributes)
        {
            if(Attribute.first.IsType<String>())
            {
                SetAttribute_Absolute(Stacker,Attribute.first.GetType<String>(),Attribute.second);
            }
        }
       

        for(auto& Child : Children)
        {
            if(Child.IsType<MBCLI::Window>())
            {
                Stacker.SetSubwindow(Child.GetSharedPtr<MBCLI::Window>());
            }
            else
            {
                Stacker.SetSubwindow(std::shared_ptr<MBCLI::Window>(std::make_shared<LispWindow>(Evaluator.shared_from_this(),Child)));
            }
            break;
        }
        return ReturnValue;
    }
    static void SetChild_Absolute(Evaluator& Eval,MBTUI::Absolute& Abs,Value Child)
    {
        if(Child.IsType<MBCLI::Window>())
        {
            Abs.SetSubwindow(Child.GetSharedPtr<MBCLI::Window>());
        }
        else
        {
            Abs.SetSubwindow(std::shared_ptr<MBCLI::Window>(std::make_shared<LispWindow>(Eval.shared_from_this(),Child)));
        }
    }

    void Text_SetAtr(MBTUI::Text& Text,String const& Atr,Value const& Val)
    {
        if(Atr == "color")
        {
            if(Val.IsType<String>())
            {
                Text.SetColor(ParseColor(Val.GetType<String>()));
            }
        }
        else if(Atr == "content")
        {
            if(Val.IsType<String>())
            {
                Text.SetText(Val.GetType<String>());
            }
        }
        else if(Atr == "highlight-color")
        {
            if(Val.IsType<String>())
            {
                Text.SetHighlightColor(ParseColor(Val.GetType<String>()));
            }
        }
        else if(Atr == "bg-color")
        {
            if(Val.IsType<String>())
            {
                Text.SetBGColor(ParseColor(Val.GetType<String>()));
            }
        }
        else if(Atr == "highlight-bg-color")
        {
            if(Val.IsType<String>())
            {
                Text.SetHighlightBGColor(ParseColor(Val.GetType<String>()));
            }
        }
        else if(Atr == "multiline")
        {
            if(Val.IsType<bool>())
            {
                Text.SetMultiline(Val.GetType<bool>());
            }
        }
    }
    Value Text_Create(String const& Content)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Text,MBCLI::Window>();
        auto& Text = ReturnValue.GetType<MBTUI::Text>();
        Text.SetText(Content);
        return ReturnValue;
    }
    Value Text_CreateDict(Dict const& Atrs,List const& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Text,MBCLI::Window>();
        auto& Text = ReturnValue.GetType<MBTUI::Text>();
        for(auto const& Atr : Atrs)
        {
            if(Atr.first.IsType<String>())
            {
                Text_SetAtr(Text,Atr.first.GetType<String>(),Atr.second);
            }
        }
        return ReturnValue;
    }
    Value Text_Attributes(String const& Content,Dict& Attributes)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Text,MBCLI::Window>();
        auto& Text = ReturnValue.GetType<MBTUI::Text>();
        for(auto const& Atr : Attributes)
        {
            if(Atr.first.IsType<String>())
            {
                Text_SetAtr(Text,Atr.first.GetType<String>(),Atr.second);
            }
        }
        Text.SetText(Content);
        return ReturnValue;
    }
    Value Text_GetContent(MBTUI::Text& Element)
    {
        return String(Element.GetContent());
    }

    Value NewInput(String const& String)
    {
        Value ReturnValue = Value::EmplaceExternal<MBCLI::ConsoleInput>();

        ReturnValue.GetType<MBCLI::ConsoleInput>().CharacterInput = String;
        return ReturnValue;
    }


    void SetAttribute_Hider(MBTUI::Hider& Hider,String const& Atr,Value const& Value)
    {
        if(Atr == "visible")
        {
            if(Value.IsType<bool>())
            {
                Hider.SetVisible(Value.GetType<bool>());
            }
        }
    }


    Value CreateHider(Evaluator& Evaluator,Dict& Attributes,List& Children)
    {
        auto ReturnValue = Value::EmplacePolymorphic<MBTUI::Hider,MBCLI::Window>();
        auto& Hider = ReturnValue.GetType<MBTUI::Hider>();
        for(auto const& Attribute : Attributes)
        {
            if(Attribute.first.IsType<String>())
            {
                SetAttribute_Hider(Hider,Attribute.first.GetType<String>(),Attribute.second);
            }
        }
       

        for(auto& Child : Children)
        {
            if(Child.IsType<MBCLI::Window>())
            {
                Hider.SetSubwindow(Child.GetSharedPtr<MBCLI::Window>());
            }
            else
            {
                Hider.SetSubwindow(std::shared_ptr<MBCLI::Window>(std::make_shared<LispWindow>(Evaluator.shared_from_this(),Child)));
            }
            break;
        }
        return ReturnValue;
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


        AssociatedEvaluator.AddGeneric<SetParentInfo>(ReturnValue,"set-parent-info");
        AssociatedEvaluator.AddGeneric<SetParentInfo_UpdatedInfo>(ReturnValue,"set-parent-info");
        AssociatedEvaluator.AddGeneric<SetParentInfo_Window>(ReturnValue,"set-parent-info");
        AssociatedEvaluator.AddGeneric<NewUpdatedInfo>(ReturnValue,"new-updated-info");
        AssociatedEvaluator.AddGeneric<SetUpdated>(ReturnValue,"set-updated");
        AssociatedEvaluator.AddGeneric<Updated>(ReturnValue,"updated");


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


        AssociatedEvaluator.AddGeneric<i_Spin>(ReturnValue,"spin");

        AssociatedEvaluator.AddGeneric<p_Stacker>(ReturnValue,"stacker");
        AssociatedEvaluator.AddGeneric<SetAtr_Stacker>(ReturnValue,"set-atr");
        AssociatedEvaluator.AddGeneric<SetAttribute_Absolute>(ReturnValue,"set-atr");
        AssociatedEvaluator.AddGeneric<p_AddValueChildStacker>(ReturnValue,"add-child");
        AssociatedEvaluator.AddGeneric<GetSelected>(ReturnValue,"get-selected");
        AssociatedEvaluator.AddGeneric<GetFirst_Stacker>(ReturnValue,"first");
        AssociatedEvaluator.AddGeneric<ChildCount_Stacker>(ReturnValue,"child-count");
        AssociatedEvaluator.AddGeneric<SetChildren_Stacker>(ReturnValue,"set-children");
        AssociatedEvaluator.AddGeneric<ClearChildren_Stacker>(ReturnValue,"clear-children");
        //AssociatedEvaluator.AddGeneric<p_AddChildStacker>(ReturnValue,"add-child");
        AssociatedEvaluator.AddObjectMethod<&MBTUI::Stacker::ClearChildren>(ReturnValue,"clear");
        AssociatedEvaluator.AddGeneric<p_Repl>(ReturnValue,"repl");
        AssociatedEvaluator.AddGeneric<GetLine_Repl>("get-line");
        AssociatedEvaluator.AddGeneric<SetLine_Repl>(ReturnValue,"set-line");

        AssociatedEvaluator.AddGeneric<NewInput>(ReturnValue,"create-input");
        AssociatedEvaluator.AddGeneric<Ctrl>(ReturnValue,"ctrl");
        AssociatedEvaluator.AddGeneric<Ctrl_Input>(ReturnValue,"ctrl");


        AssociatedEvaluator.AddGeneric<CreateAbsolute>(ReturnValue,"absolute");
        AssociatedEvaluator.AddGeneric<SetChild_Absolute>(ReturnValue,"set-child");

        AssociatedEvaluator.AddGeneric<Text_Create>(ReturnValue,"Text");
        AssociatedEvaluator.AddGeneric<Text_CreateDict>(ReturnValue,"Text");
        AssociatedEvaluator.AddGeneric<Text_Attributes>(ReturnValue,"Text");
        AssociatedEvaluator.AddGeneric<Text_GetContent>(ReturnValue,"get-content");
        AssociatedEvaluator.AddType<MBTUI::Text>(ReturnValue,"Text_t");

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


        AssociatedEvaluator.AddType<MBCLI::ConsoleInput>(ReturnValue,"input_t");
        AssociatedEvaluator.AddType<MBTUI::Stacker>(ReturnValue,"stacker_t");
        AssociatedEvaluator.AddType<MBTUI::Absolute>(ReturnValue,"absolute_t");
        AssociatedEvaluator.AddType<MBCLI::Dimensions>(ReturnValue,"dims_t");
        AssociatedEvaluator.AddType<MBCLI::Window>(ReturnValue,"window_t");
        AssociatedEvaluator.AddType<MBTUI::Text>(ReturnValue,"text_t");
        AssociatedEvaluator.AddType<MBTUI::REPL>(ReturnValue,"repl_t");
        AssociatedEvaluator.AddType<MBTUI::SpinWindow>(ReturnValue,"spin_t");


        //Hider
        AssociatedEvaluator.AddGeneric<CreateHider>(ReturnValue,"hider");
        AssociatedEvaluator.AddGeneric<SetAttribute_Hider>(ReturnValue,"set-atr");
        AssociatedEvaluator.AddGeneric<Text_SetAtr>(ReturnValue,"set-atr");
        //

        //AssociatedEvaluator.AddObjectMethod<&MBTUI::Stacker::EnableOverlow>(ReturnValue,"enable-overflow");

        return ReturnValue;
    }
}
