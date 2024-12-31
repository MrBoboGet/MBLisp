#pragma  once
#include "../../Module.h"
#include <MBCLI/Window.h>
#include <MBTUI/Stacker.h>
namespace MBLisp
{

    template<typename T> class TemporaryLock;
    template<typename T>
    class Temporary
    {
        friend class TemporaryLock<T>;
        std::shared_ptr<T> m_Ptr = nullptr;
        std::shared_ptr<bool> m_Alive;
    private:
        Temporary() = default;
        Temporary(TemporaryLock<T>& Lock);
    public:
        Temporary(Temporary&&) noexcept = default;
        Temporary(Temporary const&) = default;
        Temporary& operator=(Temporary&&) noexcept = default;
        Temporary& operator=(Temporary const&) = default;
        T& Get()
        {
            if(!*m_Alive)
            {
                throw std::runtime_error("Error acessing temporary value: lifetime has elapsed");   
            }
            return *m_Ptr;   
        }
        template<typename... ArgTypes>
        Temporary CreateRelated(ArgTypes&&... Args)
        {
            std::shared_ptr<T> NewPtr = std::shared_ptr<T>(new T(std::forward<ArgTypes>(Args)...));
            Temporary ReturnValue;
            ReturnValue.m_Ptr = NewPtr;
            ReturnValue.m_Alive = m_Alive;
            return ReturnValue;
        }
    };

    template<typename T>
    class TemporaryLock
    {
        friend class Temporary<T>;
        std::shared_ptr<bool> m_Alive = std::make_shared<bool>(true);
        std::shared_ptr<T> m_Ptr = nullptr;

    public:

        TemporaryLock(T& Value)
        {
            m_Ptr = std::shared_ptr<T>(&Value,[](T*){});
        }
        Temporary<T> GetTemporary()
        {
            return Temporary<T>(*this);
        }
        ~TemporaryLock()
        {
            *m_Alive = false;   
        }
    };
    template<typename T>
    Temporary<T>::Temporary(TemporaryLock<T>& Lock)
    {
        m_Ptr = Lock.m_Ptr;
        m_Alive = Lock.m_Alive;
    }

    struct WriteState
    {
        
    };
    class LispWindow : public MBCLI::Window
    {
    protected:
        MBLisp::Value m_Value;
        std::shared_ptr<MBLisp::Evaluator> m_Evaluator;
        MBLisp::Ref<MBLisp::Scope> m_ModuleScope;
    public:
        LispWindow(std::shared_ptr<Evaluator> Evaluator,Value Val);

        Evaluator& GetEvaluator()
        {
            return *m_Evaluator;
        }
        Value& GetUnderylingValue()
        {
            return m_Value;   
        }

        virtual void HandleInput(MBCLI::ConsoleInput const& Input);
        virtual MBCLI::Dimensions PreferedDimensions(MBCLI::Dimensions SuggestedDimensions);
        virtual void SetFocus(bool IsFocused);
        virtual MBCLI::CursorInfo GetCursorInfo();
        //virtual TerminalWindowBuffer GetBuffer() { return TerminalWindowBuffer();};
        virtual void WriteBuffer(MBCLI::BufferView View,bool Redraw);
    };
    class BuiltinWindowAdapter : public MBCLI::Window
    {
    protected:
        MBCLI::Window* m_Window = nullptr;
        Value m_AssociatedValue;
    public:
        BuiltinWindowAdapter(Value Window)
        {
            if(!Window.IsType<MBCLI::Window>())
            {
                throw std::runtime_error("BuiltinWindowAdapter can only be initialized with value containing a window");
            }
            m_Window = &Window.GetType<MBCLI::Window>();
            m_AssociatedValue = std::move(Window);
        }
        Value GetLispValue() 
        {
            return m_AssociatedValue;   
        }
        virtual void HandleInput(MBCLI::ConsoleInput const& Input)
        {
            m_Window->HandleInput(Input);   
        }
        virtual MBCLI::Dimensions PreferedDimensions(MBCLI::Dimensions SuggestedDimensions)
        {
            return m_Window->PreferedDimensions(SuggestedDimensions);
        }
        virtual void SetFocus(bool IsFocused)
        {
            m_Window->SetFocus(IsFocused);   
        }
        virtual MBCLI::CursorInfo GetCursorInfo()
        {
            return m_Window->GetCursorInfo();
        }
        //virtual TerminalWindowBuffer GetBuffer() { return TerminalWindowBuffer();};
        virtual void WriteBuffer(MBCLI::BufferView View,bool Redraw)
        {
            m_Window->WriteBuffer(std::move(View),Redraw);
        }
    };

    class CLIModule : public MBLisp::Module
    {

        static bool p_WindowUpdated(MBCLI::Window& Window);
        static void p_WindowHandleInput(MBCLI::Window& Window,MBCLI::ConsoleInput const& Input);
        static void p_WindowSetFocus(MBCLI::Window& Window,bool Focused);
        static MBCLI::CursorInfo p_GetCursorInfo(MBCLI::Window& Window);
        static void p_WriteBuffer(MBCLI::Window& Window,Temporary<MBCLI::BufferView> View,bool Redraw);
        //static void p_WriteBufferValue(Value Val,Temporary<MBCLI::BufferView> View,bool Redraw);


        static void SetChildren(MBLisp::Value,MBLisp::Ref<MBLisp::List> Children);

        static MBCLI::CursorInfo CursorInfoPosHidden(Int X,Int Y,bool Hidden);
        static MBCLI::CursorInfo CursorInfoPos(Int X,Int Y);
        static MBCLI::CursorInfo CursorInfoDefault();

        static void p_WriteString(Temporary<MBCLI::BufferView>& View,String const& StringToWrite);
        static void p_WriteStringOffset(Temporary<MBCLI::BufferView>& View,String const& StringToWrite,Int RowOffset,Int ColumnOffset);
        static Temporary<MBCLI::BufferView> p_SubView(Temporary<MBCLI::BufferView>& View,Int RowOffset,Int ColumnOffset);
        static Temporary<MBCLI::BufferView> p_SubViewHeightWidth(Temporary<MBCLI::BufferView>& View,Int RowOffset,Int ColumnOffset,Int Width,Int Height);
        static void p_Clear(Temporary<MBCLI::BufferView>& View);
        static void p_ClearTerm(MBCLI::MBTerminal& Term);

        static Int p_Width(Temporary<MBCLI::BufferView>& View);
        static Int p_Height(Temporary<MBCLI::BufferView>& View);
        static MBCLI::Dimensions p_Dims(Temporary<MBCLI::BufferView>& View);
        static MBCLI::Dimensions p_DimsCreate(Int Width,Int Height);
        static Int p_WidthDims(MBCLI::Dimensions Dims);
        static Int p_HeightDims(MBCLI::Dimensions Dims);

        static MBCLI::Dimensions p_PreferedDims(MBCLI::Window& Window,MBCLI::Dimensions SuggestedDims);


        static Value p_Terminal();
        static MBCLI::ConsoleInput p_GetInput(MBCLI::MBTerminal& Terminal);
        static void p_WriteWindowBuiltin(MBCLI::MBTerminal& Terminal,MBCLI::Window& Window);
        static void p_WriteWindow(Evaluator& CurrentEvaluator,MBCLI::MBTerminal& Terminal,Value Window);
        static MBCLI::Dimensions p_TermDims(MBCLI::MBTerminal& Terminal);
        static Int p_HeightTerm(MBCLI::MBTerminal& Terminal);
        static Int p_WidthTerm(MBCLI::MBTerminal& Terminal);


        static Value p_Stacker(Evaluator& Evaluator,Dict& Attributes,List& Children);
        //static void p_AddChildStacker(MBTUI::Stacker& Stacker,Ref<MBCLI::Window> Child);
        static void p_AddValueChildStacker(Evaluator& Eval,MBTUI::Stacker& Stacker,Value Child);



        static Value p_Repl(Evaluator& Evaluator,Dict& Attributes,List& Children);

    public:
        virtual MBLisp::Ref<MBLisp::Scope> GetModuleScope(MBLisp::Evaluator& AssociatedEvaluator);
    };
}
