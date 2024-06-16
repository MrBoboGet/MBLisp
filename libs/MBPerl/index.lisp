(import parsing)
(import json)
(import lsp)
(eval-lsp (load-parser MBPerl 2))


(defclass operator-nf ()
    (operator 'null)
    (args (list))
)


(defgeneric convert-expr)


(defun convert-operators (operators)
    (setl return-value (operator-nf))
    (if (eq (len :Parts operators) 0)
        (return (convert-expr :Lhs operators))
    )
    (set :operator return-value (symbol :Operator :0 :Parts operators))
    (append :args return-value (convert-expr :Lhs operators))
    (insert-elements :args return-value (map _(convert-expr :Rhs _) :Parts operators))
    return-value
)

(defun convert-idf (idf)
    (setl result (symbol :Value idf))
    result
)

(defmethod convert-expr ((expr Expr_Idf))
    (convert-idf :Identifier expr)
)

(defmethod convert-expr ((expr Expr_FuncCall))
    `(,(convert-idf :Identifier expr) ,@(map _(print (convert-expr _)) :Arguments expr))
)

(defgeneric convert-nf)
(defmethod convert-nf ((expr any_t))
    expr
)
(defmethod convert-nf ((expr operator-nf))
    (setl return-value (list))
    (append return-value :operator expr)
    (doit arg :args expr
        (append return-value (convert-nf arg))
    )
    return-value
)

(defmethod convert-expr ((expr Expr_Operators))
    (setl nf (convert-operators expr))
    (convert-nf nf)
)



(defun MBPerl (stream) 
    (setl current-tokenizer (get-tokenizer))
    (set-stream current-tokenizer stream)
    (set result (ParseExpr_Operators current-tokenizer))
    (print (to-json-string result))
    (convert-expr result)
)
