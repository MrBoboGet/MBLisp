(set quote-reader (lambda (stream) (list (symbol (quote quote) -1) (read-term stream))))
(add-reader-character *READTABLE* "'" quote-reader)
(set slash-reader (lambda (stream) (list (symbol (quote quote) -1) (symbol (read-term stream) -1))))
(add-reader-character *READTABLE* "\\" slash-reader)


(set while (macro (lambda (condition &rest body)
                    (list \tagbody \begin (list \cond condition (flatten-1 \progn body (list (list \go \begin)) ) (list \go \end)) \end true))
                    ))
(set error (lambda (error-signal) (signal error-signal true)))
(set continue (macro (lambda () (quote (go begin)))))
(set break (macro (lambda () (quote (go end)))))

(set read-clean-term (lambda (stream) (set return-value (read-term stream)) 
        (cond (eq (type return-value) symbol_t) (set return-value (symbol return-value -1)) false)
    return-value)  
)

(set backtick-read-list
     (lambda (stream) 
        (set return-value (list \flatten-1))
        (read-byte stream)
        (while true
               (skip-whitespace stream)
               (cond (eof stream) (break) true)
               (set current-byte (peek-byte stream))
               (cond (eq current-byte ")") (progn (read-byte stream) (break)) true)
               (cond (eq current-byte ",")
                     (progn 
                       (read-byte stream)
                       (cond (eq (peek-byte stream) "@") 
                             (progn (read-byte stream) (append return-value (read-term stream)) )
                             (append return-value (list \list (read-term stream))))
                     )
                     (cond (eq current-byte "(")
                           (append return-value (list \list (backtick-read-list stream)))
                           (append return-value (list \list (list \quote (read-clean-term stream)))))
               )
        )
        return-value
     ))
(set backtick-reader
     (lambda (stream) 
       (skip-whitespace stream)
       (cond (eq (peek-byte stream) "(")
                (backtick-read-list stream)
                (error "error in backtick-reader: first element has to be a ("))))
(add-reader-character *READTABLE* "`" backtick-reader)
(set defun (macro (lambda (name arglist &rest body) `(progn (set ,name (lambda ,arglist ,@body)) (set-name ,name (quote ,name))))))
(set defmacro (macro (lambda (name arglist &rest body) `(progn (set ,name (macro (lambda ,arglist ,@body))) (set-name ,name (quote ,name))))))

(defmacro incr (var increment)
     `(set ,var (plus ,var ,increment)))
(defmacro dotimes (count &rest body) 
   (set loop-variable (gensym))
  `(progn (set ,loop-variable 0)
    (while (< ,loop-variable ,count)
        ,@body
        (incr ,loop-variable 1))
   ))
(defmacro defgeneric (name)
  `(progn (set ,name (generic)) (set-name ,name (quote ,name)))
)
(defmacro defmethod (methodname overrides &rest body &envir envir)
  (set type-overrides (list))
  (set argument-symbols (list))
  (set i 0)
  (while (< i (len overrides))
    (set current-argument (index overrides i))
    (cond (eq (type current-argument) list_t)
        (progn 
            (append argument-symbols (index (index overrides i) 0))
            (append type-overrides (index (index overrides i) 1))
        )
        (progn 
            (append argument-symbols current-argument)
            (append type-overrides any_t)
        )
    )
    (incr i 1)
  )
  (setl method-def 
      `(addmethod 
          ,methodname (list ,@type-overrides) 
          (set-name (lambda (,@argument-symbols) ,@body) (quote ,methodname))))
  (return `(progn (cond (in (quote ,methodname) (environment)) null (defgeneric ,methodname)) ,method-def))
  (cond (in methodname envir)
      method-def
      `(progn (defgeneric ,methodname) ,method-def)
  )
)
(defmacro defclass (classname parents &rest slots &envir envir)
    (setl filtered-slots (list)) 
    (setl i 0)
    (setl methods (list))
    (setl constructor-sym 'empty)
    (while (< i (len slots))
        (setl current-list (index slots i))
        (cond (< (len current-list) 3)
              (progn 
                  (setl (index current-list 1) (expand (index current-list 1)))
                  (append filtered-slots current-list)
              )
              (progn 
                  (setl new-method (list))
                  (append new-method 'defmethod)
                  (setl method-name (index current-list 0))
                  (cond (eq method-name 'constructor) (cond (eq constructor-sym 'empty) (progn (setl constructor-sym (gensym)) (set method-name constructor-sym)) (setl method-name constructor-sym)) null)
                  (append new-method method-name)
                  (append new-method (index current-list 1))
                  (insert-at (index new-method 2) 0 'this)
                  (setl j 2)
                  (while (< j (len current-list))
                     (append new-method (index current-list j))
                     (incr j 1)
                  )
                  (append new-method 'this)
                  (append methods new-method)
              )
        )
        (incr i 1)
    )
    (cond (eq constructor-sym 'empty)
        null
        (set (index envir constructor-sym) (generic))
    )
    (cond (eq constructor-sym 'empty)
        `(progn (set ,classname (class (list ,@parents)  (quote ,slots))) ,@methods (set-name ,classname (quote ,classname)))
        `(progn (defgeneric ,constructor-sym) (set ,classname (class (list ,@parents)  (quote ,filtered-slots) ,constructor-sym)) ,@methods (set-name ,classname (quote ,classname)))
    )
)

(defun not (var) 
  (cond var false true))

(defgeneric iterator)
(defgeneric next)
(defgeneric current)
(defmacro slot (object accessor) `(index ,object (quote ,accessor)))

(defclass list-iterator () 
  (it-list false)
  (current-offset -1)
  (constructor (list-to-it)  
      (set (index this 'it-list) list-to-it) 
  )
)
(defmethod iterator ((list-to-it list_t))
  (list-iterator list-to-it))
(defmethod next ((it list-iterator))
  (progn (incr (slot it current-offset) 1) 
         (< (slot it current-offset) (len (slot it it-list))  ))
)
(defmethod current ((it list-iterator))
  (index (slot it it-list) (slot it current-offset)))


(defclass range ()
    (start 0)
    (end -1)
    (constructor (last)
        (set (slot this end) last)
    )
    (constructor (begin last)
        (set (slot this start) (minus begin 1))
        (set (slot this end) last)
    )
)
(defmethod iterator ((it range))
    it
)
(defmethod current ((it range))
  (slot it start)
)
(defmethod next ((it range))
  (incr (slot it start) 1)
  (< (slot it start) (slot it end))
)


(defmacro doit (boundvar iterable &rest body)
  (set it-var (gensym))
  `(progn (setl ,it-var (iterator ,iterable))
          (while (next ,it-var)
                 (setl ,boundvar (current ,it-var))
                 ,@body
          )
   )
)
(defun expand-&& (forms offset)
  (cond (eq (len forms) offset)
        'true
        `(cond ,(index forms offset) ,(expand-&& forms (plus offset 1)) false)
  )
)

(defmacro && (&rest forms)
    (expand-&& forms 0))

(defun expand-|| (forms offset)
  (cond (eq (len forms) offset)
        'false
        `(cond (not ,(index forms offset)) ,(expand-|| forms (plus offset 1)) true)
  )
)
(defmacro || (&rest forms)
    (expand-|| forms 0))

(defun map (callable iterable)
    (set return-value (list))
    (doit v iterable
        (append return-value (callable v))
    )
    return-value
)
(defun zip (iterable1 iterable2)
    (set return-value (list))
    (set first-it (iterator iterable1))
    (set second-it (iterator iterable2))
    (while (&& (next first-it) (next second-it))
        (append return-value (list (current first-it) (current second-it)))
    )
    return-value
)
(defun filter (callable iterable)
    (set return-value (list))
    (doit v iterable
        (cond (callable v) (append return-value v) false)
    )
    return-value
)

(defclass exception () (what "") (constructor (message) (set (slot this what) message)))


(defun foldl (iterable callable)
  (set it (iterator iterable))
  (set result -1)
  (cond (next it) (set result (current it)) (error "foldl requires iterable to have atleast 1 element"))
  (while (next it)
    (set result (callable result (current it)))
  )
  result
)
(defun sublist (current-list offset)
  (set return-value (list))
  (doit i (range offset (len current-list))
    (append return-value (index current-list i))
  )
  return-value
)
(defun if-func (forms)
  (set clause (index forms 0))
  (set if-body (list))
  (set else-form (list))
  (doit i (range 1 (len forms))
    (set current-form (index forms i))
    (cond (eq current-form 'else)
        (cond (&& (< (plus i 1) (len forms)) (eq (index forms (plus i 1)) 'if))
            (progn (set else-form (if-func (sublist forms (plus i 2))))  (break))
            (progn (set else-form `(progn ,@(sublist forms (plus i 1)))) (break))
        )
        (append if-body current-form)
    )
  )
  (cond (eq (len if-body) 0)
        (error  "empty if body")
        false
  )
  (cond (eq (len else-form) 0)
      `(cond ,clause (progn ,@if-body) false)
      `(cond ,clause (progn ,@if-body) ,else-form)
  )
)
(defmacro if (&rest forms)
  (if-func forms) 
)

(defmacro defvar (name star-value)
  `(set ,name (dynamic ,star-value))
)


(defmacro make-dict (&rest forms)
  (set dict-var (gensym))
  (set progn-content (list `(set ,dict-var (dict))))
  (doit form forms
    (if (not (eq (type form) list_t))
      (error "make-dict requires every form to be a key-value pair in a list")
    )
    (if (not (eq (len form) 2))
      (error "make-dict requires every form to be a key-value pair in a list")
    )
    (append progn-content `(set (index ,dict-var ,(index form 0)) ,(index form 1)))
  )
  (append progn-content dict-var)
  `(progn ,@progn-content)
)
(defmethod in (value (list-to-check list_t))
    (doit v list-to-check
        (if (eq v value) (return true))
    )
    false
)

(defmacro catch-signals (try-body &rest catch-triplets)
  (set catch-parts (list))
  (set i 0)
  (while (< i (len catch-triplets))
    (if (eq (index catch-triplets i) 'catch)
      (set catched-values (index catch-triplets (plus i 1)))
      (set catch-body (index catch-triplets (plus i 2)))
      (set catch-body `(progn ,@catch-body))
      (append catch-parts catched-values)
      (append catch-parts catch-body)
      (incr i 2)
     else
       (error "Non catch-body in try form")
    )
    (incr i 1)
  )
  `(signal-handlers (progn ,@try-body)  ,@catch-parts)
)

(defmacro try  (try-body &rest catch-triplets)
  (set catch-parts (list))
  (set i 0)
  (while (< i (len catch-triplets))
    (if (eq (index catch-triplets i) 'catch)
      (set catched-values (index catch-triplets (plus i 1)))
      (set catch-body (index catch-triplets (plus i 2)))
      (set catch-body `(progn ,@catch-body (unwind))) 
      (append catch-parts catched-values)
      (append catch-parts catch-body)
      (incr i 2)
     else
       (error "Non catch-body in try form")
    )
    (incr i 1)
  )
  `(signal-handlers (progn ,@try-body)  ,@catch-parts)
)

(defmacro catch-all (&rest body)
    `(try (,@body) catch (any_t e) (true))
)
(defmacro ignore-signals (&rest body)
    `(signal-handlers (progn ,@body) (any_t e) null)
)


(defmacro let (bindings &rest body)
    (set triplets (list))
    (doit pair bindings
        (append triplets `( (environment) (quote ,(index pair 0)) ,(index pair 1)))
    )
    `(bind-dynamic ( ,@ triplets )  (progn ,@body))
)



(defvar in-top-curry true)
(defvar arg-dict (dict))

(defun curry-reader (stream)
  (if in-top-curry
      (let ((in-top-curry false) (arg-dict (dict)))
          (set curry-term (read-term stream))
          (set ret-value `( ,\lambda (,@(map (lambda (x) (index arg-dict x)) (sort (keys arg-dict)) )) ,curry-term))
      )
      ret-value
   else
      (set current-sym "_")
      (set next-byte (peek-byte stream))
      (if (not (|| (eq next-byte " ") (eq next-byte ")")))
        (incr current-sym (read-byte stream))
      )
      (if (in current-sym arg-dict)
        (set current-sym (index arg-dict current-sym))
       else 
        (set (index arg-dict current-sym) (gensym))
        (set current-sym (index arg-dict current-sym))
      )
      current-sym
   )
)
(add-reader-character *READTABLE* "_" curry-reader)

(defmacro . (&rest args)
  (foldl args _`(index ,_ ,_1))
)
(defmacro + (&rest args)
  (foldl args _`(plus ,_ ,_1))
)
(defmacro * (&rest args)
  (foldl args _`(times ,_ ,_1))
)
(defmacro ++ (value increase)
    (set temp-sym (gensym))
    `(progn
        (set ,temp-sym ,value)
        (incr ,value ,increase)
        ,temp-sym
     )
)
(load (plus (parent-path load-filepath) "/import.lisp") true)

(defun leq (lhs rhs)
    (|| (< lhs rhs) (eq lhs rhs))
)
(defun geq (lhs rhs)
    (not (< lhs rhs))
)
(defun ge (lhs rhs)
    (not (leq lhs rhs))
)

(defun vectorize-comparison (comparison args)
    (if (eq (len args) 2)
        `(,comparison ,(. args 0) ,(. args 1))
     else 
        (set list-sym (gensym))
        (set it-sym (gensym))
        (set result-sym (gensym))
        `(progn (set ,result-sym true)
                (set ,list-sym (list ,@args))
                (doit ,it-sym (range 0 ,(plus (len args) -1))
                    (if (not (,comparison (. ,list-sym ,it-sym) (. ,list-sym (plus ,it-sym 1))))
                        (set ,result-sym false)
                        (break)
                    )
                )
                ,result-sym)
    )
)
(defun > (lhs rhs)
    (ge lhs rhs)
)
(defun <= (lhs rhs)
    (leq lhs rhs)
)
(defun >= (lhs rhs)
    (geq lhs rhs)
)
(defun => (lhs rhs)
    (geq lhs rhs)
)
(defun =< (lhs rhs)
    (leq lhs rhs)
)

(defmacro dowhile (condition &rest body)
    `(progn 
        (progn ,@body)
        (while ,condition
            ,@body
        )
     )
)

(defun member-reader (stream)
    (set sym (read-term stream))
    (set form (read-term stream))
    (if (is (type sym) symbol_t)
        `(index ,form (quote ,sym))
    else 
        `(index ,form ,sym)
    )
)
(defun index-reader (stream)
    (set sym (read-term stream))
    (set form (read-term stream))
    `(index ,form ,sym)
)
(add-reader-character *READTABLE* ":" member-reader)
(add-reader-character *READTABLE* ";" index-reader)

(defun comment-reader (stream)
    (read-line stream)
    null
)
(add-reader-character *READTABLE* "#" comment-reader)

(defun stream-reader (stream &envir envir)
    (setl return-value null)
    (setl sym (read-symbol stream))
    (if (in sym envir)
        (setl reader (index envir sym))
        (setl return-value (reader stream))
     else
        (error "stream-reader needs a symbol taking a callable as the first argument")
    )
    return-value
)
(add-reader-character *READTABLE* "@" stream-reader)


(defgeneric max)

(defmethod max (lhs rhs)
    (if (< lhs rhs) rhs else lhs)
)
(defmethod max ((list list_t))
    (if (eq (len list) 0) (return null))
    (setl max-elem :0 list)
    (doit i (range 1 (len list))
        (setl current-elem (. list i))
        (if (< max-elem current-elem) (setl max-elem current-elem))
    )
    current-elem
)
(defgeneric min)

(defmethod min (lhs rhs)
    (if (< lhs rhs) lhs else rhs)
)
(defmethod min ((list list_t))
    (if (eq (len list) 0) (return null))
    (setl min-elem :0 list)
    (doit i (range 1 (len list))
        (setl current-elem (. list i))
        (if (< current-elem min-elem) (setl min-elem current-elem))
    )
    current-elem
)

(defmethod str ((list list_t))
    (set return-value "(")
    (doit i (range 0 (len list))
        (append return-value (str (index list i)))
        (if (< (+ i 1) (len list)) (append return-value " "))
    )
    (append return-value ")")
    return-value
)
(defmethod eq ((lhs list_t) (rhs list_t))
    (if (not (eq (len lhs) (len rhs)))
        (return false)
    )
    (set return-value true)
    (doit i (range 0 (len lhs))
        (if (not (eq (. lhs i) (. rhs i))) (return false))
    )
    return-value
)
(load (plus (parent-path load-filepath) "/macros.lisp") true)
