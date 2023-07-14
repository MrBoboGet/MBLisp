(set quote-reader (lambda (stream) (list (quote quote) (read-term stream))))
(add-reader-character *READTABLE* "'" quote-reader)
(set while (macro (lambda (condition &rest body)
                    (list 'tagbody 'begin (list 'cond condition (flatten-1 'progn body (list (list 'go 'begin)) ) (list 'go 'end)) 'end true))
                    ))
(set continue (macro (lambda () (quote (go begin)))))
(set break (macro (lambda () (quote (go end)))))
(set backtick-read-list
     (lambda (stream) 
        (set return-value (list 'flatten-1))
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
                             (append return-value (list 'list (read-term stream))))
                     )
                     (cond (eq current-byte "(")
                           (append return-value (list 'list (backtick-read-list stream)))
                           (append return-value (list 'list (list 'quote (read-term stream)))))
               )
        )
        return-value
     ))
(set backtick-reader
     (lambda (stream) 
       (skip-whitespace stream)
       (cond (eq (peek-byte stream) "(")
                (backtick-read-list stream)
                (print "error in backtick-reader: first element has to be a ("))))
(add-reader-character *READTABLE* "`" backtick-reader)
(set defun (macro (lambda (name arglist &rest body) `(set ,name (lambda ,arglist ,@body)))))
(set defmacro (macro (lambda (name arglist &rest body) `(set ,name (macro (lambda ,arglist ,@body))))))
(defmacro incr (var increment)
     `(set ,var (+ ,var ,increment)))
(defmacro dotimes (count &rest body) 
   (set loop-variable (gensym))
  `(progn (set ,loop-variable 0)
    (while (< ,loop-variable ,count)
        ,@body
        (incr ,loop-variable 1))
   ))
(defmacro defgeneric (name)
  `(set ,name (generic)))
(defmacro defmethod (methodname overrides &rest body)
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
  `(addmethod ,methodname (list ,@type-overrides) (lambda (,@argument-symbols) ,@body))
)
(defmacro defclass (classname parents &rest slots)
    (set filtered-slots (list)) 
    (set constructors (list))
    (set i 0)
    (while (< i (len slots))
        (set current-list (index slots i))
        (cond (eq (index current-list 0) 'constructor)
              (append constructors (index current-list 1))
              (append filtered-slots current-list)
        )
        (incr i 1)
    )
    (cond (eq (len constructors) 0)
        `(set ,classname (class (list ,@parents)  (quote ,slots)))
        `(set ,classname (class (list ,@parents)  (quote ,filtered-slots) ,(index constructors 0)))
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
  (constructor (lambda (it list-to-it)  (set (index it 'it-list) list-to-it) it))
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
    (constructor 
      (lambda (it &rest args)
        (cond (eq (len args) 1)
              (set (slot it end) (index args 0))
              (progn 
                (set (slot it start) (minus (index args 0) 1))
                (set (slot it end) (index args 1))
              )
        )
        it
      ))
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
  `(progn (set ,it-var (iterator ,iterable))
          (while (next ,it-var)
                 (set ,boundvar (current ,it-var))
                 ,@body
          )
   )
)
(defun expand-&& (forms offset)
  (cond (eq (len forms) offset)
        'true
        `(cond ,(index forms offset) ,(expand-&& forms (+ offset 1)) false)
  )
)

(defmacro && (&rest forms)
    (expand-&& forms 0))

(defun expand-|| (forms offset)
  (cond (eq (len forms) offset)
        'false
        `(cond (not ,(index forms offset)) ,(expand-|| forms (+ offset 1)) true)
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

(defclass exception () (what "") (constructor (lambda (e message) (set (slot e what) message))))

(defun error (error-signal)
  (signal error-signal)
  (print (+ "Un-handled error: " error-signal))
)

(defun foldl (iterable callable)
  (set it (iterator iterable))
  (set result -1)
  (cond (next it) (set result (current it)) (error "foldl requires iterable to have atleast 1 element"))
  (while (next it)
    (set result (callable result (current it)))
  )
  result
)
(set in-top-curry true)
(set curry-args (list))
(defun curry-reader (stream)
  (cond in-top-curry
        (progn
          (set in-top-curry false)
          (set curry-term (read-term stream))
          (set in-top-curry true)
          (set ret-value `(lambda (,@curry-args) ,curry-term))
          (set curry-args (list))
          ret-value
        )
        (progn
          (set new-symbol (gensym))
          (append curry-args new-symbol)
          new-symbol
        )
  )
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
        (cond (&& (< (+ i 1) (len forms)) (eq (index forms (+ i 1)) 'if))
            (progn (set else-form (if-func (sublist forms (+ i 2))))  (break))
            (progn (set else-form `(progn ,@(sublist forms (+ i 1)))) (break))
        )
        (append if-body current-form)
    )
  )
  (cond (eq (len if-body) 0)
        (progn (print  "empty if body:") (print forms) (print (eq (index forms 0) 'else)))
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

(add-reader-character *READTABLE* "_" curry-reader)
(defun . (&rest args)
  (foldl args _(index _ _))
)
(defmacro defvar (name star-value)
  `(set ,name (dynamic ,star-value))
)
(load (+ (parent-path load-filepath) "/import.lisp"))


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
