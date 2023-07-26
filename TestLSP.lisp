(set lsp (get-internal-module "lsp"))
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

(set overriden-extractors (make-dict ('if if-token-extractor)))
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
         else if (eq (type value) function_t)
          (append return-value (list ast "function"))
         else if (eq (type value) lambda_t)
          (append return-value (list ast "function"))
         else if (eq (type value) generic_t)
          (append return-value (list ast "function"))
         else if (eq (type value) type_t)
          (append return-value (list ast "class"))
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
(set envir-modifiers (make-dict ('defun func-envir) ('defmacro func-envir) ('defmethod method-envir) ('doit doit-envir) ))
(set diagnostics-overrides (make-dict ('if if-diagnostics)))


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
  (if (in form-head `(defun defmacro defgeneric defclass))
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


(defun open-handler (handler uri content)
  (set new-envir (new-environment))
  (set (index new-envir 'load-filepath) uri)
  (set file-stream (in-stream content))
  (set input-stream-symbol (gensym))
  (set (index new-envir input-stream-symbol) file-stream)

  (set semantic-tokens (list))
  (set diagnostics (list))
  (try
   (
      (while (not (eof file-stream))
             (set new-term (eval `(read-term ,file-stream) new-envir))
             (if (should-execute-form new-term)
               (eval new-term new-envir)
              else if (is-trivial-set-form new-term)
               (set (index new-envir (index new-term 1)) null)
             )
             (insert-elements semantic-tokens (default-extractor new-envir new-term))
             (insert-elements diagnostics (get-diagnostics new-envir new-term))
             (skip-whitespace file-stream)
      )
   )
   catch (any_t e)
   (

   )
  )
  (lsp:set-document-tokens handler uri semantic-tokens)
  (lsp:set-document-diagnostics handler uri diagnostics)
)
(lsp:add-on-open-handler handler open-handler)
(lsp:handle-requests handler)
