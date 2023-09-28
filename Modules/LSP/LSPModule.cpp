#include "LSPModule.h"
#include <MBUtility/InterfaceAdaptors.h>
#include <MBUtility/MBFiles.h>

#include <MBUnicode/MBUnicode.h>
namespace MBLisp
{
    Value LSPHandler::CreateLSPHandler BUILTIN_ARGLIST
    {
        return Value::MakeExternal(LSPHandler());
    }
    //takes string, method, and callable -> string
    Value LSPHandler::AddGenericRequestHandler BUILTIN_ARGLIST
    {
        Value ReturnValue;

        return ReturnValue;
    }
    //document string, list of (symbol type) pairs
    Value LSPHandler::SetDocumentTokens BUILTIN_ARGLIST
    {
        Value ReturnValue;
        LSPHandler& Handler = Arguments[0].GetType<LSPHandler>();
        String& DocumentID = Arguments[1].GetType<String>();
        List& Tokens  = Arguments[2].GetType<List>();

        auto& Document = Handler.m_OpenedDocuments[DocumentID];
        //if(DocumentIt == Handler.m_OpenedDocuments.end())
        //{
        //    throw std::runtime_error("Cannot find document \""+Document+"\" among opened documents");   
        //}
        std::vector<MBLSP::SemanticToken> NewTokens;
        for(auto const& Token : Tokens)
        {
            if(!Token.IsType<List>())
            {
                throw std::runtime_error("Token has to be a list containing a symbol and a string");
            }
            List const& TokenList = Token.GetType<List>();
            if(TokenList.size() != 2 || !(TokenList[0].IsType<Symbol>() && TokenList[1].IsType<String>()))
            {
                throw std::runtime_error("Token has to be a list containing a symbol and a string");  
            }
            Symbol const& SymbolToken = TokenList[0].GetType<Symbol>();
            MBLSP::SemanticToken NewToken;
            NewToken.Length = Context.GetEvaluator().GetSymbolString(SymbolToken.ID).length();
            NewToken.Pos = Document.Lines.ByteOffsetToPosition(SymbolToken.SymbolLocation.Position);
            NewToken.Type = StringToTokenType(TokenList[1].GetType<String>());
            NewTokens.push_back(NewToken);
        }
        std::sort(NewTokens.begin(),NewTokens.end());
        Document.Tokens = MBLSP::CombineTokens(NewTokens, Document.Tokens);
        return ReturnValue;
    }
    Value LSPHandler::SetDocumentDiagnostics BUILTIN_ARGLIST
    {
        Value ReturnValue;
        LSPHandler& Handler = Arguments[0].GetType<LSPHandler>();
        String& Document = Arguments[1].GetType<String>();
        List& Tokens  = Arguments[2].GetType<List>();

        auto& DocumentIt = Handler.m_OpenedDocuments[Document];
        std::vector<MBLSP::Diagnostic> NewDiagnostics;
        for(auto const& Diagnostic : Tokens)
        {
            if(!Diagnostic.IsType<List>())
            {
                throw std::runtime_error("Diagnostic has to be a list containing a symbol and a string");
            }
            List const& DiagnosticList = Diagnostic.GetType<List>();
            if(DiagnosticList.size() != 2 || !(DiagnosticList[0].IsType<Symbol>() && DiagnosticList[1].IsType<String>()))
            {
                throw std::runtime_error("Token has to be a list containing a symbol and a string");  
            }
            Symbol const& DiagnosticToken = DiagnosticList[0].GetType<Symbol>();
            MBLSP::Diagnostic NewDiagnostic;
            NewDiagnostic.range.start = DocumentIt.Lines.ByteOffsetToPosition(DiagnosticToken.SymbolLocation.Position);
            NewDiagnostic.range.end = NewDiagnostic.range.start + Context.GetEvaluator().GetSymbolString(DiagnosticToken.ID).length();
            NewDiagnostic.message = DiagnosticList[1].GetType<String>();
            NewDiagnostics.push_back(NewDiagnostic);
        }
        DocumentIt.Diagnostics = std::move(NewDiagnostics);
        return ReturnValue;
    }
    Value LSPHandler::SetDocumentJumps BUILTIN_ARGLIST
    {
        Value ReturnValue;
        LSPHandler& Handler = Arguments[0].GetType<LSPHandler>();
        String& DocumentID = Arguments[1].GetType<String>();
        List& Tokens  = Arguments[2].GetType<List>();

        auto& Document = Handler.m_OpenedDocuments[DocumentID];
        std::vector<JumpSymbol> NewJumps;
        for(auto const& Jump : Tokens)
        {
            if(!Jump.IsType<List>())
            {
                throw std::runtime_error("Jump has to be a list containing two symbols");
            }
            List const& DiagnosticList = Jump.GetType<List>();
            if(DiagnosticList.size() != 2 || !(DiagnosticList[0].IsType<Symbol>() && DiagnosticList[1].IsType<Symbol>()))
            {
                throw std::runtime_error("Jump has to be a list containing two symbols");  
            }
            JumpSymbol NewJump;
            NewJump.Source = Document.Lines.ByteOffsetToPosition(DiagnosticList[0].GetType<Symbol>().SymbolLocation.Position);
            NewJump.Target = DiagnosticList[1].GetType<Symbol>();
            NewJump.Length = Context.GetEvaluator().GetSymbolString(DiagnosticList[0].GetType<Symbol>().ID).length();
            NewJumps.push_back(NewJump);
        }
        std::sort(NewJumps.begin(),NewJumps.end());
        Document.Jumps = std::move(NewJumps);
        return ReturnValue;
    }
    Value LSPHandler::AddOnOpenHandler BUILTIN_ARGLIST
    {
        Value ReturnValue;
        LSPHandler& Handler = Arguments[0].GetType<LSPHandler>();
        Handler.m_OnOpenHandlers.push_back(Arguments[1]);
        return ReturnValue;
    }

    LSPHandler::DocumentInfo LSPHandler::p_CreateDocumentInfo(std::string const& Content)
    {
        DocumentInfo ReturnValue;
        ReturnValue.Lines = MBLSP::LineIndex(Content);
        return ReturnValue;
    }
    MBLSP::TokenType LSPHandler::StringToTokenType(std::string_view StringToConvert)
    {
        MBLSP::TokenType ReturnValue = MBLSP::TokenType::Property;
        if(StringToConvert == "function")
        {
            ReturnValue = MBLSP::TokenType::Function;
        }
        else if(StringToConvert == "macro")
        {
            ReturnValue = MBLSP::TokenType::Macro;
        }
        else if(StringToConvert == "class")
        {
            ReturnValue = MBLSP::TokenType::Class;
        }
        return ReturnValue;
    }
    MBLSP::Initialize_Response LSPHandler::HandleRequest(MBLSP::InitializeRequest const& Request)
    {
        MBLSP::Initialize_Response ReturnValue;
        ReturnValue.result = MBLSP::Initialize_Result();
        ReturnValue.result->capabilities.textDocumentSync = MBLSP::TextDocumentSyncOptions();
        //ReturnValue.result->capabilities.declarationProvider = true;
        ReturnValue.result->capabilities.semanticTokensProvider = MBLSP::SemanticTokensOptions();
        MBLSP::SemanticTokensLegend Legend;
        Legend.tokenTypes = MBLSP::GetTokenLegend();
        ReturnValue.result->capabilities.semanticTokensProvider->legend = Legend;
        ReturnValue.result->capabilities.semanticTokensProvider->full = true;
        ReturnValue.result->capabilities.semanticTokensProvider->range = false;

        ReturnValue.result->capabilities.definitionProvider = true;

        return ReturnValue;
    }
    void LSPHandler::OpenedDocument(std::string const& URI,std::string const& Content)
    {
        DocumentInfo& Newdocument = m_OpenedDocuments[URI];
        Newdocument = p_CreateDocumentInfo(Content);
        p_RunOpenHandlers(Newdocument,URI,Content);
    }
    void LSPHandler::ClosedDocument(std::string const& URI)
    {
        //keeping documents in memory is fine for the purpose of jump to definiton

        //if(auto It = m_OpenedDocuments.find(URI); It != m_OpenedDocuments.end())
        //{
        //    m_OpenedDocuments.erase(It);   
        //}
    }
    MBLSP::SemanticToken_Response LSPHandler::HandleRequest(MBLSP::SemanticToken_Request const& Request)
    {
        MBLSP::SemanticToken_Response ReturnValue;
        ReturnValue.result = MBLSP::SemanticTokens();
        ReturnValue.result->data = MBLSP::CalculateSemanticTokens(m_OpenedDocuments[Request.params.textDocument.uri].Tokens);
        return ReturnValue;
    }
    MBLSP::SemanticTokensRange_Response LSPHandler::HandleRequest(MBLSP::SemanticTokensRange_Request const& Request)
    {
        MBLSP::SemanticTokensRange_Response ReturnValue;
        ReturnValue.result = MBLSP::SemanticTokens();
        ReturnValue.result->data = MBLSP::GetTokenRange(MBLSP::CalculateSemanticTokens(m_OpenedDocuments[Request.params.textDocument.uri].Tokens),Request.params.range);
        return ReturnValue;
    }
    MBLSP::GotoDefinition_Response LSPHandler::HandleRequest(MBLSP::GotoDefinition_Request const& Request)
    {
        MBLSP::GotoDefinition_Response ReturnValue;
        ReturnValue.result = std::vector<MBLSP::Location>();
        auto& DocumentIt = m_OpenedDocuments[Request.params.textDocument.uri];
        auto const& Jumps = DocumentIt.Jumps;
        auto TargetIt = std::lower_bound(Jumps.begin(),Jumps.end(),Request.params.position,[](JumpSymbol const& lhs,MBLSP::Position const& rhs)
                {
                    return lhs.Source +  lhs.Length < rhs;
                });
        if(TargetIt != Jumps.end() && MBLSP::Contains(TargetIt->Source,TargetIt->Length,Request.params.position))
        {
            MBLSP::Location NewLocation;
            NewLocation.uri = m_Evaluator->GetSymbolString(TargetIt->Target.SymbolLocation.URI);
            auto JumpIt = m_OpenedDocuments.find(NewLocation.uri);
            if(JumpIt != m_OpenedDocuments.end())
            {
                NewLocation.range.start = JumpIt->second.Lines.ByteOffsetToPosition(TargetIt->Target.SymbolLocation.Position);
                NewLocation.range.end = NewLocation.range.start + m_Evaluator->GetSymbolString(TargetIt->Target.ID).size();
            }
            else
            {
                std::string FileData = MBUtility::ReadWholeFile(NewLocation.uri);
                MBLSP::LineIndex Index(FileData);
                NewLocation.range.start = Index.ByteOffsetToPosition(TargetIt->Target.SymbolLocation.Position);
                NewLocation.range.end = NewLocation.range.start + m_Evaluator->GetSymbolString(TargetIt->Target.ID).size();
                NewLocation.uri = MBLSP::URLEncodePath(std::filesystem::absolute(NewLocation.uri));
            }
            ReturnValue.result->push_back(NewLocation);
        }
        return ReturnValue;
    }
    void LSPHandler::DocumentChanged(std::string const& URI,std::string const& NewContent, std::vector<MBLSP::TextChange> const& Changes)
    {
        DocumentInfo& Document = m_OpenedDocuments[URI];
        Document.Tokens = MBLSP::UpdateSemanticTokens(Document.Tokens,Changes);
        Document.Lines = MBLSP::LineIndex(NewContent);
        p_RunOpenHandlers(Document,URI,NewContent);
    }
    std::string LSPHandler::p_CanonURI(std::string const& URI)
    {
        return MBUnicode::PathToUTF8(std::filesystem::canonical(MBLSP::URLDecodePath(URI)));
    }
    void LSPHandler::p_RunOpenHandlers(DocumentInfo& Document,std::string const& URI,std::string const& Content)
    {
        auto HandlersCopy = m_OnOpenHandlers;
        //run handlers
        try
        {
            for(auto const& Handler : HandlersCopy)
            {
                m_Evaluator->Eval(*m_ExecutionState,Handler,{m_This,URI,Content});
            }
        }
        catch(...)
        {
               
        }
        MBLSP::PublishDiagnostics_Notification Notification;
        Notification.params.diagnostics = std::move(Document.Diagnostics);
        Notification.params.uri = URI;
        m_LSPServer->SendNotification(std::move(Notification));
    }
    //mainloop
    Value LSPHandler::HandleRequests BUILTIN_ARGLIST
    {
        Value ReturnValue;
        LSPHandler& Handler = Arguments[0].GetType<LSPHandler>();
        Handler.m_Evaluator = &Context.GetEvaluator();
        Handler.m_ExecutionState = &Context.GetState();
        Handler.m_This  = Arguments[0];

        MBUtility::StreamReader& Input = Context.GetState().GetCurrentScope().FindVariable(Context.GetEvaluator().GetSymbolID("*standard-input*")).GetType<MBUtility::StreamReader>();
        std::unique_ptr<MBUtility::MBOctetOutputStream>& Output = Context.GetState().GetCurrentScope().FindVariable(Context.GetEvaluator().GetSymbolID("*standard-output*")).
            GetType<std::unique_ptr<MBUtility::MBOctetOutputStream>>();

        MBLSP::LSP_ServerHandler Server  = MBLSP::LSP_ServerHandler(
                std::make_unique<MBUtility::NonOwningIndeterminateInputStream>(&Input),
                std::make_unique<MBUtility::NonOwningOutputStream>(Output.get()),
                std::make_unique<LSPServerAdapter>(&Handler)
                );
        Server.Run();
        return ReturnValue;
    }
}
