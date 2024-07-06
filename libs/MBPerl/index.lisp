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
    ("[" 2)
    ("*" 4)
    ("=" 20)
    ("==" 10)
    ("<" 9)
    ("<=" 9)
    (">=" 9)
    (">" 9)
    ("=~" 8)
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
    #extra args always implies left associative operator whose extra arguments should be apended
    #to the individual parts
    (setl binary (> (len :ExtraArgs :Operator top-op) 0))
    (doit i (range 0 (+ (len top-ops) -1))
        (setl begin (. top-ops i))
        (setl end (. top-ops (+ i 1)))
        #incredibly complicated if-statement, supporting these kind of 
        #behaviour that differs depending on the context it appears in is a bit difficult
        (if (&& (eq (+ end (* begin -1)) 1) 
                (eq :operator return-value '=~) 
                (not (eq begin -1)) 
                (is (type :Rhs (. :Parts operators begin)) Expr_Regex))
                 (append :args return-value (convert-regex :Rhs (. :Parts operators begin)))
         else
            (if (|| (not binary) (eq begin -1))
                (append :args return-value 
                    (convert-expr (split-operators operators begin end)))
             else
                (setl new-ret (operator-nf))
                (set :operator new-ret :operator return-value)
                (set :assignment new-ret :assignment return-value)
                (insert-elements :args return-value (map convert-expr :ExtraArgs :Operator (. :Parts operators begin)))
                (setl new-lhs (convert-nf return-value))
                (if (eq (+ i 2) (len top-ops))
                    (return new-lhs)
                )
                (append :args new-ret new-lhs)
                (setl return-value new-lhs)
            )
        )
    )
    return-value
)


(defun convert-idf (idf)
    (setl result (symbol (symbol :Value idf) :Position idf (path-id load-filepath)))
    result
)
(defun convert-location (pos)
    (setl result (symbol (symbol "") pos (path-id load-filepath)))
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
    (setl func-name (convert-idf :Identifier expr))
    (setl res `(,func-name ,@(map convert-expr :Arguments expr)))
    (set-loc res func-name)
    res
)


(defclass RegexExpr ()
    (op "")
    (regex "")
    (modifiers "")
    (parts (list))
)
(defmethod assign ((lhs string_t) (rhs string_t))
    (clear lhs)
    (append lhs rhs)
    lhs
)
(defmethod apply ((input string_t) (reg RegexExpr))
    (if (|| (eq :op reg "") (eq :op reg "m"))
        (matching :regex reg input)
     else if (eq :op reg "s")
        (assign input (substitute :regex reg input :0 :parts reg))
     else if (eq :op reg "d")
        (assign input (substitute :regex reg input ""))
     else if (eq :op reg "e")
        (match :regex reg input "")
    )
)
(defmethod convert-regex ((expr Expr_Regex))
    (setl res (RegexExpr))
    (setl parts (split-quoted :Content expr "/" "\\"))
    (setl :op res :0 parts)
    (setl :regex res (regex :1 parts))
    (setl :modifiers res (. parts (+ (len parts) -1)))
    (doit i (range 2 (+ (len parts) -1))
        (append :parts res (. parts i))
    )
    res
)
(defmethod convert-expr ((expr Expr_Regex))
    (setl res (convert-regex expr))
    (if (|| (eq :op res "s") (eq :op res "d"))
        `(set $_ (apply $_ ,res))
    else
        `(apply $_ ,res)
    )
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
    ((symbol "[") '.)
))
(set binary-ops (make-dict 
    ('= true)
    ('== true)
))
(set $_ (dynamic ""))

(defmethod get-location ((expr list_t))
    (get-loc expr)
)
(defmethod get-location ((expr symbol_t))
    expr
)
(defmethod get-location ((expr any_t))
    (symbol "")
)
(defmethod convert-nf ((expr operator-nf))
    (setl return-value (list))
    (if (eq :operator expr '.)
        (return (convert-nf-dot expr))
    )
    (setl op :operator expr)
    (setl binary (in op binary-ops))
    (if (in op op-map) (setl op (. op-map op)))

    (if (eq :operator expr '=~)
        (return (foldl :args expr _(progn `(apply ,_1   ,_2))))
    )
    (if :assignment expr 
        (return (foldl :args expr _(progn `(setl ,_1 , `(,op ,_1   ,_2)))))
    )
    (if binary (return (foldl :args expr _(progn `(,op ,_1 ,_2)))))
    (append return-value op)
    (doit arg :args expr
        (append return-value arg)
    )
    return-value
)
(defmethod convert-expr ((expr Expr_String))
   (setl res `(progn ,(substr :Content expr 1 (+ (len :Content expr ) -2))))
   (set-loc res (convert-location :Begin expr))
   res
)
(defmethod convert-expr ((expr Expr_Operators))
    (setl nf (convert-operators expr))
    (setl res (convert-nf nf))
    (if (is (type nf) operator-nf)
        (set-loc res (get-location :0 :args nf))
    )
    res
)
(defmethod convert-expr ((expr Expr_List))
    (setl res `(list ,@(map convert-expr :Values expr)))
    (set-loc res (convert-location :Begin expr))
    res
)
(defmethod convert-expr ((expr Expr_Dict))
    (setl res 
        `(make-dict ,@(map _(progn `(,(convert-expr :Key _) ,(convert-expr :Value _)))  :Values expr)))
    (set-loc res (convert-location :Begin expr))
    res
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
    (setl if-sym (convert-idf :IfPart :If stmt))
    (insert-if res :If stmt)
    (doit clause :ElseClauses stmt
        (append res 'else)
        (insert-if res clause)
    )
    (set-loc res if-sym)
    res
)
(defmethod convert-statement ((stmt Statement_For))
    (setl var-sym (gensym))
    (setl empty-var true)
    (setl for-sym (convert-idf :ForPart stmt))
    (signal (semantic-token (convert-idf :ForPart stmt) "macro"))
    (if (&& (not (eq :Value :VarName stmt "")) (not (eq :Value :VarName stmt null)))
        (setl var-sym (convert-idf :VarName stmt))
        (setl empty-var false)
    )
    (setl res null)
    (if (not empty-var)
        (setl res `(doit ,var-sym ,(convert-expr :Enumerable stmt) ,@(map convert-statement :Body stmt)))
     else
        (setl res `(doit ,var-sym ,(convert-expr :Enumerable stmt) 
            (let (($_ ,var-sym)) ,@(map convert-statement :Body stmt))))
    )
    (set-loc res for-sym)
    res
)


(defmethod convert-statement ((stmt Statement_While))
    (setl while-sym (convert-idf :WhilePart stmt))
    (signal (semantic-token  while-sym "macro"))
    (setl res `(while ,(convert-expr :Condition stmt) ,@(map convert-statement :Body stmt)) )
    (set-loc res while-sym)
    res
)
(defmethod convert-statement ((stmt Statement_Return))
    (setl return-sym (convert-idf :ReturnPart stmt))
    (setl res `(,return-sym  ,(convert-expr :Value stmt)))
    (set-loc res return-sym)
    res
)
(defmethod empty ((idf Idf))
    (|| (eq :Value idf null) (eq :Value idf ""))
)
    
(defmethod convert-statement ((stmt Statement_Func))
    (setl func-sym (convert-idf :FuncPart stmt))
    (setl res `(defmethod ,(convert-idf :Name stmt) 
        ,(map _(if (not (empty :Type _)) (list (convert-idf :Name _) (convert-idf :Type _) ) else 
            (convert-idf :Name _)) :Args stmt) ,@(map convert-statement :Content stmt)))
    (set-loc res func-sym)
    res
)
    
(defmethod convert-statement ((stmt Statement_Expr))
    (convert-expr :Expr stmt)
)

(defun MBPerl (stream) 
    (setl current-tokenizer (get-tokenizer))
    (set-stream current-tokenizer stream)
    (setl result (ParseStatementList current-tokenizer))
    `(progn ,@(map convert-statement :Statements result))
)
