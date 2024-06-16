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
(defclass Expr_FuncCall(Expr)
    (Identifier (Idf))
    (Arguments (list))
    
)
(defclass OperatorPart()
    (Operator null)
    (Rhs (Expr))
    
)
(defclass Op()
    (Value null)
    
)
(defclass Expr_Operators(Expr)
    (Lhs (Expr))
    (Parts (list))
    
)
(defun get-tokenizer ()
    (tokenizer "\\s+"(list "\\w+" "\\d+" "\\." "\\+" "\\(" "\\)" ","))
)
(defun FillIdf_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Idf at position"  (str (position (peek tokenizer))) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun FillIdf (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Idf at position"  (str (position (peek tokenizer))) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseIdf (tokenizer)
    (setl ReturnValue (Idf))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Idf at position"  (str (position (peek tokenizer))) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Idf_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun FillExpr_Idf (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun ParseExpr_Idf (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (return ReturnValue)
    
)
(defun FillOp_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Op at position"  (str (position (peek tokenizer))) ": expected dot"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun FillOp (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Op at position"  (str (position (peek tokenizer))) ": expected dot"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseOp (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Op at position"  (str (position (peek tokenizer))) ": expected dot"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_FuncCall_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 11 0 :type (peek tokenizer 0))(. LOOKTable 11 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 8 0 :type (peek tokenizer 0))(. LOOKTable 8 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 6))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun FillExpr_FuncCall (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 11 0 :type (peek tokenizer 0))(. LOOKTable 11 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 8 0 :type (peek tokenizer 0))(. LOOKTable 8 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 6))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_FuncCall (tokenizer)
    (setl ReturnValue (Expr_FuncCall))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 11 0 :type (peek tokenizer 0))(. LOOKTable 11 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 8 0 :type (peek tokenizer 0))(. LOOKTable 8 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 6))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 22 0 :type (peek tokenizer 0))(. LOOKTable 22 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str (position (peek tokenizer))) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str (position (peek tokenizer))) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 5))
    (error (+ "Error parsing Expr_FuncCall at position"  (str (position (peek tokenizer))) ": expected rpar"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Operators_0 (ReturnValue tokenizer)
    (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing Expr_Operators at position"  (str (position (peek tokenizer))) ": expected _L3"))
    

)(while (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Parts) (OperatorPart))
    (Fill_L4 (back (. ReturnValue 'Parts)) tokenizer)
    
)

)
(defun FillExpr_Operators (ReturnValue tokenizer)
    (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing Expr_Operators at position"  (str (position (peek tokenizer))) ": expected _L3"))
    

)(while (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Parts) (OperatorPart))
    (Fill_L4 (back (. ReturnValue 'Parts)) tokenizer)
    
)

)
(defun ParseExpr_Operators (tokenizer)
    (setl ReturnValue (Expr_Operators))
    (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L3 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Lhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing Expr_Operators at position"  (str (position (peek tokenizer))) ": expected _L3"))
    

)(while (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
    (append (. ReturnValue 'Parts) (OperatorPart))
    (Fill_L4 (back (. ReturnValue 'Parts)) tokenizer)
    
)
(return ReturnValue)

)
(defun Fill_L4_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected _L5"))
    

)
)
(defun Fill_L4 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected _L5"))
    

)
)
(defun Parse_L4 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_Idf"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Idf tokenizer))
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L5 at position"  (str (position (peek tokenizer))) ": expected Expr_FuncCall"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_FuncCall tokenizer))
        
    
    else (error (+ "Error parsing _L4 at position"  (str (position (peek tokenizer))) ": expected _L5"))
    

)(return ReturnValue)

)
(defun FillExpr_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str (position (peek tokenizer))) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun FillExpr (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str (position (peek tokenizer))) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun ParseExpr (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str (position (peek tokenizer))) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    (return ReturnValue)
    
)
(set LOOKTable (list (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false false true true true) ) (list (list true false false false false false false false) (list false false true false false true true true) ) (list (list false false true false false false false false) (list true false false false false false false true) ) (list (list false false true false false false false false) (list true false false false false false false true) ) (list (list true false false false false false false false) (list false false false false true false false true) ) (list (list true false false false false false false false) (list false false false false true false false true) ) (list (list false false false false false false true false) (list true false false false false false false false) ) (list (list false false false false false false true false) (list true false false false false false false false) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false false true true true) ) (list (list true false false false false false false false) (list false false false false true false false true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false false true true true) ) (list (list true false false false false false false false) (list false false false false true false false true) ) (list (list false false true false false false false false) (list true false false false false false false true) ) (list (list false false true false false false false false) (list true false false false false false false true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) (list (list true false false false false false false false) (list false false true false true true true true) ) ))