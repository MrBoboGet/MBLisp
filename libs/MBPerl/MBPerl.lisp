(import text)
(defclass Idf()
    (Position null)
    (Value null)
    
)
(defclass Expr()
    
)
(defclass Expr_Idf(Expr)
    (Identifier (Idf))
    
)
(defclass Expr_Num(Expr)
    (Value null)
    (Position null)
    
)
(defclass Expr_Bool(Expr)
    (Value null)
    
)
(defclass Expr_List(Expr)
    (Begin null)
    (Values (list))
    
)
(defclass DictEntry()
    (Key (Expr))
    (Value (Expr))
    
)
(defclass Expr_Dict(Expr)
    (Begin null)
    (Values (list))
    
)
(defclass Expr_FuncCall(Expr)
    (Identifier (Idf))
    (Arguments (list))
    
)
(defclass OperatorPart()
    (Operator (Op))
    (Rhs (Expr))
    
)
(defclass Op()
    (Value null)
    (Assignment  false)
    (ExtraArgs (list))
    
)
(defclass Expr_Operators(Expr)
    (Lhs (Expr))
    (Parts (list))
    
)
(defclass Statement()
    
)
(defclass IfContent()
    (IfPart (Idf))
    (Condition (Expr))
    (HasCondition  true)
    (Body (list))
    
)
(defclass Statement_If(Statement)
    (If (IfContent))
    (ElseClauses (list))
    
)
(defclass Statement_Return(Statement)
    (ReturnPart (Idf))
    (Value (Expr))
    
)
(defclass Statement_While(Statement)
    (WhilePart (Idf))
    (Condition (Expr))
    (Body (list))
    
)
(defclass Statement_For(Statement)
    (ForPart (Idf))
    (Enumerable (Expr))
    (VarName (Idf))
    (Body (list))
    
)
(defclass Statement_Expr(Statement)
    (Expr (Expr))
    
)
(defclass Argument()
    (Type (Idf))
    (Name (Idf))
    
)
(defclass Statement_Func(Statement)
    (FuncPart (Idf))
    (Name (Idf))
    (Args (list))
    (Content (list))
    
)
(defclass MemberVariable()
    (Name (Idf))
    (Value (Expr))
    
)
(defclass MemberFunc()
    (Name (Idf))
    (Args (list))
    (Body (list))
    
)
(defclass Statement_Class(Statement)
    (ClassPart (Idf))
    (Name (Idf))
    (Parents (list))
    (Variables (list))
    (Methods (list))
    
)
(defclass StatementList()
    (Statements (list))
    
)
(defclass Expr_Regex(Expr)
    (Begin null)
    (Content null)
    
)
(defclass Expr_String(Expr)
    (Begin null)
    (Content null)
    
)
(defclass Expr_Lambda(Expr)
    (Variables (list))
    (Body (list))
    
)
(defun get-tokenizer ()
    (tokenizer "\\s+"(list "true" "false" "class" "func" "return" ":" "\\{" "\\}" "\\[" "\\]" "for" "if" "in\\s" "while" "else" ";" "\\d+" "\"([^\"\\\\]|\\\\.)*\"" "(s|d|e|m)/(([^\\n/\\\\]|\\\\.)*/)+\\w*" "_" "\\w+|\\$_" "\\." "\\+" "\\*" "/" "\\\\\\" "=~" "<" "<=" ">" ">=" "==" "=" "\\(" "\\)" ","))
)
(defun FillIdf_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 20))
        (error (+ "Error parsing Idf at position "  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseIdf_0 (tokenizer)
    (setl ReturnValue (Idf))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 20))
        (error (+ "Error parsing Idf at position "  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillIdf (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 20))
        (error (+ "Error parsing Idf at position "  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseIdf (tokenizer)
    (setl ReturnValue (Idf))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 20))
        (error (+ "Error parsing Idf at position "  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Idf_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun ParseExpr_Idf_0 (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Idf (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun ParseExpr_Idf (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Num_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 16))
        (error (+ "Error parsing Expr_Num at position "  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Num_0 (tokenizer)
    (setl ReturnValue (Expr_Num))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 16))
        (error (+ "Error parsing Expr_Num at position "  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Num (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 16))
        (error (+ "Error parsing Expr_Num at position "  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Num (tokenizer)
    (setl ReturnValue (Expr_Num))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 16))
        (error (+ "Error parsing Expr_Num at position "  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Bool_0 (ReturnValue tokenizer)
    (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) true)
        (if (not (eq :type (peek tokenizer 0) 0))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected T"))
            
        )
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) false)
        (if (not (eq :type (peek tokenizer 0) 1))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected F"))
            
        )
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Expr_Bool at position "  (str :position (peek tokenizer 0)) ": expected _L1"))
    

)
)
(defun ParseExpr_Bool_0 (tokenizer)
    (setl ReturnValue (Expr_Bool))
    (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) true)
        (if (not (eq :type (peek tokenizer 0) 0))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected T"))
            
        )
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) false)
        (if (not (eq :type (peek tokenizer 0) 1))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected F"))
            
        )
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Expr_Bool at position "  (str :position (peek tokenizer 0)) ": expected _L1"))
    

)(return ReturnValue)

)
(defun FillExpr_Bool (ReturnValue tokenizer)
    (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) true)
        (if (not (eq :type (peek tokenizer 0) 0))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected T"))
            
        )
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) false)
        (if (not (eq :type (peek tokenizer 0) 1))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected F"))
            
        )
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Expr_Bool at position "  (str :position (peek tokenizer 0)) ": expected _L1"))
    

)
)
(defun ParseExpr_Bool (tokenizer)
    (setl ReturnValue (Expr_Bool))
    (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) true)
        (if (not (eq :type (peek tokenizer 0) 0))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected T"))
            
        )
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (setl (. ReturnValue 'Value) false)
        (if (not (eq :type (peek tokenizer 0) 1))
            (error (+ "Error parsing _L1 at position "  (str :position (peek tokenizer 0)) ": expected F"))
            
        )
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Expr_Bool at position "  (str :position (peek tokenizer 0)) ": expected _L1"))
    

)(return ReturnValue)

)
(defun FillOp_0 (ReturnValue tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 21))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected dot"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 26))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected comp"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 22))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected plus"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 23))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected times"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 31))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eqq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 27))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected le"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 28))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected leq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 29))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected ge"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 23 0 :type (peek tokenizer 0))(. LOOKTable 23 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 30))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected geq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 24))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected div"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L2"))
    

)(if (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 26 0 :type (peek tokenizer 0))(. LOOKTable 26 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L3 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (consume-token tokenizer)
        (setl (. ReturnValue 'Assignment) true)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L3"))
    

)
)

)
(defun ParseOp_0 (tokenizer)
    (setl ReturnValue (Op))
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 21))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected dot"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 26))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected comp"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 22))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected plus"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 23))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected times"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 31))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eqq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 27))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected le"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 28))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected leq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 29))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected ge"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 23 0 :type (peek tokenizer 0))(. LOOKTable 23 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 30))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected geq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 24))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected div"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L2"))
    

)(if (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 26 0 :type (peek tokenizer 0))(. LOOKTable 26 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L3 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (consume-token tokenizer)
        (setl (. ReturnValue 'Assignment) true)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L3"))
    

)
)
(return ReturnValue)

)
(defun FillOp (ReturnValue tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 21))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected dot"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 26))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected comp"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 22))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected plus"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 23))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected times"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 31))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eqq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 27))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected le"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 28))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected leq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 29))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected ge"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 23 0 :type (peek tokenizer 0))(. LOOKTable 23 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 30))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected geq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 24))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected div"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L2"))
    

)(if (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 26 0 :type (peek tokenizer 0))(. LOOKTable 26 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L3 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (consume-token tokenizer)
        (setl (. ReturnValue 'Assignment) true)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L3"))
    

)
)

)
(defun ParseOp (tokenizer)
    (setl ReturnValue (Op))
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 21))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected dot"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 26))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected comp"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 22))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected plus"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 23))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected times"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 31))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected eqq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 27))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected le"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 28))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected leq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 29))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected ge"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 23 0 :type (peek tokenizer 0))(. LOOKTable 23 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 30))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected geq"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 24))
            (error (+ "Error parsing _L2 at position "  (str :position (peek tokenizer 0)) ": expected div"))
            
        )
        (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
        (consume-token tokenizer)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L2"))
    

)(if (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 26 0 :type (peek tokenizer 0))(. LOOKTable 26 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 32))
            (error (+ "Error parsing _L3 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
            
        )
        (consume-token tokenizer)
        (setl (. ReturnValue 'Assignment) true)
        
    
    else (error (+ "Error parsing Op at position "  (str :position (peek tokenizer 0)) ": expected _L3"))
    

)
)
(return ReturnValue)

)
(defun FillPostOp_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 31 0 :type (peek tokenizer 0))(. LOOKTable 31 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 32 0 :type (peek tokenizer 0))(. LOOKTable 32 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'ExtraArgs) (Expr))
            (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 29 0 :type (peek tokenizer 0))(. LOOKTable 29 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 30 0 :type (peek tokenizer 0))(. LOOKTable 30 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'ExtraArgs) (Expr))
                    (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected _L5"))
                
            
            )
    )
    

else (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected _L4"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)

)
(defun ParsePostOp_0 (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 31 0 :type (peek tokenizer 0))(. LOOKTable 31 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 32 0 :type (peek tokenizer 0))(. LOOKTable 32 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'ExtraArgs) (Expr))
            (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 29 0 :type (peek tokenizer 0))(. LOOKTable 29 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 30 0 :type (peek tokenizer 0))(. LOOKTable 30 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'ExtraArgs) (Expr))
                    (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected _L5"))
                
            
            )
    )
    

else (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected _L4"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillPostOp (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 31 0 :type (peek tokenizer 0))(. LOOKTable 31 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 32 0 :type (peek tokenizer 0))(. LOOKTable 32 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'ExtraArgs) (Expr))
            (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 29 0 :type (peek tokenizer 0))(. LOOKTable 29 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 30 0 :type (peek tokenizer 0))(. LOOKTable 30 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'ExtraArgs) (Expr))
                    (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected _L5"))
                
            
            )
    )
    

else (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected _L4"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)

)
(defun ParsePostOp (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 31 0 :type (peek tokenizer 0))(. LOOKTable 31 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 32 0 :type (peek tokenizer 0))(. LOOKTable 32 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'ExtraArgs) (Expr))
            (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 29 0 :type (peek tokenizer 0))(. LOOKTable 29 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 30 0 :type (peek tokenizer 0))(. LOOKTable 30 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L5 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'ExtraArgs) (Expr))
                    (set (back (. ReturnValue 'ExtraArgs))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L4 at position "  (str :position (peek tokenizer 0)) ": expected _L5"))
                
            
            )
    )
    

else (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected _L4"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing PostOp at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_FuncCall_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 37 0 :type (peek tokenizer 0))(. LOOKTable 37 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 38 0 :type (peek tokenizer 0))(. LOOKTable 38 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 35 0 :type (peek tokenizer 0))(. LOOKTable 35 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 36 0 :type (peek tokenizer 0))(. LOOKTable 36 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected _L7"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected _L6"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_FuncCall_0 (tokenizer)
    (setl ReturnValue (Expr_FuncCall))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 37 0 :type (peek tokenizer 0))(. LOOKTable 37 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 38 0 :type (peek tokenizer 0))(. LOOKTable 38 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 35 0 :type (peek tokenizer 0))(. LOOKTable 35 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 36 0 :type (peek tokenizer 0))(. LOOKTable 36 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected _L7"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected _L6"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_FuncCall (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 37 0 :type (peek tokenizer 0))(. LOOKTable 37 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 38 0 :type (peek tokenizer 0))(. LOOKTable 38 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 35 0 :type (peek tokenizer 0))(. LOOKTable 35 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 36 0 :type (peek tokenizer 0))(. LOOKTable 36 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected _L7"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected _L6"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_FuncCall (tokenizer)
    (setl ReturnValue (Expr_FuncCall))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 37 0 :type (peek tokenizer 0))(. LOOKTable 37 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 38 0 :type (peek tokenizer 0))(. LOOKTable 38 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 35 0 :type (peek tokenizer 0))(. LOOKTable 35 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 36 0 :type (peek tokenizer 0))(. LOOKTable 36 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L7 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L6 at position "  (str :position (peek tokenizer 0)) ": expected _L7"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected _L6"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Expr_FuncCall at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Regex_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 18))
        (error (+ "Error parsing Expr_Regex at position "  (str :position (peek tokenizer 0)) ": expected regex"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Regex_0 (tokenizer)
    (setl ReturnValue (Expr_Regex))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 18))
        (error (+ "Error parsing Expr_Regex at position "  (str :position (peek tokenizer 0)) ": expected regex"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Regex (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 18))
        (error (+ "Error parsing Expr_Regex at position "  (str :position (peek tokenizer 0)) ": expected regex"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Regex (tokenizer)
    (setl ReturnValue (Expr_Regex))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 18))
        (error (+ "Error parsing Expr_Regex at position "  (str :position (peek tokenizer 0)) ": expected regex"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_String_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 17))
        (error (+ "Error parsing Expr_String at position "  (str :position (peek tokenizer 0)) ": expected str"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_String_0 (tokenizer)
    (setl ReturnValue (Expr_String))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 17))
        (error (+ "Error parsing Expr_String at position "  (str :position (peek tokenizer 0)) ": expected str"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_String (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 17))
        (error (+ "Error parsing Expr_String at position "  (str :position (peek tokenizer 0)) ": expected str"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_String (tokenizer)
    (setl ReturnValue (Expr_String))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 17))
        (error (+ "Error parsing Expr_String at position "  (str :position (peek tokenizer 0)) ": expected str"))
        
    )
    (setl (. ReturnValue 'Content) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Lambda_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 25))
        (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected backslash"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 47 0 :type (peek tokenizer 0))(. LOOKTable 47 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 48 0 :type (peek tokenizer 0))(. LOOKTable 48 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Variables) (Idf))
            (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
            (while (&& (. LOOKTable 45 0 :type (peek tokenizer 0))(. LOOKTable 45 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 46 0 :type (peek tokenizer 0))(. LOOKTable 46 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Variables) (Idf))
                    (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
                    
                
                else (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected _L9"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected _L8"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected colon"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_Lambda_0 (tokenizer)
    (setl ReturnValue (Expr_Lambda))
    (if (not (eq :type (peek tokenizer 0) 25))
        (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected backslash"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 47 0 :type (peek tokenizer 0))(. LOOKTable 47 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 48 0 :type (peek tokenizer 0))(. LOOKTable 48 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Variables) (Idf))
            (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
            (while (&& (. LOOKTable 45 0 :type (peek tokenizer 0))(. LOOKTable 45 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 46 0 :type (peek tokenizer 0))(. LOOKTable 46 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Variables) (Idf))
                    (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
                    
                
                else (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected _L9"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected _L8"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected colon"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Lambda (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 25))
        (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected backslash"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 47 0 :type (peek tokenizer 0))(. LOOKTable 47 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 48 0 :type (peek tokenizer 0))(. LOOKTable 48 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Variables) (Idf))
            (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
            (while (&& (. LOOKTable 45 0 :type (peek tokenizer 0))(. LOOKTable 45 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 46 0 :type (peek tokenizer 0))(. LOOKTable 46 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Variables) (Idf))
                    (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
                    
                
                else (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected _L9"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected _L8"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected colon"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_Lambda (tokenizer)
    (setl ReturnValue (Expr_Lambda))
    (if (not (eq :type (peek tokenizer 0) 25))
        (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected backslash"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 47 0 :type (peek tokenizer 0))(. LOOKTable 47 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 48 0 :type (peek tokenizer 0))(. LOOKTable 48 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Variables) (Idf))
            (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
            (while (&& (. LOOKTable 45 0 :type (peek tokenizer 0))(. LOOKTable 45 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 46 0 :type (peek tokenizer 0))(. LOOKTable 46 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L9 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Variables) (Idf))
                    (FillIdf (back (. ReturnValue 'Variables)) tokenizer)
                    
                
                else (error (+ "Error parsing _L8 at position "  (str :position (peek tokenizer 0)) ": expected _L9"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected _L8"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected colon"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Lambda at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Lambda_Short_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (append (. ReturnValue 'Variables) (. (ParseExpr_Underscore tokenizer) 'Identifier))
    (if (not (&& (. LOOKTable 51 0 :type (peek tokenizer 0))(. LOOKTable 51 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
        
    )
    (append (. ReturnValue 'Body) (Statement_Expr))
    (set (back (. ReturnValue 'Body))(Parse_L10 tokenizer))
    
)
(defun ParseExpr_Lambda_Short_0 (tokenizer)
    (setl ReturnValue (Expr_Lambda))
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (append (. ReturnValue 'Variables) (. (ParseExpr_Underscore tokenizer) 'Identifier))
    (if (not (&& (. LOOKTable 51 0 :type (peek tokenizer 0))(. LOOKTable 51 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
        
    )
    (append (. ReturnValue 'Body) (Statement_Expr))
    (set (back (. ReturnValue 'Body))(Parse_L10 tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Lambda_Short (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (append (. ReturnValue 'Variables) (. (ParseExpr_Underscore tokenizer) 'Identifier))
    (if (not (&& (. LOOKTable 51 0 :type (peek tokenizer 0))(. LOOKTable 51 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
        
    )
    (append (. ReturnValue 'Body) (Statement_Expr))
    (set (back (. ReturnValue 'Body))(Parse_L10 tokenizer))
    
)
(defun ParseExpr_Lambda_Short (tokenizer)
    (setl ReturnValue (Expr_Lambda))
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (append (. ReturnValue 'Variables) (. (ParseExpr_Underscore tokenizer) 'Identifier))
    (if (not (&& (. LOOKTable 51 0 :type (peek tokenizer 0))(. LOOKTable 51 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Lambda_Short at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
        
    )
    (append (. ReturnValue 'Body) (Statement_Expr))
    (set (back (. ReturnValue 'Body))(Parse_L10 tokenizer))
    (return ReturnValue)
    
)
(defun Fill_L10_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 33 0 :type (peek tokenizer 0))(. LOOKTable 33 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr_FuncCall tokenizer))
    
)
(defun Parse_L10_0 (tokenizer)
    (setl ReturnValue (Statement_Expr))
    (if (not (&& (. LOOKTable 33 0 :type (peek tokenizer 0))(. LOOKTable 33 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr_FuncCall tokenizer))
    (return ReturnValue)
    
)
(defun Fill_L10_1 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 34))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    
)
(defun Parse_L10_1 (tokenizer)
    (setl ReturnValue (Statement_Expr))
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 34))
        (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun Fill_L10 (ReturnValue tokenizer)
    (if (&& (. LOOKTable 52 0 :type (peek tokenizer 0))(. LOOKTable 52 1 :type (peek tokenizer 1)))
        (Fill_L10_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 53 0 :type (peek tokenizer 0))(. LOOKTable 53 1 :type (peek tokenizer 1)))
        (Fill_L10_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
    

)
)
(defun Parse_L10 (tokenizer)
    (setl ReturnValue (Statement_Expr))
    (if (&& (. LOOKTable 52 0 :type (peek tokenizer 0))(. LOOKTable 52 1 :type (peek tokenizer 1)))
        (Fill_L10_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 53 0 :type (peek tokenizer 0))(. LOOKTable 53 1 :type (peek tokenizer 1)))
        (Fill_L10_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing _L10 at position "  (str :position (peek tokenizer 0)) ": expected _L10"))
    

)(return ReturnValue)

)
(defun FillExpr_Underscore_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Identifier 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 19))
        (error (+ "Error parsing Expr_Underscore at position "  (str :position (peek tokenizer 0)) ": expected underscore"))
        
    )
    (setl (. ReturnValue 'Identifier 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Underscore_0 (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (setl (. ReturnValue 'Identifier 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 19))
        (error (+ "Error parsing Expr_Underscore at position "  (str :position (peek tokenizer 0)) ": expected underscore"))
        
    )
    (setl (. ReturnValue 'Identifier 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Underscore (ReturnValue tokenizer)
    (setl (. ReturnValue 'Identifier 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 19))
        (error (+ "Error parsing Expr_Underscore at position "  (str :position (peek tokenizer 0)) ": expected underscore"))
        
    )
    (setl (. ReturnValue 'Identifier 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Underscore (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (setl (. ReturnValue 'Identifier 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 19))
        (error (+ "Error parsing Expr_Underscore at position "  (str :position (peek tokenizer 0)) ": expected underscore"))
        
    )
    (setl (. ReturnValue 'Identifier 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillDictEntry_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Key)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected colon"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    
)
(defun ParseDictEntry_0 (tokenizer)
    (setl ReturnValue (DictEntry))
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Key)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected colon"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (return ReturnValue)
    
)
(defun FillDictEntry (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Key)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected colon"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    
)
(defun ParseDictEntry (tokenizer)
    (setl ReturnValue (DictEntry))
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Key)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected colon"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing DictEntry at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_List_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 62 0 :type (peek tokenizer 0))(. LOOKTable 62 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 63 0 :type (peek tokenizer 0))(. LOOKTable 63 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Values) (Expr))
            (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 60 0 :type (peek tokenizer 0))(. LOOKTable 60 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 61 0 :type (peek tokenizer 0))(. LOOKTable 61 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Values) (Expr))
                    (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected _L12"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected _L11"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_List_0 (tokenizer)
    (setl ReturnValue (Expr_List))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 62 0 :type (peek tokenizer 0))(. LOOKTable 62 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 63 0 :type (peek tokenizer 0))(. LOOKTable 63 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Values) (Expr))
            (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 60 0 :type (peek tokenizer 0))(. LOOKTable 60 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 61 0 :type (peek tokenizer 0))(. LOOKTable 61 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Values) (Expr))
                    (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected _L12"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected _L11"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_List (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 62 0 :type (peek tokenizer 0))(. LOOKTable 62 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 63 0 :type (peek tokenizer 0))(. LOOKTable 63 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Values) (Expr))
            (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 60 0 :type (peek tokenizer 0))(. LOOKTable 60 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 61 0 :type (peek tokenizer 0))(. LOOKTable 61 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Values) (Expr))
                    (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected _L12"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected _L11"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_List (tokenizer)
    (setl ReturnValue (Expr_List))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 8))
        (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected lsquare"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 62 0 :type (peek tokenizer 0))(. LOOKTable 62 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 63 0 :type (peek tokenizer 0))(. LOOKTable 63 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Values) (Expr))
            (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 60 0 :type (peek tokenizer 0))(. LOOKTable 60 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 61 0 :type (peek tokenizer 0))(. LOOKTable 61 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L12 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Values) (Expr))
                    (set (back (. ReturnValue 'Values))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L11 at position "  (str :position (peek tokenizer 0)) ": expected _L12"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected _L11"))


)
)
(if (not (eq :type (peek tokenizer 0) 9))
    (error (+ "Error parsing Expr_List at position "  (str :position (peek tokenizer 0)) ": expected rsquare"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Dict_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 68 0 :type (peek tokenizer 0))(. LOOKTable 68 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 69 0 :type (peek tokenizer 0))(. LOOKTable 69 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                
            )
            (append (. ReturnValue 'Values) (DictEntry))
            (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
            (while (&& (. LOOKTable 66 0 :type (peek tokenizer 0))(. LOOKTable 66 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 67 0 :type (peek tokenizer 0))(. LOOKTable 67 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                        
                    )
                    (append (. ReturnValue 'Values) (DictEntry))
                    (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
                    
                
                else (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected _L14"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected _L13"))


)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_Dict_0 (tokenizer)
    (setl ReturnValue (Expr_Dict))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 68 0 :type (peek tokenizer 0))(. LOOKTable 68 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 69 0 :type (peek tokenizer 0))(. LOOKTable 69 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                
            )
            (append (. ReturnValue 'Values) (DictEntry))
            (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
            (while (&& (. LOOKTable 66 0 :type (peek tokenizer 0))(. LOOKTable 66 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 67 0 :type (peek tokenizer 0))(. LOOKTable 67 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                        
                    )
                    (append (. ReturnValue 'Values) (DictEntry))
                    (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
                    
                
                else (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected _L14"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected _L13"))


)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Dict (ReturnValue tokenizer)
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 68 0 :type (peek tokenizer 0))(. LOOKTable 68 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 69 0 :type (peek tokenizer 0))(. LOOKTable 69 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                
            )
            (append (. ReturnValue 'Values) (DictEntry))
            (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
            (while (&& (. LOOKTable 66 0 :type (peek tokenizer 0))(. LOOKTable 66 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 67 0 :type (peek tokenizer 0))(. LOOKTable 67 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                        
                    )
                    (append (. ReturnValue 'Values) (DictEntry))
                    (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
                    
                
                else (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected _L14"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected _L13"))


)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_Dict (tokenizer)
    (setl ReturnValue (Expr_Dict))
    (setl (. ReturnValue 'Begin) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 68 0 :type (peek tokenizer 0))(. LOOKTable 68 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 69 0 :type (peek tokenizer 0))(. LOOKTable 69 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                
            )
            (append (. ReturnValue 'Values) (DictEntry))
            (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
            (while (&& (. LOOKTable 66 0 :type (peek tokenizer 0))(. LOOKTable 66 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 67 0 :type (peek tokenizer 0))(. LOOKTable 67 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 56 0 :type (peek tokenizer 0))(. LOOKTable 56 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L14 at position "  (str :position (peek tokenizer 0)) ": expected DictEntry"))
                        
                    )
                    (append (. ReturnValue 'Values) (DictEntry))
                    (FillDictEntry (back (. ReturnValue 'Values)) tokenizer)
                    
                
                else (error (+ "Error parsing _L13 at position "  (str :position (peek tokenizer 0)) ": expected _L14"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected _L13"))


)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Expr_Dict at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Term_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set ReturnValue(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 34))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseExpr_Term_0 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set ReturnValue(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 34))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Term_1 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Idf"))
        
    )
    (set ReturnValue(ParseExpr_Idf tokenizer))
    
)
(defun ParseExpr_Term_1 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Idf"))
        
    )
    (set ReturnValue(ParseExpr_Idf tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_2 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 33 0 :type (peek tokenizer 0))(. LOOKTable 33 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set ReturnValue(ParseExpr_FuncCall tokenizer))
    
)
(defun ParseExpr_Term_2 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 33 0 :type (peek tokenizer 0))(. LOOKTable 33 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set ReturnValue(ParseExpr_FuncCall tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_3 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Num"))
        
    )
    (set ReturnValue(ParseExpr_Num tokenizer))
    
)
(defun ParseExpr_Term_3 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Num"))
        
    )
    (set ReturnValue(ParseExpr_Num tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_4 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Bool"))
        
    )
    (set ReturnValue(ParseExpr_Bool tokenizer))
    
)
(defun ParseExpr_Term_4 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Bool"))
        
    )
    (set ReturnValue(ParseExpr_Bool tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_5 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 39 0 :type (peek tokenizer 0))(. LOOKTable 39 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Regex"))
        
    )
    (set ReturnValue(ParseExpr_Regex tokenizer))
    
)
(defun ParseExpr_Term_5 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 39 0 :type (peek tokenizer 0))(. LOOKTable 39 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Regex"))
        
    )
    (set ReturnValue(ParseExpr_Regex tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_6 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 41 0 :type (peek tokenizer 0))(. LOOKTable 41 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_String"))
        
    )
    (set ReturnValue(ParseExpr_String tokenizer))
    
)
(defun ParseExpr_Term_6 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 41 0 :type (peek tokenizer 0))(. LOOKTable 41 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_String"))
        
    )
    (set ReturnValue(ParseExpr_String tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_7 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 58 0 :type (peek tokenizer 0))(. LOOKTable 58 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_List"))
        
    )
    (set ReturnValue(ParseExpr_List tokenizer))
    
)
(defun ParseExpr_Term_7 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 58 0 :type (peek tokenizer 0))(. LOOKTable 58 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_List"))
        
    )
    (set ReturnValue(ParseExpr_List tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_8 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 64 0 :type (peek tokenizer 0))(. LOOKTable 64 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Dict"))
        
    )
    (set ReturnValue(ParseExpr_Dict tokenizer))
    
)
(defun ParseExpr_Term_8 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 64 0 :type (peek tokenizer 0))(. LOOKTable 64 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Dict"))
        
    )
    (set ReturnValue(ParseExpr_Dict tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_9 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 43 0 :type (peek tokenizer 0))(. LOOKTable 43 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Lambda"))
        
    )
    (set ReturnValue(ParseExpr_Lambda tokenizer))
    
)
(defun ParseExpr_Term_9 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 43 0 :type (peek tokenizer 0))(. LOOKTable 43 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Lambda"))
        
    )
    (set ReturnValue(ParseExpr_Lambda tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_10 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 49 0 :type (peek tokenizer 0))(. LOOKTable 49 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Lambda_Short"))
        
    )
    (set ReturnValue(ParseExpr_Lambda_Short tokenizer))
    
)
(defun ParseExpr_Term_10 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 49 0 :type (peek tokenizer 0))(. LOOKTable 49 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Lambda_Short"))
        
    )
    (set ReturnValue(ParseExpr_Lambda_Short tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_11 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (set ReturnValue(ParseExpr_Underscore tokenizer))
    
)
(defun ParseExpr_Term_11 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 54 0 :type (peek tokenizer 0))(. LOOKTable 54 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Underscore"))
        
    )
    (set ReturnValue(ParseExpr_Underscore tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term (ReturnValue tokenizer)
    (if (&& (. LOOKTable 71 0 :type (peek tokenizer 0))(. LOOKTable 71 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_0 tokenizer))
        
    
    else if (&& (. LOOKTable 72 0 :type (peek tokenizer 0))(. LOOKTable 72 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_1 tokenizer))
        
    
    else if (&& (. LOOKTable 73 0 :type (peek tokenizer 0))(. LOOKTable 73 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_2 tokenizer))
        
    
    else if (&& (. LOOKTable 74 0 :type (peek tokenizer 0))(. LOOKTable 74 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_3 tokenizer))
        
    
    else if (&& (. LOOKTable 75 0 :type (peek tokenizer 0))(. LOOKTable 75 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_4 tokenizer))
        
    
    else if (&& (. LOOKTable 76 0 :type (peek tokenizer 0))(. LOOKTable 76 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_5 tokenizer))
        
    
    else if (&& (. LOOKTable 77 0 :type (peek tokenizer 0))(. LOOKTable 77 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_6 tokenizer))
        
    
    else if (&& (. LOOKTable 78 0 :type (peek tokenizer 0))(. LOOKTable 78 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_7 tokenizer))
        
    
    else if (&& (. LOOKTable 79 0 :type (peek tokenizer 0))(. LOOKTable 79 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_8 tokenizer))
        
    
    else if (&& (. LOOKTable 80 0 :type (peek tokenizer 0))(. LOOKTable 80 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_9 tokenizer))
        
    
    else if (&& (. LOOKTable 81 0 :type (peek tokenizer 0))(. LOOKTable 81 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_10 tokenizer))
        
    
    else if (&& (. LOOKTable 82 0 :type (peek tokenizer 0))(. LOOKTable 82 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_11 tokenizer))
        
    
    else (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
    

)
)
(defun ParseExpr_Term (tokenizer)
    (setl ReturnValue (Expr))
    (if (&& (. LOOKTable 71 0 :type (peek tokenizer 0))(. LOOKTable 71 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_0 tokenizer))
        
    
    else if (&& (. LOOKTable 72 0 :type (peek tokenizer 0))(. LOOKTable 72 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_1 tokenizer))
        
    
    else if (&& (. LOOKTable 73 0 :type (peek tokenizer 0))(. LOOKTable 73 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_2 tokenizer))
        
    
    else if (&& (. LOOKTable 74 0 :type (peek tokenizer 0))(. LOOKTable 74 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_3 tokenizer))
        
    
    else if (&& (. LOOKTable 75 0 :type (peek tokenizer 0))(. LOOKTable 75 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_4 tokenizer))
        
    
    else if (&& (. LOOKTable 76 0 :type (peek tokenizer 0))(. LOOKTable 76 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_5 tokenizer))
        
    
    else if (&& (. LOOKTable 77 0 :type (peek tokenizer 0))(. LOOKTable 77 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_6 tokenizer))
        
    
    else if (&& (. LOOKTable 78 0 :type (peek tokenizer 0))(. LOOKTable 78 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_7 tokenizer))
        
    
    else if (&& (. LOOKTable 79 0 :type (peek tokenizer 0))(. LOOKTable 79 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_8 tokenizer))
        
    
    else if (&& (. LOOKTable 80 0 :type (peek tokenizer 0))(. LOOKTable 80 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_9 tokenizer))
        
    
    else if (&& (. LOOKTable 81 0 :type (peek tokenizer 0))(. LOOKTable 81 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_10 tokenizer))
        
    
    else if (&& (. LOOKTable 82 0 :type (peek tokenizer 0))(. LOOKTable 82 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_11 tokenizer))
        
    
    else (error (+ "Error parsing Expr_Term at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
    

)(return ReturnValue)

)
(defun FillExpr_Operators_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 87 0 :type (peek tokenizer 0))(. LOOKTable 87 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L15 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    
)
(defun ParseExpr_Operators_0 (tokenizer)
    (setl ReturnValue (Expr_Operators))
    (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 87 0 :type (peek tokenizer 0))(. LOOKTable 87 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L15 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    (return ReturnValue)
    
)
(defun FillExpr_Operators (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 87 0 :type (peek tokenizer 0))(. LOOKTable 87 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L15 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    
)
(defun ParseExpr_Operators (tokenizer)
    (setl ReturnValue (Expr_Operators))
    (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 87 0 :type (peek tokenizer 0))(. LOOKTable 87 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L15 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    (return ReturnValue)
    
)
(defun Fill_L15_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 11 0 :type (peek tokenizer 0))(. LOOKTable 11 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (FillOp (. ReturnValue 'Operator) tokenizer)
    (if (&& (. LOOKTable 86 0 :type (peek tokenizer 0))(. LOOKTable 86 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L16 at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected _L16"))
    

)
)
(defun Parse_L15_0 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (not (&& (. LOOKTable 11 0 :type (peek tokenizer 0))(. LOOKTable 11 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (FillOp (. ReturnValue 'Operator) tokenizer)
    (if (&& (. LOOKTable 86 0 :type (peek tokenizer 0))(. LOOKTable 86 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 70 0 :type (peek tokenizer 0))(. LOOKTable 70 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L16 at position "  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected _L16"))
    

)(return ReturnValue)

)
(defun Fill_L15_1 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected PostOp"))
        
    )
    (FillPostOp (. ReturnValue 'Operator) tokenizer)
    
)
(defun Parse_L15_1 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected PostOp"))
        
    )
    (FillPostOp (. ReturnValue 'Operator) tokenizer)
    (return ReturnValue)
    
)
(defun Fill_L15 (ReturnValue tokenizer)
    (if (&& (. LOOKTable 88 0 :type (peek tokenizer 0))(. LOOKTable 88 1 :type (peek tokenizer 1)))
        (Fill_L15_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 89 0 :type (peek tokenizer 0))(. LOOKTable 89 1 :type (peek tokenizer 1)))
        (Fill_L15_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected _L15"))
    

)
)
(defun Parse_L15 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (&& (. LOOKTable 88 0 :type (peek tokenizer 0))(. LOOKTable 88 1 :type (peek tokenizer 1)))
        (Fill_L15_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 89 0 :type (peek tokenizer 0))(. LOOKTable 89 1 :type (peek tokenizer 1)))
        (Fill_L15_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing _L15 at position "  (str :position (peek tokenizer 0)) ": expected _L15"))
    

)(return ReturnValue)

)
(defun FillExpr_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 83 0 :type (peek tokenizer 0))(. LOOKTable 83 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun ParseExpr_0 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 83 0 :type (peek tokenizer 0))(. LOOKTable 83 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 83 0 :type (peek tokenizer 0))(. LOOKTable 83 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun ParseExpr (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 83 0 :type (peek tokenizer 0))(. LOOKTable 83 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_For_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'ForPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 10))
        (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected for"))
        
    )
    (setl (. ReturnValue 'ForPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 94 0 :type (peek tokenizer 0))(. LOOKTable 94 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 95 0 :type (peek tokenizer 0))(. LOOKTable 95 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (FillIdf (. ReturnValue 'VarName) tokenizer)
            (if (not (eq :type (peek tokenizer 0) 12))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected in"))
                
            )
            (consume-token tokenizer)
            
        
        else (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected _L17"))
        
    
    )
)
(if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
    
)
(set (. ReturnValue 'Enumerable)(ParseExpr tokenizer))
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_For_0 (tokenizer)
    (setl ReturnValue (Statement_For))
    (setl (. ReturnValue 'ForPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 10))
        (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected for"))
        
    )
    (setl (. ReturnValue 'ForPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 94 0 :type (peek tokenizer 0))(. LOOKTable 94 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 95 0 :type (peek tokenizer 0))(. LOOKTable 95 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (FillIdf (. ReturnValue 'VarName) tokenizer)
            (if (not (eq :type (peek tokenizer 0) 12))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected in"))
                
            )
            (consume-token tokenizer)
            
        
        else (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected _L17"))
        
    
    )
)
(if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
    
)
(set (. ReturnValue 'Enumerable)(ParseExpr tokenizer))
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillStatement_For (ReturnValue tokenizer)
    (setl (. ReturnValue 'ForPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 10))
        (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected for"))
        
    )
    (setl (. ReturnValue 'ForPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 94 0 :type (peek tokenizer 0))(. LOOKTable 94 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 95 0 :type (peek tokenizer 0))(. LOOKTable 95 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (FillIdf (. ReturnValue 'VarName) tokenizer)
            (if (not (eq :type (peek tokenizer 0) 12))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected in"))
                
            )
            (consume-token tokenizer)
            
        
        else (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected _L17"))
        
    
    )
)
(if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
    
)
(set (. ReturnValue 'Enumerable)(ParseExpr tokenizer))
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_For (tokenizer)
    (setl ReturnValue (Statement_For))
    (setl (. ReturnValue 'ForPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 10))
        (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected for"))
        
    )
    (setl (. ReturnValue 'ForPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (&& (. LOOKTable 94 0 :type (peek tokenizer 0))(. LOOKTable 94 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 95 0 :type (peek tokenizer 0))(. LOOKTable 95 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (FillIdf (. ReturnValue 'VarName) tokenizer)
            (if (not (eq :type (peek tokenizer 0) 12))
                (error (+ "Error parsing _L17 at position "  (str :position (peek tokenizer 0)) ": expected in"))
                
            )
            (consume-token tokenizer)
            
        
        else (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected _L17"))
        
    
    )
)
(if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
    
)
(set (. ReturnValue 'Enumerable)(ParseExpr tokenizer))
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_For at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillIfContent_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'IfPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 11))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected if"))
        
    )
    (setl (. ReturnValue 'IfPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseIfContent_0 (tokenizer)
    (setl ReturnValue (IfContent))
    (setl (. ReturnValue 'IfPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 11))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected if"))
        
    )
    (setl (. ReturnValue 'IfPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillIfContent (ReturnValue tokenizer)
    (setl (. ReturnValue 'IfPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 11))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected if"))
        
    )
    (setl (. ReturnValue 'IfPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseIfContent (tokenizer)
    (setl ReturnValue (IfContent))
    (setl (. ReturnValue 'IfPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 11))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected if"))
        
    )
    (setl (. ReturnValue 'IfPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing IfContent at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_If_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
        
    )
    (FillIfContent (. ReturnValue 'If) tokenizer)
    (while (&& (. LOOKTable 100 0 :type (peek tokenizer 0))(. LOOKTable 100 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 101 0 :type (peek tokenizer 0))(. LOOKTable 101 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 14))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected else"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
                
            )
            (append (. ReturnValue 'ElseClauses) (IfContent))
            (FillIfContent (back (. ReturnValue 'ElseClauses)) tokenizer)
            
        
        else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L18"))
        
    
    )
)
(if (&& (. LOOKTable 104 0 :type (peek tokenizer 0))(. LOOKTable 104 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 105 0 :type (peek tokenizer 0))(. LOOKTable 105 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 14))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected else"))
            
        )
        (consume-token tokenizer)
        (if (not (&& (. LOOKTable 102 0 :type (peek tokenizer 0))(. LOOKTable 102 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected _L20"))
            
        )
        (append (. ReturnValue 'ElseClauses) (IfContent))
        (Fill_L20 (back (. ReturnValue 'ElseClauses)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L19"))
    

)
)

)
(defun ParseStatement_If_0 (tokenizer)
    (setl ReturnValue (Statement_If))
    (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
        
    )
    (FillIfContent (. ReturnValue 'If) tokenizer)
    (while (&& (. LOOKTable 100 0 :type (peek tokenizer 0))(. LOOKTable 100 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 101 0 :type (peek tokenizer 0))(. LOOKTable 101 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 14))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected else"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
                
            )
            (append (. ReturnValue 'ElseClauses) (IfContent))
            (FillIfContent (back (. ReturnValue 'ElseClauses)) tokenizer)
            
        
        else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L18"))
        
    
    )
)
(if (&& (. LOOKTable 104 0 :type (peek tokenizer 0))(. LOOKTable 104 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 105 0 :type (peek tokenizer 0))(. LOOKTable 105 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 14))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected else"))
            
        )
        (consume-token tokenizer)
        (if (not (&& (. LOOKTable 102 0 :type (peek tokenizer 0))(. LOOKTable 102 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected _L20"))
            
        )
        (append (. ReturnValue 'ElseClauses) (IfContent))
        (Fill_L20 (back (. ReturnValue 'ElseClauses)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L19"))
    

)
)
(return ReturnValue)

)
(defun FillStatement_If (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
        
    )
    (FillIfContent (. ReturnValue 'If) tokenizer)
    (while (&& (. LOOKTable 100 0 :type (peek tokenizer 0))(. LOOKTable 100 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 101 0 :type (peek tokenizer 0))(. LOOKTable 101 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 14))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected else"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
                
            )
            (append (. ReturnValue 'ElseClauses) (IfContent))
            (FillIfContent (back (. ReturnValue 'ElseClauses)) tokenizer)
            
        
        else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L18"))
        
    
    )
)
(if (&& (. LOOKTable 104 0 :type (peek tokenizer 0))(. LOOKTable 104 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 105 0 :type (peek tokenizer 0))(. LOOKTable 105 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 14))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected else"))
            
        )
        (consume-token tokenizer)
        (if (not (&& (. LOOKTable 102 0 :type (peek tokenizer 0))(. LOOKTable 102 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected _L20"))
            
        )
        (append (. ReturnValue 'ElseClauses) (IfContent))
        (Fill_L20 (back (. ReturnValue 'ElseClauses)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L19"))
    

)
)

)
(defun ParseStatement_If (tokenizer)
    (setl ReturnValue (Statement_If))
    (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
        
    )
    (FillIfContent (. ReturnValue 'If) tokenizer)
    (while (&& (. LOOKTable 100 0 :type (peek tokenizer 0))(. LOOKTable 100 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 101 0 :type (peek tokenizer 0))(. LOOKTable 101 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 14))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected else"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 96 0 :type (peek tokenizer 0))(. LOOKTable 96 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L18 at position "  (str :position (peek tokenizer 0)) ": expected IfContent"))
                
            )
            (append (. ReturnValue 'ElseClauses) (IfContent))
            (FillIfContent (back (. ReturnValue 'ElseClauses)) tokenizer)
            
        
        else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L18"))
        
    
    )
)
(if (&& (. LOOKTable 104 0 :type (peek tokenizer 0))(. LOOKTable 104 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 105 0 :type (peek tokenizer 0))(. LOOKTable 105 1 :type (peek tokenizer 1)))
        (if (not (eq :type (peek tokenizer 0) 14))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected else"))
            
        )
        (consume-token tokenizer)
        (if (not (&& (. LOOKTable 102 0 :type (peek tokenizer 0))(. LOOKTable 102 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L19 at position "  (str :position (peek tokenizer 0)) ": expected _L20"))
            
        )
        (append (. ReturnValue 'ElseClauses) (IfContent))
        (Fill_L20 (back (. ReturnValue 'ElseClauses)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_If at position "  (str :position (peek tokenizer 0)) ": expected _L19"))
    

)
)
(return ReturnValue)

)
(defun Fill_L20_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (setl (. ReturnValue 'HasCondition) false)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun Parse_L20_0 (tokenizer)
    (setl ReturnValue (IfContent))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (setl (. ReturnValue 'HasCondition) false)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun Fill_L20 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (setl (. ReturnValue 'HasCondition) false)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun Parse_L20 (tokenizer)
    (setl ReturnValue (IfContent))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (setl (. ReturnValue 'HasCondition) false)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing _L20 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_While_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'WhilePart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 13))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected while"))
        
    )
    (setl (. ReturnValue 'WhilePart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_While_0 (tokenizer)
    (setl ReturnValue (Statement_While))
    (setl (. ReturnValue 'WhilePart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 13))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected while"))
        
    )
    (setl (. ReturnValue 'WhilePart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_While (ReturnValue tokenizer)
    (setl (. ReturnValue 'WhilePart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 13))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected while"))
        
    )
    (setl (. ReturnValue 'WhilePart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_While (tokenizer)
    (setl ReturnValue (Statement_While))
    (setl (. ReturnValue 'WhilePart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 13))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected while"))
        
    )
    (setl (. ReturnValue 'WhilePart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Condition)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
        
    )
    (consume-token tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Body) (Statement))
        (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
        
    )
    (if (not (eq :type (peek tokenizer 0) 7))
        (error (+ "Error parsing Statement_While at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_Expr_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_Expr_0 (tokenizer)
    (setl ReturnValue (Statement_Expr))
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_Expr (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_Expr (tokenizer)
    (setl ReturnValue (Statement_Expr))
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Expr)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Expr at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_Return_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'ReturnPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected return"))
        
    )
    (setl (. ReturnValue 'ReturnPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_Return_0 (tokenizer)
    (setl ReturnValue (Statement_Return))
    (setl (. ReturnValue 'ReturnPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected return"))
        
    )
    (setl (. ReturnValue 'ReturnPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillStatement_Return (ReturnValue tokenizer)
    (setl (. ReturnValue 'ReturnPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected return"))
        
    )
    (setl (. ReturnValue 'ReturnPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseStatement_Return (tokenizer)
    (setl ReturnValue (Statement_Return))
    (setl (. ReturnValue 'ReturnPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected return"))
        
    )
    (setl (. ReturnValue 'ReturnPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing Statement_Return at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillArgument_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Type) tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    
)
(defun ParseArgument_0 (tokenizer)
    (setl ReturnValue (Argument))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Type) tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (return ReturnValue)
    
)
(defun FillArgument_1 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    
)
(defun ParseArgument_1 (tokenizer)
    (setl ReturnValue (Argument))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (return ReturnValue)
    
)
(defun FillArgument (ReturnValue tokenizer)
    (if (&& (. LOOKTable 113 0 :type (peek tokenizer 0))(. LOOKTable 113 1 :type (peek tokenizer 1)))
        (FillArgument_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 114 0 :type (peek tokenizer 0))(. LOOKTable 114 1 :type (peek tokenizer 1)))
        (FillArgument_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
    

)
)
(defun ParseArgument (tokenizer)
    (setl ReturnValue (Argument))
    (if (&& (. LOOKTable 113 0 :type (peek tokenizer 0))(. LOOKTable 113 1 :type (peek tokenizer 1)))
        (FillArgument_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 114 0 :type (peek tokenizer 0))(. LOOKTable 114 1 :type (peek tokenizer 1)))
        (FillArgument_1 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing Argument at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
    

)(return ReturnValue)

)
(defun FillStatement_Func_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'FuncPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected func"))
        
    )
    (setl (. ReturnValue 'FuncPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 119 0 :type (peek tokenizer 0))(. LOOKTable 119 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 120 0 :type (peek tokenizer 0))(. LOOKTable 120 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 117 0 :type (peek tokenizer 0))(. LOOKTable 117 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 118 0 :type (peek tokenizer 0))(. LOOKTable 118 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected _L22"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected _L21"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Content) (Statement))
    (set (back (. ReturnValue 'Content))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_Func_0 (tokenizer)
    (setl ReturnValue (Statement_Func))
    (setl (. ReturnValue 'FuncPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected func"))
        
    )
    (setl (. ReturnValue 'FuncPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 119 0 :type (peek tokenizer 0))(. LOOKTable 119 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 120 0 :type (peek tokenizer 0))(. LOOKTable 120 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 117 0 :type (peek tokenizer 0))(. LOOKTable 117 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 118 0 :type (peek tokenizer 0))(. LOOKTable 118 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected _L22"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected _L21"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Content) (Statement))
    (set (back (. ReturnValue 'Content))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillStatement_Func (ReturnValue tokenizer)
    (setl (. ReturnValue 'FuncPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected func"))
        
    )
    (setl (. ReturnValue 'FuncPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 119 0 :type (peek tokenizer 0))(. LOOKTable 119 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 120 0 :type (peek tokenizer 0))(. LOOKTable 120 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 117 0 :type (peek tokenizer 0))(. LOOKTable 117 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 118 0 :type (peek tokenizer 0))(. LOOKTable 118 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected _L22"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected _L21"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Content) (Statement))
    (set (back (. ReturnValue 'Content))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_Func (tokenizer)
    (setl ReturnValue (Statement_Func))
    (setl (. ReturnValue 'FuncPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected func"))
        
    )
    (setl (. ReturnValue 'FuncPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 119 0 :type (peek tokenizer 0))(. LOOKTable 119 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 120 0 :type (peek tokenizer 0))(. LOOKTable 120 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 117 0 :type (peek tokenizer 0))(. LOOKTable 117 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 118 0 :type (peek tokenizer 0))(. LOOKTable 118 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L22 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L21 at position "  (str :position (peek tokenizer 0)) ": expected _L22"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected _L21"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Content) (Statement))
    (set (back (. ReturnValue 'Content))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Func at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillStatement_Class_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'ClassPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected class"))
        
    )
    (setl (. ReturnValue 'ClassPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (&& (. LOOKTable 125 0 :type (peek tokenizer 0))(. LOOKTable 125 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 126 0 :type (peek tokenizer 0))(. LOOKTable 126 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 5))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected colon"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Parents) (Idf))
            (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
            (while (&& (. LOOKTable 123 0 :type (peek tokenizer 0))(. LOOKTable 123 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 124 0 :type (peek tokenizer 0))(. LOOKTable 124 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Parents) (Idf))
                    (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
                    
                
                else (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected _L24"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L23"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 135 0 :type (peek tokenizer 0))(. LOOKTable 135 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 136 0 :type (peek tokenizer 0))(. LOOKTable 136 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 127 0 :type (peek tokenizer 0))(. LOOKTable 127 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L26"))
            
        )
        (append (. ReturnValue 'Variables) (MemberVariable))
        (Fill_L26 (back (. ReturnValue 'Variables)) tokenizer)
        
    
    else if (&& (. LOOKTable 137 0 :type (peek tokenizer 0))(. LOOKTable 137 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 133 0 :type (peek tokenizer 0))(. LOOKTable 133 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L27"))
            
        )
        (append (. ReturnValue 'Methods) (MemberFunc))
        (Fill_L27 (back (. ReturnValue 'Methods)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L25"))
    

)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_Class_0 (tokenizer)
    (setl ReturnValue (Statement_Class))
    (setl (. ReturnValue 'ClassPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected class"))
        
    )
    (setl (. ReturnValue 'ClassPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (&& (. LOOKTable 125 0 :type (peek tokenizer 0))(. LOOKTable 125 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 126 0 :type (peek tokenizer 0))(. LOOKTable 126 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 5))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected colon"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Parents) (Idf))
            (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
            (while (&& (. LOOKTable 123 0 :type (peek tokenizer 0))(. LOOKTable 123 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 124 0 :type (peek tokenizer 0))(. LOOKTable 124 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Parents) (Idf))
                    (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
                    
                
                else (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected _L24"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L23"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 135 0 :type (peek tokenizer 0))(. LOOKTable 135 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 136 0 :type (peek tokenizer 0))(. LOOKTable 136 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 127 0 :type (peek tokenizer 0))(. LOOKTable 127 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L26"))
            
        )
        (append (. ReturnValue 'Variables) (MemberVariable))
        (Fill_L26 (back (. ReturnValue 'Variables)) tokenizer)
        
    
    else if (&& (. LOOKTable 137 0 :type (peek tokenizer 0))(. LOOKTable 137 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 133 0 :type (peek tokenizer 0))(. LOOKTable 133 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L27"))
            
        )
        (append (. ReturnValue 'Methods) (MemberFunc))
        (Fill_L27 (back (. ReturnValue 'Methods)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L25"))
    

)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillStatement_Class (ReturnValue tokenizer)
    (setl (. ReturnValue 'ClassPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected class"))
        
    )
    (setl (. ReturnValue 'ClassPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (&& (. LOOKTable 125 0 :type (peek tokenizer 0))(. LOOKTable 125 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 126 0 :type (peek tokenizer 0))(. LOOKTable 126 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 5))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected colon"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Parents) (Idf))
            (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
            (while (&& (. LOOKTable 123 0 :type (peek tokenizer 0))(. LOOKTable 123 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 124 0 :type (peek tokenizer 0))(. LOOKTable 124 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Parents) (Idf))
                    (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
                    
                
                else (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected _L24"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L23"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 135 0 :type (peek tokenizer 0))(. LOOKTable 135 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 136 0 :type (peek tokenizer 0))(. LOOKTable 136 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 127 0 :type (peek tokenizer 0))(. LOOKTable 127 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L26"))
            
        )
        (append (. ReturnValue 'Variables) (MemberVariable))
        (Fill_L26 (back (. ReturnValue 'Variables)) tokenizer)
        
    
    else if (&& (. LOOKTable 137 0 :type (peek tokenizer 0))(. LOOKTable 137 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 133 0 :type (peek tokenizer 0))(. LOOKTable 133 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L27"))
            
        )
        (append (. ReturnValue 'Methods) (MemberFunc))
        (Fill_L27 (back (. ReturnValue 'Methods)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L25"))
    

)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun ParseStatement_Class (tokenizer)
    (setl ReturnValue (Statement_Class))
    (setl (. ReturnValue 'ClassPart 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected class"))
        
    )
    (setl (. ReturnValue 'ClassPart 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (&& (. LOOKTable 125 0 :type (peek tokenizer 0))(. LOOKTable 125 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 126 0 :type (peek tokenizer 0))(. LOOKTable 126 1 :type (peek tokenizer 1)))
            (if (not (eq :type (peek tokenizer 0) 5))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected colon"))
                
            )
            (consume-token tokenizer)
            (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                
            )
            (append (. ReturnValue 'Parents) (Idf))
            (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
            (while (&& (. LOOKTable 123 0 :type (peek tokenizer 0))(. LOOKTable 123 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 124 0 :type (peek tokenizer 0))(. LOOKTable 124 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L24 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
                        
                    )
                    (append (. ReturnValue 'Parents) (Idf))
                    (FillIdf (back (. ReturnValue 'Parents)) tokenizer)
                    
                
                else (error (+ "Error parsing _L23 at position "  (str :position (peek tokenizer 0)) ": expected _L24"))
                
            
            )
    )
    

else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L23"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 135 0 :type (peek tokenizer 0))(. LOOKTable 135 1 :type (peek tokenizer 1)))
    (if (&& (. LOOKTable 136 0 :type (peek tokenizer 0))(. LOOKTable 136 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 127 0 :type (peek tokenizer 0))(. LOOKTable 127 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L26"))
            
        )
        (append (. ReturnValue 'Variables) (MemberVariable))
        (Fill_L26 (back (. ReturnValue 'Variables)) tokenizer)
        
    
    else if (&& (. LOOKTable 137 0 :type (peek tokenizer 0))(. LOOKTable 137 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 133 0 :type (peek tokenizer 0))(. LOOKTable 133 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L25 at position "  (str :position (peek tokenizer 0)) ": expected _L27"))
            
        )
        (append (. ReturnValue 'Methods) (MemberFunc))
        (Fill_L27 (back (. ReturnValue 'Methods)) tokenizer)
        
    
    else (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected _L25"))
    

)
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing Statement_Class at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun Fill_L26_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 32))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun Parse_L26_0 (tokenizer)
    (setl ReturnValue (MemberVariable))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 32))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun Fill_L26 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 32))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    
)
(defun Parse_L26 (tokenizer)
    (setl ReturnValue (MemberVariable))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 32))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected eq"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 90 0 :type (peek tokenizer 0))(. LOOKTable 90 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set (. ReturnValue 'Value)(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 15))
        (error (+ "Error parsing _L26 at position "  (str :position (peek tokenizer 0)) ": expected semi"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun Fill_L27_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 131 0 :type (peek tokenizer 0))(. LOOKTable 131 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 132 0 :type (peek tokenizer 0))(. LOOKTable 132 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 129 0 :type (peek tokenizer 0))(. LOOKTable 129 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 130 0 :type (peek tokenizer 0))(. LOOKTable 130 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected _L29"))
                
            
            )
    )
    

else (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected _L28"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun Parse_L27_0 (tokenizer)
    (setl ReturnValue (MemberFunc))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 131 0 :type (peek tokenizer 0))(. LOOKTable 131 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 132 0 :type (peek tokenizer 0))(. LOOKTable 132 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 129 0 :type (peek tokenizer 0))(. LOOKTable 129 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 130 0 :type (peek tokenizer 0))(. LOOKTable 130 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected _L29"))
                
            
            )
    )
    

else (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected _L28"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun Fill_L27 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 131 0 :type (peek tokenizer 0))(. LOOKTable 131 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 132 0 :type (peek tokenizer 0))(. LOOKTable 132 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 129 0 :type (peek tokenizer 0))(. LOOKTable 129 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 130 0 :type (peek tokenizer 0))(. LOOKTable 130 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected _L29"))
                
            
            )
    )
    

else (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected _L28"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)

)
(defun Parse_L27 (tokenizer)
    (setl ReturnValue (MemberFunc))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Name) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 33))
        (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 131 0 :type (peek tokenizer 0))(. LOOKTable 131 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 132 0 :type (peek tokenizer 0))(. LOOKTable 132 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                
            )
            (append (. ReturnValue 'Args) (Argument))
            (FillArgument (back (. ReturnValue 'Args)) tokenizer)
            (while (&& (. LOOKTable 129 0 :type (peek tokenizer 0))(. LOOKTable 129 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 130 0 :type (peek tokenizer 0))(. LOOKTable 130 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 35))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 112 0 :type (peek tokenizer 0))(. LOOKTable 112 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L29 at position "  (str :position (peek tokenizer 0)) ": expected Argument"))
                        
                    )
                    (append (. ReturnValue 'Args) (Argument))
                    (FillArgument (back (. ReturnValue 'Args)) tokenizer)
                    
                
                else (error (+ "Error parsing _L28 at position "  (str :position (peek tokenizer 0)) ": expected _L29"))
                
            
            )
    )
    

else (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected _L28"))


)
)
(if (not (eq :type (peek tokenizer 0) 34))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected lcurl"))
    
)
(consume-token tokenizer)
(while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Body) (Statement))
    (set (back (. ReturnValue 'Body))(ParseStatement tokenizer))
    
)
(if (not (eq :type (peek tokenizer 0) 7))
    (error (+ "Error parsing _L27 at position "  (str :position (peek tokenizer 0)) ": expected rcurl"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillStatement_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 92 0 :type (peek tokenizer 0))(. LOOKTable 92 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_For"))
        
    )
    (set ReturnValue(ParseStatement_For tokenizer))
    
)
(defun ParseStatement_0 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 92 0 :type (peek tokenizer 0))(. LOOKTable 92 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_For"))
        
    )
    (set ReturnValue(ParseStatement_For tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_1 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 106 0 :type (peek tokenizer 0))(. LOOKTable 106 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_While"))
        
    )
    (set ReturnValue(ParseStatement_While tokenizer))
    
)
(defun ParseStatement_1 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 106 0 :type (peek tokenizer 0))(. LOOKTable 106 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_While"))
        
    )
    (set ReturnValue(ParseStatement_While tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_2 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 108 0 :type (peek tokenizer 0))(. LOOKTable 108 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Expr"))
        
    )
    (set ReturnValue(ParseStatement_Expr tokenizer))
    
)
(defun ParseStatement_2 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 108 0 :type (peek tokenizer 0))(. LOOKTable 108 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Expr"))
        
    )
    (set ReturnValue(ParseStatement_Expr tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_3 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 98 0 :type (peek tokenizer 0))(. LOOKTable 98 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_If"))
        
    )
    (set ReturnValue(ParseStatement_If tokenizer))
    
)
(defun ParseStatement_3 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 98 0 :type (peek tokenizer 0))(. LOOKTable 98 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_If"))
        
    )
    (set ReturnValue(ParseStatement_If tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_4 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 110 0 :type (peek tokenizer 0))(. LOOKTable 110 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Return"))
        
    )
    (set ReturnValue(ParseStatement_Return tokenizer))
    
)
(defun ParseStatement_4 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 110 0 :type (peek tokenizer 0))(. LOOKTable 110 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Return"))
        
    )
    (set ReturnValue(ParseStatement_Return tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_5 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 115 0 :type (peek tokenizer 0))(. LOOKTable 115 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Func"))
        
    )
    (set ReturnValue(ParseStatement_Func tokenizer))
    
)
(defun ParseStatement_5 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 115 0 :type (peek tokenizer 0))(. LOOKTable 115 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Func"))
        
    )
    (set ReturnValue(ParseStatement_Func tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement_6 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 121 0 :type (peek tokenizer 0))(. LOOKTable 121 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Class"))
        
    )
    (set ReturnValue(ParseStatement_Class tokenizer))
    
)
(defun ParseStatement_6 (tokenizer)
    (setl ReturnValue (Statement))
    (if (not (&& (. LOOKTable 121 0 :type (peek tokenizer 0))(. LOOKTable 121 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement_Class"))
        
    )
    (set ReturnValue(ParseStatement_Class tokenizer))
    (return ReturnValue)
    
)
(defun FillStatement (ReturnValue tokenizer)
    (if (&& (. LOOKTable 139 0 :type (peek tokenizer 0))(. LOOKTable 139 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_0 tokenizer))
        
    
    else if (&& (. LOOKTable 140 0 :type (peek tokenizer 0))(. LOOKTable 140 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_1 tokenizer))
        
    
    else if (&& (. LOOKTable 141 0 :type (peek tokenizer 0))(. LOOKTable 141 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_2 tokenizer))
        
    
    else if (&& (. LOOKTable 142 0 :type (peek tokenizer 0))(. LOOKTable 142 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_3 tokenizer))
        
    
    else if (&& (. LOOKTable 143 0 :type (peek tokenizer 0))(. LOOKTable 143 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_4 tokenizer))
        
    
    else if (&& (. LOOKTable 144 0 :type (peek tokenizer 0))(. LOOKTable 144 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_5 tokenizer))
        
    
    else if (&& (. LOOKTable 145 0 :type (peek tokenizer 0))(. LOOKTable 145 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_6 tokenizer))
        
    
    else (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement"))
    

)
)
(defun ParseStatement (tokenizer)
    (setl ReturnValue (Statement))
    (if (&& (. LOOKTable 139 0 :type (peek tokenizer 0))(. LOOKTable 139 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_0 tokenizer))
        
    
    else if (&& (. LOOKTable 140 0 :type (peek tokenizer 0))(. LOOKTable 140 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_1 tokenizer))
        
    
    else if (&& (. LOOKTable 141 0 :type (peek tokenizer 0))(. LOOKTable 141 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_2 tokenizer))
        
    
    else if (&& (. LOOKTable 142 0 :type (peek tokenizer 0))(. LOOKTable 142 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_3 tokenizer))
        
    
    else if (&& (. LOOKTable 143 0 :type (peek tokenizer 0))(. LOOKTable 143 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_4 tokenizer))
        
    
    else if (&& (. LOOKTable 144 0 :type (peek tokenizer 0))(. LOOKTable 144 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_5 tokenizer))
        
    
    else if (&& (. LOOKTable 145 0 :type (peek tokenizer 0))(. LOOKTable 145 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseStatement_6 tokenizer))
        
    
    else (error (+ "Error parsing Statement at position "  (str :position (peek tokenizer 0)) ": expected Statement"))
    

)(return ReturnValue)

)
(defun FillStatementList_0 (ReturnValue tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Statements) (Statement))
        (set (back (. ReturnValue 'Statements))(ParseStatement tokenizer))
        
    )
    
)
(defun ParseStatementList_0 (tokenizer)
    (setl ReturnValue (StatementList))
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Statements) (Statement))
        (set (back (. ReturnValue 'Statements))(ParseStatement tokenizer))
        
    )
    (return ReturnValue)
    
)
(defun FillStatementList (ReturnValue tokenizer)
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Statements) (Statement))
        (set (back (. ReturnValue 'Statements))(ParseStatement tokenizer))
        
    )
    
)
(defun ParseStatementList (tokenizer)
    (setl ReturnValue (StatementList))
    (while (&& (. LOOKTable 138 0 :type (peek tokenizer 0))(. LOOKTable 138 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Statements) (Statement))
        (set (back (. ReturnValue 'Statements))(ParseStatement tokenizer))
        
    )
    (return ReturnValue)
    
)
(set LOOKTable (list (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false true false false true false false false false true true true true true false true true true true true true true true true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false true false false true false false false false true true true true true false true true true true true true true true true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false false true true true true false true true true true true true true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false true true true true false true true true true true true true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false true true true true false true true true true true true true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false true) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true true true true) ) (list (list false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false) (list false false false false false true false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false) (list false false false false false true false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true true) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false true false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false true true true true true false true true true true true true true true true true true) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false true true true true true false true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false false true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false false true) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false true true) ) (list (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true true true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true true true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false false true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false false true true true true true true true true true true true true true true true true true true false false true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true true true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false) (list false false false false false true false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false) (list false false false false false true true true true true false false false false false true false false false false false true true true true false true true true true true true true false true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list false false false false false false false false true false false false false false false false false false false false false true true true true false true true true true true true true false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false true true true true false true true true true true true true false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false true true false false true) ) (list (list false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true true false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true true true true) ) (list (list false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false true) ) (list (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) ) (list (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true true true true false true true true false true true false true false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true true true true false true true true false true true false true false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) (list (list false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true true true) ) (list (list false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false true true true) ) (list (list false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false true true true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true true false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false false true) ) (list (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false true false false true) ) (list (list true true true true true false true false true false true true false true false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) (list (list false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) (list (list false false false false false false false false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list true true false false false false true false true false false false false false false false true true true true true false false false false true false false false false false false false true false false false) ) (list (list false false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list false false true false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false false) (list false false false false false false false false false false false false false false false false false false false false true false false false false false false false false false false false false false false false false) ) (list (list true true true true true false true false true false true true false true false false true true true true true false false false false true false false false false false false false true false false true) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) (list (list true true true true true false true false true false true true false true false false true true true true true false false false false true false false false false false false false true false false true) (list true true false false false true true true true true false false false false false true true true true true true true true true true true true true true true true true true true false false true) ) ))