#pragma once

#include "../../Evaluator.h"
#include <MBLSP/SemanticTokens.h>

#include <MBLSP/MBLSP.h>
namespace MBLisp
{
    struct LSPServerAdapter : public MBLSP::LSP_Server
    {
        MBLSP::LSP_Server* m_InternalServer = nullptr;
    public:

        LSPServerAdapter(MBLSP::LSP_Server* ServerToAdapt)
        {
            m_InternalServer = ServerToAdapt;   
        }
        virtual MBLSP::Initialize_Response HandleRequest(MBLSP::InitializeRequest const& Request)
        {
            return m_InternalServer->HandleRequest(Request);   
        }
        virtual void OpenedDocument(std::string const& URI,std::string const& Content) 
        {
            m_InternalServer->OpenedDocument(URI,Content);
        }
        virtual void ClosedDocument(std::string const& URI)
        {
             m_InternalServer->ClosedDocument(URI);
        }
        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent)
        {
            m_InternalServer->DocumentChanged(URI,NewContent);
        }
        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent, std::vector<MBLSP::TextChange> const& Changes)
        {
            m_InternalServer->DocumentChanged(URI,NewContent,Changes);
        }

        virtual void SetHandler(MBLSP::LSP_ServerHandler* AssociatedHandler)
        {
            m_InternalServer->SetHandler(AssociatedHandler);
        }
        virtual MBLSP::GotoDefinition_Response HandleRequest(MBLSP::GotoDefinition_Request const& Request)
        {
            return m_InternalServer->HandleRequest(Request);
        }
        virtual MBLSP::SemanticToken_Response HandleRequest(MBLSP::SemanticToken_Request const& Request)
        {
            return m_InternalServer->HandleRequest(Request);
        }
        virtual MBLSP::SemanticTokensRange_Response HandleRequest(MBLSP::SemanticTokensRange_Request const& Request)
        {
            return m_InternalServer->HandleRequest(Request);
        }

        virtual MBParsing::JSONObject HandleGenericRequest(MBParsing::JSONObject const& GenericRequest)
        {
            return(m_InternalServer->HandleGenericRequest(GenericRequest));
        }
        virtual void HandleGenericNotification(MBParsing::JSONObject const& GenericNotification)
        {
            m_InternalServer->HandleGenericNotification(GenericNotification);
        }
    };

    
    struct LSPHandler : public MBLSP::LSP_Server
    {
        struct JumpSymbol
        {
            MBLSP::Position Source;
            int Length = 0;
            Symbol Target;
            bool operator<(JumpSymbol Rhs) const
            {
                return Source < Rhs.Source;
            }
        };
        struct DocumentInfo
        {
            MBLSP::LineIndex Lines;
            std::vector<MBLSP::SemanticToken> Tokens;
            std::vector<MBLSP::Diagnostic> Diagnostics;
            std::vector<JumpSymbol> Jumps;
        };
        std::unordered_map<std::string,DocumentInfo> m_OpenedDocuments;
        std::unordered_map<std::string,Value> m_RequestHandlers; 
        std::vector<Value> m_OnOpenHandlers;
        Evaluator* m_Evaluator = nullptr;
        MBLSP::LSP_ServerHandler* m_LSPServer = nullptr;
        Ref<Scope> m_Scope = nullptr;
        Value m_This;
        DocumentInfo p_CreateDocumentInfo(std::string const& Content);
        
        static MBLSP::TokenType StringToTokenType(std::string_view StringToConvert);


        void p_RunOpenHandlers(DocumentInfo& Document,std::string const& URI,std::string const& Content);

        static std::string p_CanonURI(std::string const& URI);
    public:

        virtual MBLSP::Initialize_Response HandleRequest(MBLSP::InitializeRequest const& Request);
        virtual void OpenedDocument(std::string const& URI,std::string const& Content);
        virtual void ClosedDocument(std::string const& URI);
        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent, std::vector<MBLSP::TextChange> const& Changes);

        virtual void DocumentChanged(std::string const& URI,std::string const& NewContent)
        {
               
        }
        virtual void SetHandler(MBLSP::LSP_ServerHandler* AssociatedHandler) 
        {
            m_LSPServer = AssociatedHandler;
        }
        virtual MBLSP::SemanticToken_Response HandleRequest(MBLSP::SemanticToken_Request const& Request);
        virtual MBLSP::SemanticTokensRange_Response HandleRequest(MBLSP::SemanticTokensRange_Request const& Request);
        virtual MBLSP::GotoDefinition_Response HandleRequest(MBLSP::GotoDefinition_Request const& Request);

        //no arguments
        static Value CreateLSPHandler BUILTIN_ARGLIST;
        //takes string, method, and callable -> string
        static Value AddGenericRequestHandler BUILTIN_ARGLIST;
        //document string, list of (symbol type) pairs
        static Value SetDocumentTokens BUILTIN_ARGLIST;
        //document string, list of (symbol type) pairs
        static Value SetDocumentDiagnostics BUILTIN_ARGLIST;
        //list of symbol symbol pairs
        static Value SetDocumentJumps BUILTIN_ARGLIST;
        //document string, list of (symbol type) pairs
        static Value AddOnOpenHandler BUILTIN_ARGLIST;
        //mainloop
        static Value HandleRequests BUILTIN_ARGLIST;
    };
    class LSPModule : public Module
    {
        Ref<Scope> m_ModuleScope = MakeRef<Scope>();
        Evaluator* m_Evaluator = nullptr;

    public:
        void SetEvaluator(Evaluator* AssociatedEvaluator) override
        {
            m_Evaluator = AssociatedEvaluator;
            m_ModuleScope->SetVariable(m_Evaluator->GetSymbolID("create-lsp-server"),LSPHandler::CreateLSPHandler);
            m_Evaluator->AddMethod<LSPHandler,Any>(m_ModuleScope,"add-generic-request-handler",LSPHandler::AddGenericRequestHandler);
            m_Evaluator->AddMethod<LSPHandler,Any>(m_ModuleScope,"add-on-open-handler",LSPHandler::AddOnOpenHandler);
            m_Evaluator->AddMethod<LSPHandler,String,List>(m_ModuleScope,"set-document-tokens",LSPHandler::SetDocumentTokens);
            m_Evaluator->AddMethod<LSPHandler,String,List>(m_ModuleScope,"set-document-diagnostics",LSPHandler::SetDocumentDiagnostics);
            m_Evaluator->AddMethod<LSPHandler,String,List>(m_ModuleScope,"set-document-jumps",LSPHandler::SetDocumentJumps);
            m_Evaluator->AddMethod<LSPHandler>(m_ModuleScope,"handle-requests",LSPHandler::HandleRequests);
        }
        Ref<Scope> GetModuleScope() override
        {
            return m_ModuleScope;   
        }
    };
}
