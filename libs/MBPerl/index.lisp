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
    ("=" 20)
    ("==" 10)
))


(defgeneric convert-expr)


(defun get-top-ops (op-list)
    (setl return-value (list))
    (setl max-prio -1)
    (setl top-op "")
    (doit i (range 0 (len op-list))
        (setl cur-op :Operator (. op-list i))
        (setl cur-prio (. op-prec cur-op))
        (if (> cur-prio max-prio)
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
        (append :Parts return-value (. :Parts operators i))
    )
    return-value
)
(defun convert-operators (operators)
    (setl return-value (operator-nf))
    (if (eq (len :Parts operators) 0)
        (return (convert-expr :Lhs operators))
    )
    (setl top-ops (get-top-ops :Parts operators))
    (setl :operator return-value (symbol :Operator (. :Parts operators :0 top-ops)))
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
    (setl result (symbol (symbol :Value idf) :Position idf (path-id load-filepath)))
    result
)

(defmethod convert-expr ((expr Expr_Bool))
    :Value expr
)

(defmethod convert-expr ((expr Expr_Idf))
    (convert-idf :Identifier expr)
)
(defmethod convert-expr ((expr Expr_Num))
    :Value expr
)

(defmethod convert-expr ((expr Expr_FuncCall))
    `(,(convert-idf :Identifier expr) ,@(map _(convert-expr _) :Arguments expr))
)

(defgeneric convert-nf)
(defmethod convert-nf ((expr any_t))
    expr
)

(defun convert-nf-dot (expr)
    (setl return-value (list '%>%))
    (append return-value :0 :args expr)
    (doit i (range 1 (len :args expr))
        (setl cur-arg (. :args expr i))
        (if (eq (type cur-arg) symbol_t)
            (append return-value `(. ,cur-arg))
         else
            (append return-value cur-arg)
        )
    )
    return-value
)
(defun convert-nf-eq (expr)
    (setl return-value (list))

    return-value
)
(set special-ops (make-dict 
    ('. convert-nf-dot)
))
(set op-map (make-dict 
    ('= 'setl)
    ('== 'eq)
))
(set binary-ops (make-dict 
    ('= true)
    ('== true)
))
(defmethod convert-nf ((expr operator-nf))
    (setl return-value (list))
    (if (eq :operator expr '.)
        (return (convert-nf-dot expr))
    )
    (setl op :operator expr)
    (setl binary (in op binary-ops))
    (if (in op op-map) (setl op (. op-map op)))
    (append return-value op)
    (if binary (return (foldl :args expr _(progn `(,op ,_1 ,_2)))))
    (doit arg :args expr
        (append return-value (convert-nf arg))
    )
    return-value
)

(defmethod convert-expr ((expr Expr_Operators))
    (setl nf (convert-operators expr))
    (convert-nf nf)
)
(defgeneric vector-apply)
(defmethod vector-apply (op (lhs list_t) rhs)
    (setl return-value (list))
    (doit e lhs
        (append return-value (op e rhs))
    )
    return-value
)
(defmethod vector-apply (op lhs (rhs list_t))
    (setl return-value (list))
    (doit e rhs
        (append return-value (op lhs e))
    )
    return-value
)
(defmethod vector-apply (op (lhs list_t) (rhs list_t))
    (setl return-value (list))   
    (if (&& (eq (len lhs) 0) (eq (len rhs) 0)) (return (list)))
    (setl lhs-i 0)
    (setl rhs-i 0)
    (setl max-i (max (len lhs) (len rhs)))
    (while (not (|| (eq lhs-i max-i) (eq rhs-i max-i)))
        (if (eq lhs-i (len lhs)) (setl lhs-i 0))
        (if (eq rhs-i (len rhs)) (setl rhs-i 0))
        (append return-value (op (. lhs lhs-i) (. rhs rhs-i)))
        (incr lhs-i 1)
        (incr rhs-i 1)
    )
    return-value
)

(defmacro vectorise (op)
    `(progn 
        (defmethod ,op ((lhs list_t) rhs)
            (,vector-apply ,op lhs rhs)
        )
        (defmethod ,op (lhs  (rhs list_t))
            (,vector-apply ,op lhs rhs)
        )
        (defmethod ,op ((lhs list_t)  (rhs list_t))
            (,vector-apply ,op lhs rhs)
        )
     )
)
(vectorise plus)
(vectorise minus)
(vectorise times)

(defgeneric convert-statement)

(defmethod convert-statement ((stmt Statement_If))
    `(progn)
)
(defmethod convert-statement ((stmt Statement_For))
    `(progn)
)
(defmethod convert-statement ((stmt Statement_While))
    `(progn)
)
(defmethod convert-statement ((stmt Statement_Expr))
    (convert-expr :Expr stmt)
)

(defun MBPerl (stream) 
    (setl current-tokenizer (get-tokenizer))
    (set-stream current-tokenizer stream)
    (set result (ParseStatementList current-tokenizer))
    `(progn ,@(map _(convert-statement _) :Statements result))
)
