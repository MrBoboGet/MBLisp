(import io)
(defclass test-result ()
    (valid true)
    (location null)
    (error-string "")
)

(defclass module ()
    (name "default")
    (results (list))
    (constructor (lambda (this name) (set :name this name) this))
)
(set total-modules (list))
(set *current-module* (dynamic (module "default")))


(defun str-except (error)
    (if (applicable str error)
        (str error)
     else 
        "unknown reason"
    )
)

(defun get-assert-form (con con-string val1 val2)
    (set val1-string (str val1))
    (set val2-string (str val2))
    (set result-sym (gensym))
    `(progn 
        (set ,result-sym (,test-result))
        (try 
         (
           (if (not (,con ,val1 ,val2))
                (set (. ,result-sym 'valid) false)
                (set (. ,result-sym 'error-string) 
                     (+ ,val1-string " " ,con-string " " ,val2-string))
           )
           
         )
         catch (any_t error)
         (
            (set (. ,result-sym 'valid) false)
            (set (. ,result-sym 'error-string)
                 (+ "exception thrown: " (,str-except error)))
         )
        )
        (add-result ,result-sym)
    )
)

(defun add-result (result)
    (append :results *current-module* result)
)

(defmacro assert_eq (val1 val2)
    (get-assert-form eq "not equal to" val1 val2)
)

(defmacro assert_not_eq (val1 val2)
    (get-assert-form _(not (eq _1 _2)) "was equal to" val1 val2)
)

(defmacro assert_>= (val1 val2)
    (get-assert-form >= "not greater or equal to" val1 val2)
)

(defmacro assert_<= (val1 val2)
    (get-assert-form <= "was greater than" val1 val2)
)

(defmacro assert_< (val1 val2)
    (get-assert-form < "was greater or equal to" val1 val2)
)

(defmacro assert_> (val1 val2)
    (get-assert-form > "was lesser than" val1 val2)
)

(defun finalize-current-module ()
    (if (not (eq (len :results *current-module*) 0))
        (append total-modules *current-module*)
    ) 
)

(defun new-module (module-name)
    (finalize-current-module)
    (set *current-module* (module module-name))
)

(set *testing* (dynamic false))

(defun display-result ()
    (if (not *testing*)
        (finalize-current-module)
        (set total-tests 0)
        (set total-passed 0)
        (set module-print-string (list))
        (doit module total-modules
            (set current-test-size (len :results module))
            (set passed (foldl (map _(cond :valid _ 1 0) :results module)   plus))
            (incr total-tests current-test-size)
            (incr total-passed passed)
            (if (eq passed current-test-size)
                (append module-print-string (+ :name module " all tests passed (" (str current-test-size) "/" (str current-test-size) ")"))
             else 
                (set error-string (+ :name module " failed some tests (" (str passed) "/" (str current-test-size) "):\n"))
                (doit result :results module
                    (if (not :valid result)
                        (append error-string (+ :error-string result "\n"))
                    )
                )
                (append module-print-string error-string)
            )
        )
        (print (+ "Passed (" (str total-tests) "/" (str total-passed) ")"))
        (doit string module-print-string 
            (print string)
        )
    )
)
(defun test-dir ()
    (let ((*testing* true))
        (doit file (dir-it (parent-path load-filepath))
            (print load-filepath)
            (if (&& (eq (extension file) ".lisp") (not (eq (canonical load-filepath) (canonical file))))
                (eval `(load ,file) (new-environment))
            )
        )
    )
    (print *testing*)
    (display-result)
)
