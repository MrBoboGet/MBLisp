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
    (tokenizer "\\s+"(list "\\d+" "\\w+" "\\." "\\+" "\\*" "\\(" "\\)" ","))
)
(defun FillIdf_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 1))
        (error (+ "Error parsing Idf at position"  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseIdf_0 (tokenizer)
    (setl ReturnValue (Idf))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 1))
        (error (+ "Error parsing Idf at position"  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillIdf (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 1))
        (error (+ "Error parsing Idf at position"  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseIdf (tokenizer)
    (setl ReturnValue (Idf))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 1))
        (error (+ "Error parsing Idf at position"  (str :position (peek tokenizer 0)) ": expected idf"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Idf_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun ParseExpr_Idf_0 (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Idf (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    
)
(defun ParseExpr_Idf (tokenizer)
    (setl ReturnValue (Expr_Idf))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Idf at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Num_0 (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Expr_Num at position"  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Num_0 (tokenizer)
    (setl ReturnValue (Expr_Num))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Expr_Num at position"  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Num (ReturnValue tokenizer)
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Expr_Num at position"  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    
)
(defun ParseExpr_Num (tokenizer)
    (setl ReturnValue (Expr_Num))
    (setl (. ReturnValue 'Position) :position (peek tokenizer 0))
    (if (not (eq :type (peek tokenizer 0) 0))
        (error (+ "Error parsing Expr_Num at position"  (str :position (peek tokenizer 0)) ": expected num"))
        
    )
    (setl (. ReturnValue 'Value) (int :value (peek tokenizer 0)))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillOp_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected dot"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseOp_0 (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 2))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected dot"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillOp_1 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected plus"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseOp_1 (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 3))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected plus"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillOp_2 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected times"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    
)
(defun ParseOp_2 (tokenizer)
    (setl ReturnValue (Op))
    (if (not (eq :type (peek tokenizer 0) 4))
        (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected times"))
        
    )
    (setl (. ReturnValue 'Value) :value (peek tokenizer 0))
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillOp (ReturnValue tokenizer)
    (if (&& (. LOOKTable 7 0 :type (peek tokenizer 0))(. LOOKTable 7 1 :type (peek tokenizer 1)))
        (FillOp_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 8 0 :type (peek tokenizer 0))(. LOOKTable 8 1 :type (peek tokenizer 1)))
        (FillOp_1 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (FillOp_2 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected Op"))
    

)
)
(defun ParseOp (tokenizer)
    (setl ReturnValue (Op))
    (if (&& (. LOOKTable 7 0 :type (peek tokenizer 0))(. LOOKTable 7 1 :type (peek tokenizer 1)))
        (FillOp_0 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 8 0 :type (peek tokenizer 0))(. LOOKTable 8 1 :type (peek tokenizer 1)))
        (FillOp_1 ReturnValue tokenizer)
        
    
    else if (&& (. LOOKTable 9 0 :type (peek tokenizer 0))(. LOOKTable 9 1 :type (peek tokenizer 1)))
        (FillOp_2 ReturnValue tokenizer)
        
    
    else (error (+ "Error parsing Op at position"  (str :position (peek tokenizer 0)) ": expected Op"))
    

)(return ReturnValue)

)
(defun FillExpr_FuncCall_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 13 0 :type (peek tokenizer 0))(. LOOKTable 13 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 7))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_FuncCall_0 (tokenizer)
    (setl ReturnValue (Expr_FuncCall))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 13 0 :type (peek tokenizer 0))(. LOOKTable 13 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 7))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_FuncCall (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 13 0 :type (peek tokenizer 0))(. LOOKTable 13 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 7))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)

)
(defun ParseExpr_FuncCall (tokenizer)
    (setl ReturnValue (Expr_FuncCall))
    (if (not (&& (. LOOKTable 0 0 :type (peek tokenizer 0))(. LOOKTable 0 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected Idf"))
        
    )
    (FillIdf (. ReturnValue 'Identifier) tokenizer)
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (&& (. LOOKTable 14 0 :type (peek tokenizer 0))(. LOOKTable 14 1 :type (peek tokenizer 1)))
        (if (&& (. LOOKTable 15 0 :type (peek tokenizer 0))(. LOOKTable 15 1 :type (peek tokenizer 1)))
            (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                
            )
            (append (. ReturnValue 'Arguments) (Expr))
            (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
            (while (&& (. LOOKTable 12 0 :type (peek tokenizer 0))(. LOOKTable 12 1 :type (peek tokenizer 1)))
                (if (&& (. LOOKTable 13 0 :type (peek tokenizer 0))(. LOOKTable 13 1 :type (peek tokenizer 1)))
                    (if (not (eq :type (peek tokenizer 0) 7))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected comma"))
                        
                    )
                    (consume-token tokenizer)
                    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
                        (error (+ "Error parsing _L2 at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
                        
                    )
                    (append (. ReturnValue 'Arguments) (Expr))
                    (set (back (. ReturnValue 'Arguments))(ParseExpr tokenizer))
                    
                
                else (error (+ "Error parsing _L1 at position"  (str :position (peek tokenizer 0)) ": expected _L2"))
                
            
            )
    )
    

else (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected _L1"))


)
)
(if (not (eq :type (peek tokenizer 0) 6))
    (error (+ "Error parsing Expr_FuncCall at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
    
)
(consume-token tokenizer)
(return ReturnValue)

)
(defun FillExpr_Term_0 (ReturnValue tokenizer)
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set ReturnValue(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    
)
(defun ParseExpr_Term_0 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (eq :type (peek tokenizer 0) 5))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected lpar"))
        
    )
    (consume-token tokenizer)
    (if (not (&& (. LOOKTable 27 0 :type (peek tokenizer 0))(. LOOKTable 27 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr"))
        
    )
    (set ReturnValue(ParseExpr tokenizer))
    (if (not (eq :type (peek tokenizer 0) 6))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected rpar"))
        
    )
    (consume-token tokenizer)
    (return ReturnValue)
    
)
(defun FillExpr_Term_1 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Idf"))
        
    )
    (set ReturnValue(ParseExpr_Idf tokenizer))
    
)
(defun ParseExpr_Term_1 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 2 0 :type (peek tokenizer 0))(. LOOKTable 2 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Idf"))
        
    )
    (set ReturnValue(ParseExpr_Idf tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_2 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set ReturnValue(ParseExpr_FuncCall tokenizer))
    
)
(defun ParseExpr_Term_2 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 10 0 :type (peek tokenizer 0))(. LOOKTable 10 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_FuncCall"))
        
    )
    (set ReturnValue(ParseExpr_FuncCall tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term_3 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Num"))
        
    )
    (set ReturnValue(ParseExpr_Num tokenizer))
    
)
(defun ParseExpr_Term_3 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 4 0 :type (peek tokenizer 0))(. LOOKTable 4 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Num"))
        
    )
    (set ReturnValue(ParseExpr_Num tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr_Term (ReturnValue tokenizer)
    (if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_0 tokenizer))
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_1 tokenizer))
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_2 tokenizer))
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_3 tokenizer))
        
    
    else (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
    

)
)
(defun ParseExpr_Term (tokenizer)
    (setl ReturnValue (Expr))
    (if (&& (. LOOKTable 17 0 :type (peek tokenizer 0))(. LOOKTable 17 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_0 tokenizer))
        
    
    else if (&& (. LOOKTable 18 0 :type (peek tokenizer 0))(. LOOKTable 18 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_1 tokenizer))
        
    
    else if (&& (. LOOKTable 19 0 :type (peek tokenizer 0))(. LOOKTable 19 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_2 tokenizer))
        
    
    else if (&& (. LOOKTable 20 0 :type (peek tokenizer 0))(. LOOKTable 20 1 :type (peek tokenizer 1)))
        (set ReturnValue(ParseExpr_Term_3 tokenizer))
        
    
    else (error (+ "Error parsing Expr_Term at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
    

)(return ReturnValue)

)
(defun FillExpr_Operators_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L3 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    
)
(defun ParseExpr_Operators_0 (tokenizer)
    (setl ReturnValue (Expr_Operators))
    (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L3 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    (return ReturnValue)
    
)
(defun FillExpr_Operators (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L3 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    
)
(defun ParseExpr_Operators (tokenizer)
    (setl ReturnValue (Expr_Operators))
    (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr_Operators at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
        
    )
    (set (. ReturnValue 'Lhs)(ParseExpr_Term tokenizer))
    (while (&& (. LOOKTable 25 0 :type (peek tokenizer 0))(. LOOKTable 25 1 :type (peek tokenizer 1)))
        (append (. ReturnValue 'Parts) (OperatorPart))
        (Fill_L3 (back (. ReturnValue 'Parts)) tokenizer)
        
    )
    (return ReturnValue)
    
)
(defun Fill_L3_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L4 at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected _L4"))
    

)
)
(defun Parse_L3_0 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L4 at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected _L4"))
    

)(return ReturnValue)

)
(defun Fill_L3 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L4 at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected _L4"))
    

)
)
(defun Parse_L3 (tokenizer)
    (setl ReturnValue (OperatorPart))
    (if (not (&& (. LOOKTable 6 0 :type (peek tokenizer 0))(. LOOKTable 6 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected Op"))
        
    )
    (setl (. ReturnValue 'Operator) (. (ParseOp tokenizer) 'Value))
    (if (&& (. LOOKTable 24 0 :type (peek tokenizer 0))(. LOOKTable 24 1 :type (peek tokenizer 1)))
        (if (not (&& (. LOOKTable 16 0 :type (peek tokenizer 0))(. LOOKTable 16 1 :type (peek tokenizer 1))))
            (error (+ "Error parsing _L4 at position"  (str :position (peek tokenizer 0)) ": expected Expr_Term"))
            
        )
        (set (. ReturnValue 'Rhs)(ParseExpr_Term tokenizer))
        
    
    else (error (+ "Error parsing _L3 at position"  (str :position (peek tokenizer 0)) ": expected _L4"))
    

)(return ReturnValue)

)
(defun FillExpr_0 (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun ParseExpr_0 (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    (return ReturnValue)
    
)
(defun FillExpr (ReturnValue tokenizer)
    (if (not (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    
)
(defun ParseExpr (tokenizer)
    (setl ReturnValue (Expr))
    (if (not (&& (. LOOKTable 21 0 :type (peek tokenizer 0))(. LOOKTable 21 1 :type (peek tokenizer 1))))
        (error (+ "Error parsing Expr at position"  (str :position (peek tokenizer 0)) ": expected Expr_Operators"))
        
    )
    (set ReturnValue(ParseExpr_Operators tokenizer))
    (return ReturnValue)
    
)
(set LOOKTable (list (list (list false true false false false false false false false) (list false false true true true true true true true) ) (list (list false true false false false false false false false) (list false false true true true true true true true) ) (list (list false true false false false false false false false) (list false false true true true false true true true) ) (list (list false true false false false false false false false) (list false false true true true false true true true) ) (list (list true false false false false false false false false) (list false false true true true false true true true) ) (list (list true false false false false false false false false) (list false false true true true false true true true) ) (list (list false false true true true false false false false) (list true true false false false true false false true) ) (list (list false false true false false false false false false) (list true true false false false true false false true) ) (list (list false false false true false false false false false) (list true true false false false true false false true) ) (list (list false false false false true false false false false) (list true true false false false true false false true) ) (list (list false true false false false false false false false) (list false false false false false true false false true) ) (list (list false true false false false false false false false) (list false false false false false true false false true) ) (list (list false false false false false false false true false) (list true true false false false true false false false) ) (list (list false false false false false false false true false) (list true true false false false true false false false) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list false false false false false true false false false) (list true true false false false true false false false) ) (list (list false true false false false false false false false) (list false false true true true false true true true) ) (list (list false true false false false false false false false) (list false false false false false true false false true) ) (list (list true false false false false false false false false) (list false false true true true false true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list false false true true true false false false false) (list true true false false false true false false true) ) (list (list false false true true true false false false false) (list true true false false false true false false true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) (list (list true true false false false true false false false) (list true true true true true true true true true) ) ))