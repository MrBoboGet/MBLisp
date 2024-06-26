(import parsing)
(import json)
(import lsp)
(eval-lsp (load-parser MBPerl 2))


(defclass operator-nf ()
    (operator 'null)
    (assignment false)
    (args (list))
)

(set op-prec (make-dict 
    ("+" 5)
    ("-" 5)
    ("." 1)
    ("*" 4)
    ("=" 20)
    ("==" 10)
    ("<" 9)
    ("<=" 9)
    (">=" 9)
    (">" 9)
))


(defgeneric convert-expr)


(defun get-top-ops (op-list)
    (setl return-value (list))
    (setl max-prio -1)
    (setl top-op "")
    (doit i (range 0 (len op-list))
        (setl cur-op :Value :Operator (. op-list i))
        (setl cur-prio (. op-prec cur-op))
        (if :Assignment :Operator (. op-list i)
            (setl cur-prio :"=" op-prec)
        )
        (if (> cur-prio max-prio)
            (setl max-prio cur-prio)
            (setl top-op cur-op)
        )
    )
    (doit i (range 0 (len op-list))
        (if (eq :Value :Operator (. op-list i) top-op)
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
    (setl top-op (. :Parts operators :0 top-ops))
    (setl :operator return-value (symbol :Value :Operator top-op))
    (setl :assignment return-value :Assignment :Operator top-op)
    (insert-at top-ops 0 -1)
    (insert-at top-ops (len top-ops) (len :Parts operators))
    (doit i (range 0 (+ (len top-ops) -1))
        (append :args return-value (convert-expr (split-operators operators (. top-ops i) (. top-ops (+ i 1)))))
    )

    #(set :operator return-value (symbol :Value :Operator :0 :Parts operators))
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
    `(,(convert-idf :Identifier expr) ,@(map convert-expr :Arguments expr))
)


(defclass regex ()
    (op "")
    (modifiers "")
    (parts (list))
)
(defmethod apply ((str string_t) (reg regex))
    "temporary"
)
(defmethod convert-expr ((expr Expr_Regex))
    (setl res (regex))
    (setl parts (split-quoted :Content expr "/" "\\"))
    (setl :op res :0 parts)
    (setl :modifiers res (. parts (+ (len parts) -1)))
    (doit i (range 1 (+ (len parts) -1))
        (append :parts res (. parts i))
    )
    res
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

    (if :assignment expr 
        (return (foldl :args expr _(progn `(setl ,_1 , `(,op ,_1   ,_2)))))
    )
    (if binary (return (foldl :args expr _(progn `(,op ,_1 ,_2)))))
    (append return-value op)
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

(defgeneric insert-if)
(defmethod insert-if ((out list_t) (content IfContent))
    (if :HasCondition content (append out (convert-idf :IfPart content)) 
            (append out (convert-expr :Condition content)))
    (setl res `(,@(map convert-statement :Body content)))
    (insert-elements out res)
)
(defmethod convert-statement ((stmt Statement_If))
    (setl res (list))
    (insert-if res :If stmt)
    (doit clause :ElseClauses stmt
        (append res 'else)
        (insert-if res clause)
    )
    res
)
(defmethod convert-statement ((stmt Statement_For))
    (setl var-sym (gensym))
    (if (&& (not (eq :Value :VarName stmt "")) (not (eq :Value :VarName stmt null)))
        (setl var-sym (convert-idf :VarName stmt))
    )
    `(doit ,var-sym ,(convert-expr :Enumerable stmt) ,@(map convert-statement :Body stmt))
)


(defmethod convert-statement ((stmt Statement_While))
    `(while ,(convert-expr :Condition stmt) ,@(map convert-statement :Body stmt)) 
)
(defmethod convert-statement ((stmt Statement_Expr))
    (convert-expr :Expr stmt)
)

(defun MBPerl (stream) 
    (setl current-tokenizer (get-tokenizer))
    (set-stream current-tokenizer stream)
    (set result (ParseStatementList current-tokenizer))
    `(progn ,@(map convert-statement :Statements result))
)
