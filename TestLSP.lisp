(import lsp lsp)
(set handler (lsp:create-lsp-server))
(defun insert-elements (out-list in-list)
  (doit e in-list
    (append out-list e)
  )
)

(defun test-func () (print "hello world"))
(defun if-token-extractor (envir ast)
   (set return-value (list)) 
   (doit e ast
        (if (&& (eq (type e) symbol_t) (|| (eq (str e) "if") (eq (str e) "else")))
            (append return-value (list e "macro"))
         else 
            (insert-elements return-value (default-extractor envir e))
        )
   )
   return-value
)
(defun if-diagnostics (envir ast)
  (set return-value (list))
  (doit e ast
    (if (is-trivial-set-form e)
      (set (index envir (. e 1)) null)
    )
    (if (not (|| (eq e 'if) (eq e 'else)))
      (insert-elements return-value (get-diagnostics envir  e))
    )
  )
  return-value
)

(defun try-token-extractor (envir ast)
   (set return-value (list)) 
   (doit e ast
        (if (&& (eq (type e) symbol_t) (|| (eq e 'catch) (eq e 'catch)))

            (append return-value (list e "macro"))
         else 
            (insert-elements return-value (default-extractor envir e))
        )
   )
   return-value
)
(defun try-diagnostics (envir ast)
  (set return-value (list))
  (set i 1)
  (while (< i (len ast))
    (set e (. ast i))
    (if (is-trivial-set-form e)
      (set (index envir (. e 1)) null)
    )
    (if (eq e 'catch)
      (set catch-envir (new-environment))
      (set-parent catch-envir envir)
      (shadow catch-envir (. (. ast (+ i 1)) 1))
      (insert-elements return-value (get-diagnostics catch-envir  (. ast (+ i 1))))
      (insert-elements return-value (get-diagnostics catch-envir  (. ast (+ i 2))))
      (incr i 2)
     else 
      (insert-elements return-value (get-diagnostics envir  e))
    )
    (incr i 1)
  )
  return-value
)


(defun all (iterable)
    (set return-value true)
    (doit e iterable 
        (if (not e)
            (return false)
        )
    )
    return-value
)
(defun is-literal (ast)
    (|| (type-eq ast int_t) (type-eq ast string_t) (type-eq ast float_t)
        (&& (type-eq ast list_t) (< 0 (len ast)) (eq (. ast 0) 'quote)))
)

(defun is-compile-time-dot (envir ast)
  (set return-value false)
  (if (&& (< 2 (len ast)) (type-eq (. ast 1) symbol_t))
      (set base (. ast 1))
      (if (&& (in base envir) (not (type-eq (. envir base) null_t)))
        (set return-value (all (map _(is-literal (. ast _)) (range 2 (len ast)))))
      ) 
  ) 
  return-value
)




(defun dot-token-extractor (envir ast)
   (set return-value (list)) 
   (if (is-compile-time-dot envir ast)
        (set map-like (. envir (. ast 1)))
        (insert-elements return-value (default-extractor envir (. ast 1)))
        (doit e (map _(eval (. ast _)) (range 2 (len ast)))
            (try
             (
                (insert-elements return-value (default-extractor map-like e))
                (set map-like (. map-like e))
             )
             catch (any_t except)
             (

             )
            )
        )
    else 
        (doit e ast
            (insert-elements return-value (default-extractor envir e))
        )
   )
   return-value
)


(defun dot-diagnostics (envir ast)
  (set return-value (list))
  return-value
)

(defun type-eq (lhs rhs)
    (eq (type lhs) rhs)
)

(set overriden-extractors (make-dict 
                            ('if if-token-extractor)
                            ('try try-token-extractor)
                            ('. dot-token-extractor)
))

(defun default-extractor (envir ast)
    (set return-value (list))
    (if (eq (type ast) symbol_t)
      (if (< (position ast) 0)
        (return return-value)
      ) 
      (if (in ast envir)
        (set value (index envir ast))
        (if (eq (type value) macro_t)
          (append return-value (list ast "macro"))
          (signal (symbol-location ast (name value)))
         else if (eq (type value) function_t)
          (append return-value (list ast "function"))
         else if (eq (type value) lambda_t)
          (append return-value (list ast "function"))
          (signal (symbol-location ast (name value)))
         else if (eq (type value) generic_t)
          (append return-value (list ast "function"))
          (signal (symbol-location ast (name value)))
         else if (eq (type value) type_t)
          (append return-value (list ast "class"))
          (signal (symbol-location ast (name value)))
         else 
          (append return-value (list ast "var"))
        )
       else if (is-special ast)
        (append return-value (list ast "macro"))
       else 
        (append return-value (list ast "var"))
      )
     else if (eq (type ast) list_t)
       (if (not (< (len ast) 1))
           (set head (index ast 0))
           (if (&& (eq (type head) symbol_t) (in head overriden-extractors))
             (return ((index overriden-extractors head) envir ast))
           )
       )

       (doit sub-form ast
         (insert-elements return-value (default-extractor envir sub-form))
       )
    )  
    return-value
)

(defun quote-diagnostics (envir ast)
    (list)
)

(defun catch-diagnostics (envir ast)
  (set return-value (list))
  (doit e ast
    (if (is-trivial-set-form e)
      (set (index envir (. e 1)) null)
    )
    (if (not (|| (eq e 'if) (eq e 'else)))
      (insert-elements return-value (get-diagnostics envir  e))
    )
  )
  return-value
)

(defun func-envir (envir ast)
  (set return-value (new-environment))
  (set-parent return-value envir)
  (doit arg (. ast 2)
    (if (eq (type arg) symbol_t) (shadow return-value arg))
  )
  return-value
)
(defun lambda-envir (envir ast)
  (set return-value (new-environment))
  (set-parent return-value envir)
  (doit arg (. ast 1)
    (if (eq (type arg) symbol_t) (shadow return-value arg))
  )
  return-value
)

(defun method-envir (envir ast)
  (set return-value (new-environment))
  (set-parent return-value envir)
  (doit arg (. ast 2)
    (if (eq (type arg) symbol_t) (shadow return-value arg))
    (if (eq (type arg) list_t) (shadow return-value (. arg  0)))
  )
  return-value
)
(defun doit-envir (envir ast)
  (set return-value (new-environment))
  (set-parent return-value envir)
  (shadow return-value (index ast 1))
  return-value
)
(set envir-modifiers (make-dict 
                        ('defun func-envir) 
                        ('defmacro func-envir) 
                        ('lambda lambda-envir) 
                        ('defmethod method-envir) 
                        ('doit doit-envir)
))

(set diagnostics-overrides (make-dict 
                        ('if if-diagnostics) 
                        ('quote quote-diagnostics)
                        ('try try-diagnostics)
))


(defun get-diagnostics (envir ast)
  (set return-value (list)) 
  (if (eq (type ast) symbol_t)
    (if (&& (not (is-special ast)) (not (in ast envir)) (not (< (position ast) 0) ))
      (append return-value (list ast (+ (+ "Couldn't find symbol " (str ast)) " in current environment")))
    )
   else if (eq (type ast) list_t)
    (if (not (< (len ast) 1))
      (set head (. ast 0))
      (if (eq (type head) symbol_t)
        (if (in head diagnostics-overrides)
          (return ((. diagnostics-overrides head) envir ast))
         else if (in head envir-modifiers)
          (set envir ((. envir-modifiers head) envir ast))
        )
      )
    )
    (doit e ast
      (if (is-trivial-set-form e)
        (set (index envir (. e 1)) null)
      )
      (insert-elements return-value (get-diagnostics envir  e))
    )
  )
  return-value
)

(defun should-execute-form (form)
  (if (not (eq (type form) list_t))
    (return false)
  )
  (set form-head (index form 0))
  (if (not (eq (type form-head) symbol_t))
    (return false)
  )
  (if (in form-head `(defun defmacro defgeneric defclass import))
    (return true)
  )
  false
)

(defun is-trivial-set-form (new-term)
    (&& (eq (type new-term) list_t) 
        (not (< (len new-term) 2)) 
        (eq (index new-term 0) 'set) 
        (eq (type (index new-term 1)) symbol_t)
    )
)

(defmethod str ((ls list_t))
    (set return-value "(")
    (doit e ls
        (incr return-value (+ (str e) " "))
    )
    (incr return-value ")")
    return-value
)
(defun in-rec (thing-to-inspect target)
    (set return-value false)
    (if (eq (type thing-to-inspect) list_t)
        (doit e thing-to-inspect
            (cond (in-rec e target) (return true) false)
        )
    )
    (eq thing-to-inspect target)
)
(set delayed-map (make-dict ('defun true) ('defmacro true) ('defmethod true) ('defclass true)))

(defclass symbol-location ()
    (symbols (list))
    (constructor (lambda (res source dest) (set (slot res symbols) (list source dest)) res))
)

(defclass file-data ()
    (jump-symbols (list))
    (constructor (lambda (res) res))
)


(set open-documents (dict))

(defun open-handler (handler uri content)
  (set new-envir (new-environment))
  (set (index new-envir 'load-filepath) uri)
  (set file-stream (in-stream content))
  (set input-stream-symbol (gensym))
  (set (index new-envir input-stream-symbol) file-stream)

  (set new-file-data (file-data))

  (set jump-symbols (list))

  (set semantic-tokens (list))
  (set diagnostics (list))
  (set delayed-forms (list))
  (set diagnostics (list))

  (catch-all
          (while (not (eof file-stream))
                 (set new-term (eval `(read-term ,file-stream) new-envir))
                 (skip-whitespace file-stream)
                 (if (should-execute-form new-term)
                   (eval new-term new-envir)
                  else if (is-trivial-set-form new-term)
                   (set (index new-envir (index new-term 1)) null)
                 )
                 (if (&& (type-eq new-term list_t) (< 0 (len new-term)) (type-eq (. new-term 0) symbol_t) (in (. new-term 0) delayed-map))
                        (append delayed-forms new-term)
                        (continue)
                 )
                 (catch-signals
                  (
                   (insert-elements semantic-tokens (default-extractor new-envir new-term))
                   (insert-elements diagnostics (get-diagnostics new-envir new-term))
                  )
                  catch (symbol-location loc)
                  (
                     (append jump-symbols (slot loc symbols))
                  )
                  catch (any_t e)
                  (
                    false
                  )
                 )
          )
          (doit new-term delayed-forms
                 (catch-signals
                  (
                   (insert-elements semantic-tokens (default-extractor new-envir new-term))
                   (insert-elements diagnostics (get-diagnostics new-envir new-term))
                  )
                  catch (symbol-location loc)
                  (
                     (append jump-symbols (slot loc symbols))
                  )
                  catch (any_t e)
                  (
                    false
                  )
                 )
          )
  )

  (set (. open-documents uri) new-file-data)
  (lsp:set-document-tokens handler uri semantic-tokens)
  (lsp:set-document-diagnostics handler uri diagnostics)
  (lsp:set-document-jumps handler uri jump-symbols)
)
(if (not is-repl)
    (lsp:add-on-open-handler handler open-handler)
    (lsp:handle-requests handler)
)
