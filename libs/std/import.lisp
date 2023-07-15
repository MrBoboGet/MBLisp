(set user-libs-path (+ (user-home-dir) "/.mblisp/libs"))
(set user-libs (list-dir user-libs-path))
(defun module-specifier-to-string (module-symbol)
  (+ "/" (foldl (split (str module-symbol) ".") _(+ _ "/"  _)))
)
(defmacro import (value &rest binding)
  (set file-to-load "")
  (if (eq (type value) symbol_t)
    (set module-path (module-specifier-to-string value))
    (if (exists (+ user-libs-path module-path))
        (set file-to-load (+ user-libs-path module-path))
     else if (exists (+ (cwd) module-path))
        (set file-to-load (+ (cwd) module-path))
     else 
        (error (+ "Cannot find module in working directory or user libs: " (str value)))
    )
   else if (eq (type value) string_t)
      (set file-to-load (+ (cwd) (str value)))
   else 
      (error "First argument to import has to be either a string or a symbol")
  )
  (if (is-directory file-to-load)
    (set file-to-load (+ file-to-load "/index.lisp"))
  )
  (if (not (in ".lisp" file-to-load))
     (set file-to-load (+ file-to-load ".lisp")) 
  )
  (set scope-to-eval '(environment))
  (if (eq (len binding) 0)
    `(eval (load ,file-to-load))
   else
    (set scope-sym (index binding 0))
    `(progn
        (set ,scope-sym (environment))
        (eval (load ,file-to-load) ,scope-sym)
     )
  )
)
(defun expand-sym-string (symbol-to-expand)
   (set symbol-parts (map symbol (split (str symbol-to-expand) ":")))
   (if (eq (len symbol-parts) 1)
     (return symbol-to-expand)
   )
   (set symbol-symbols (list))
   (doit i (range 1 (len symbol-parts))
         (append symbol-symbols `(quote ,(index symbol-parts i)))
   )
  `(. ,@(index symbol-parts 0) ,@symbol-symbols)
)
(add-character-expander *READTABLE* ":" expand-sym-string)
