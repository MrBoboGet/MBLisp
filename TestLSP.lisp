(set lsp (get-internal-module "lsp"))

(set handler (lsp:create-lsp-server))

(defun insert-elements (out-list in-list)
  (doit e in-list
    (append out-list e)
  )
)

(defun test-func () (print "hello world"))

(defun default-extractor (envir ast)
    (set return-value (list))
    (if (eq (type ast) symbol_t)
      (if (in ast envir)
        (set value (index envir ast))
        (if (eq (type value) macro_t)
          (append return-value (list ast "macro"))
         else if (eq (type value) function_t)
          (append return-value (list ast "function"))
         else if (eq (type value) lambda_t)
          (append return-value (list ast "function"))
         else 
          (append return-value (list ast "var"))
        )

       else 
        (append return-value (list ast "var"))
      )
     else if (eq (type ast) list_t)
       (doit sub-form ast
         (insert-elements return-value (default-extractor envir sub-form))
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
  (if (in form-head `(defun defmacro))
    (return true)
  )
  false
)


(defun open-handler (handler uri content)
  (set new-envir (environment))
  (set (index new-envir 'load-filepath) uri)
  (set file-stream (in-stream content))
  (set input-stream-symbol (gensym))
  (set (index new-envir input-stream-symbol) file-stream)

  (set semantic-tokens (list))
  (set diagnostics (list))
  (while (not (eof file-stream))
         (set new-term (eval `(read-term ,file-stream) new-envir))
         (if (should-execute-form new-term)
           (eval new-term new-envir)
         )
         (insert-elements semantic-tokens (default-extractor new-envir new-term))
         (skip-whitespace file-stream)
  )
  (lsp:set-document-tokens handler uri semantic-tokens)
)

(lsp:add-on-open-handler handler open-handler)
(lsp:handle-requests handler)
(print "adsadasdsadasdsadasd")
