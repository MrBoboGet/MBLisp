(import parsing)
(import json)
(import lsp)
(eval-lsp (load-parser MBPerl 2))


(defclass operator-nf ()
    (operator 'null)
    (args (list))
)

(set op-prec (make-dict 
    ("+" 5)
    ("-" 5)
    ("." 1)
    ("*" 4)
))


(defgeneric convert-expr)


(defun get-top-ops (op-list)
    (setl return-value (list))
    (setl max-prio 1000) #arbitrary af
    (setl top-op "")
    (doit i (range 0 (len op-list))
        (setl cur-op :Operator (. op-list i))
        (setl cur-prio (. op-prec cur-op))
        (if (< cur-prio max-prio)
            (setl max-prio cur-prio)
            (setl top-op cur-op)
        )
    )
    (doit i (range 0 (len op-list))
        (if (eq :Operator (. op-list i) top-op)
            (append return-value i)
        )
    )
    return-value
)


(defun split-operators (operators begin end)
    (setl return-value (Expr_Operators))
    (if (not (eq begin -1))
        (setl :Lhs return-value :Rhs (. :Parts operators begin))
     else 
        (setl :Lhs return-value :Lhs operators)
    )
    (incr begin 1)

    (doit i (range begin end)
        (if (eq i 0)
            (append :args return-value  :Lhs operators)
         else
            (append :args return-value :Rhs (. :Parts operators i))
        )
    )

    return-value
)
(defun convert-operators (operators)
    (setl return-value (operator-nf))
    (if (eq (len :Parts operators) 0)
        (return (convert-expr :Lhs operators))
    )
    (setl top-ops (get-top-ops :Parts operators))
    (setl :operator return-value (symbol :Operator :0 :Parts operators))
    (insert-at top-ops 0 -1)
    (insert-at top-ops (len top-ops) (len :Parts operators))
    (doit i (range 0 (+ (len top-ops) -1))
        (append :args return-value (convert-expr (split-operators operators (. top-ops i) (. top-ops (+ i 1)))))
    )

    #(set :operator return-value (symbol :Operator :0 :Parts operators))
    #(append :args return-value (convert-expr :Lhs operators))
    #(insert-elements :args return-value (map _(convert-expr :Rhs _) :Parts operators))
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
