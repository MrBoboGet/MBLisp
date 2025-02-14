(import lsp-internal lsp)
(import lsp.types)
(import dump)
(import json)
(set handler (lsp:create-lsp-server))

(import eval-lsp)


(defun if-token-extractor (envir ast)
   (setl return-value (list)) 
   (doit e ast
        (if (&& (eq (type e) symbol_t) (|| (eq (str e) "if") (eq (str e) "else")))
            (append return-value (list e "macro"))
        )
   )
   return-value
)

(defun go-diagnostics (envir ast)
    (list)
)

(defun try-token-extractor (envir ast)
   (setl return-value (list)) 
   (doit e ast
        (if (&& (eq (type e) symbol_t) (|| (eq e 'catch) (eq e 'catch)))
            (append return-value (list e "macro"))
        )
   )
   return-value
)

(defun all (iterable)
    (setl return-value true)
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
  (setl return-value false)
  (if (&& (< 2 (len ast)) (type-eq (. ast 1) symbol_t))
      (setl base (. ast 1))
      (if (&& (in base envir) (not (type-eq (. envir base) null_t)))
        (setl return-value (all (map _(is-literal (. ast _)) (range 2 (len ast)))))
      ) 
  ) 
  return-value
)

(defun type-eq (lhs rhs)
    (eq (type lhs) rhs)
)
(defun default-extractor (envir ast)
    (setl return-value (list))
    (if (eq (type ast) symbol_t)
      (if (< (position ast) 0)
        (return return-value)
      ) 
      (if (in ast envir)
        (setl value (index envir ast))
        (setl value-type (type value))
        (if (eq value-type macro_t)
            (append return-value (list ast "macro"))
            (signal (symbol-location ast (name value)))
         else if (eq value-type function_t)
            (append return-value (list ast "function"))
         else if (eq value-type lambda_t)
            (append return-value (list ast "function"))
            (signal (symbol-location ast (name value)))
         else if (eq value-type generic_t)
            (append return-value (list ast "function"))
            (signal (symbol-location ast (name value)))
         else if (eq value-type type_t)
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
       (doit sub-form ast
         (insert-elements return-value (default-extractor envir sub-form))
       )
    )  
    return-value
)
(defun quote-diagnostics (envir ast)
    (list)
)
(defun signal-handlers-diagnostics (envir ast)
    (setl return-value (list))
    (insert-elements return-value (get-diagnostics envir (. ast 1)))
    (setl i 2)
    (while (< i (len ast))
        (setl case-envir (new-environment))
        (shadow case-envir (. ast i 1))
        (set-parent case-envir envir)
        (insert-elements return-value (get-diagnostics case-envir (. ast (+ i 1))))
        (incr i 2)
    )
    return-value
)

(defun lambda-envir (envir ast)
  (setl return-value (new-environment))
  (set-parent return-value envir)
  (doit arg (. ast 1)
    (if (eq (type arg) symbol_t) (shadow return-value arg))
  )
  return-value
)
(defun bind-diagnostics (envir ast)
  (setl return-value (list))
  (setl new-envir (new-environment))
  (set-parent new-envir envir)
  (shadow (. (. ast 1) 1) null)
  (doit i (range 1 (len ast))
    (insert-elements return-value (get-diagnostics new-envir (. ast i)))
  )
)
(defun class-diagnostics (envir ast)
  (setl return-value (list))
  (insert-elements return-value (get-diagnostics envir (. ast 1)))
  (doit slot-def (. ast 2 1)
    (insert-elements return-value (get-diagnostics envir (. slot-def 1)))
  )
  (if (eq (len ast) 4)
    (insert-elements return-value (get-diagnostics envir (. ast 3)))
  )
  return-value
)

(set envir-modifiers (make-dict 
                        ('lambda lambda-envir) 
))

(set pre-expand-diagnostics (make-dict 
))
(set pre-expand-tokens (make-dict 
        ('if if-token-extractor)
        ('try try-token-extractor)
        ('catch-signals try-token-extractor)
))

(set diagnostics-overrides (make-dict 
                        ('go go-diagnostics) 
                        ('quote quote-diagnostics)
                        ('signal-handlers signal-handlers-diagnostics)
                        ('class class-diagnostics)
))

(defun get-diagnostics (envir ast)
  (setl return-value (list)) 
  (if (eq (type ast) symbol_t)
    (if (&& (not (is-special ast)) (not (in ast envir)) (not (< (position ast) 0) ))
      (append return-value (list ast (+ (+ "Couldn't find symbol " (str ast)) " in current environment")))
    )
   else if (eq (type ast) list_t)
    (if (not (< (len ast) 1))
      (setl head (. ast 0))
      (if (eq (type head) symbol_t)
        (if (in head diagnostics-overrides)
          (return ((. diagnostics-overrides head) envir ast))
         else if (in head envir-modifiers)
          (setl envir ((. envir-modifiers head) envir ast))
        )
      )
    )
    (doit e ast
      (if (is-trivial-set-form e)
        (if (eq (. e 0) 'set)
            (if (not (in (. e 1) envir))
                (set-var envir (. e 1) null)
            )
         else 
            (set (shadow envir (. e 1)) null)
        )
      )
      (insert-elements return-value (get-diagnostics envir  e))
    )
  )
  return-value
)
(set read-time-forms (make-dict 
                    ('defun true) 
                    ('defmacro true)
                    ('defgeneric true)
                    ('defclass true)
                    ('import true)
                    ('eval-lsp true)
))

    

(defun should-execute-form (form)
  (if (not (eq (type form) list_t))
    (return false)
  )
  (setl form-head (index form 0))
  (if (not (eq (type form-head) symbol_t))
    (return false)
  )
  (if (in form-head read-time-forms)
    (return true)
  )
  false
)
(defun eval-selected (form envir)
     (if (should-execute-form form)
       (catch-all (eval form envir))
      else if (is-trivial-set-form form)
       (set-var envir (index form 1) null)
       else if (&& (is-defmethod form) (not (in :1 form envir)))
       (setl new-generic (generic))
       (set-name new-generic :1 form)
       (set-var envir :1 form new-generic)
     )
     (if (&& (eq (type form) list_t) (> (len form) 0) (eq :0 form 'progn))
        (doit i (range 1 (len form))
            (eval-selected ;i form envir)
        )
     )
)
(defun is-trivial-set-form (new-term)
    (&& (eq (type new-term) list_t) 
        (not (< (len new-term) 2)) 
        (|| (eq (index new-term 0) 'set) (eq (index new-term 0) 'setl) )
        (eq (type (index new-term 1)) symbol_t)
    )
)
(defun is-defmethod (new-term)
    (&& (eq (type new-term) list_t) 
        (> (len new-term) 1) 
        (eq (type :0 new-term) symbol_t)
        (eq :0 new-term 'defmethod))
)

(set delayed-map (make-dict ('defun true) ('defmacro true) ('defmethod true) ('defclass true)))

(defun extract-macros (envir ast tokens jumps diagnostics) 
    (setl return-value (list))
    (if (type-eq ast list_t)
        (setl macro-offset 0)
        (if (&& (not (eq (len ast) 0)) (type-eq (. ast 0) symbol_t))
            (setl macro-offset 1)
            (setl sym (. ast 0))
            (if (in sym pre-expand-diagnostics)
                (insert-elements diagnostics ((. pre-expand-diagnostics sym) envir ast))
            )
            (if (in sym pre-expand-tokens)
                (insert-elements tokens ((. pre-expand-tokens sym) envir ast))
            )
            (if (&& (in sym envir) (type-eq (index envir sym) macro_t))
                (setl value (index envir sym))
                (append return-value (list sym "macro"))
                (signal (symbol-location sym (name value)))
            )
        )
        #(setl DEBUG-RANGE (range macro-offset (len ast)))
        #(write debug-file (to-json-string DEBUG-RANGE))
        (doit i (range macro-offset (len ast))
            (insert-elements return-value (extract-macros envir (. ast i) tokens jumps diagnostics))
        )
    )
    return-value
)


(defun handle-form (envir ast tokens jumps diagnostics)
   (setl extra-ast-forms (list))
   (catch-signals
    (
      (insert-elements tokens (extract-macros envir ast tokens jumps diagnostics))
      (if (&& (type-eq ast list_t) (type-eq ( . ast 0) symbol_t))
        (setl head-sym (. ast 0))
        (setl ast (eval `(expand (quote ,ast)) envir))
      )
      (insert-elements tokens (default-extractor envir ast))
      (insert-elements diagnostics (get-diagnostics envir ast))
      (doit form extra-ast-forms
        (insert-elements tokens (default-extractor envir form))
      )
    )
    catch (symbol-location loc)
    (
       (append jumps (slot loc symbols))
    )
    catch (semantic-token new-token)
    (
       (append tokens (slot new-token content))
       "ligma"
    )
    catch (extra-ast new-ast)
    (
       (append extra-ast-forms :ast new-ast)
       "ligma"
    )
    catch (any_t e)
    (
      #(write debug-file (str e))
      false
    )
   )
)
(set debug-file (open "DEBUG_LSP.txt" "w"))

(set loaded-files (make-dict))

(defun lsp-get-scope (file-to-load)
    (setl canonical-uri (canonical file-to-load))
    (if (in canonical-uri loaded-files) (return (. loaded-files canonical-uri)))
    (open-file canonical-uri)
    (. loaded-files canonical-uri)
)
(defun open-file (uri)
    (setl content (read-bytes (open uri) 123123123))
    (try 
        (open-handler uri content)
    catch (any_t e)
        null
    )
)

(defun import-program-envir (sym binding)
    (set string (str sym))
    (set path (+ (user-home-dir) "/.mblisp/modules/" string  ".json"))
    (if (exists path)
        (set data (read-bytes (open path) 100000))
        (set result-envir (dump-to-envir data))
        (return result-envir)
    )
    (error "Could not find import")
)

(defun open-handler (handler uri content)
  (setl new-envir (new-environment))
  (setl file-stream (in-stream content))
  (setl input-stream-symbol (gensym))
  (setl (index new-envir input-stream-symbol) file-stream)

  (set no-import-handler import-program-envir)
  (setl jump-symbols (list))

  (setl semantic-tokens (list))
  (setl diagnostics (list))
  (setl delayed-forms (list))
  (setl diagnostics (list))
  (set-current-scope new-envir)
  (setl extra-ast-values (list))
  (let ((load-filepath uri))
      (catch-signals
        (
              (while (not (eof file-stream))
                     (let ((load-envir new-envir))
                        (setl new-term (eval `(read-term ,file-stream) new-envir))
                        (skip-whitespace file-stream)
                        (eval-selected new-term new-envir)
                     )
                     (if (&& (type-eq new-term list_t) (< 0 (len new-term)) (type-eq (. new-term 0) symbol_t) (in (. new-term 0) delayed-map))
                            (append delayed-forms new-term)
                      else
                            (handle-form new-envir new-term semantic-tokens jump-symbols diagnostics)
                     )
              )
              (doit ast extra-ast-values
                  (insert-elements semantic-tokens (default-extractor new-envir ast))
              )
        )    
        catch (symbol-location loc)
        (
           (append jump-symbols (slot loc symbols))
        )
        catch (semantic-token new-token)
        (
           (append semantic-tokens (slot new-token content))
           "ligma"
        )
        catch (extra-ast new-token)
        (
           (append extra-ast-values :ast new-token)
           "ligma"
        )
        catch (any_t e)
        (
          #(write debug-file (str e))
          false
        )
      )
  )
  (doit new-term delayed-forms
         (handle-form new-envir new-term semantic-tokens jump-symbols diagnostics)
  )
  (clear new-envir)
  (lsp:set-document-tokens handler uri semantic-tokens)
  (lsp:set-document-diagnostics handler uri diagnostics)
  (lsp:set-document-jumps handler uri jump-symbols)
)

(defun main ()
    (catch-signals 
        (
            (lsp:add-on-open-handler handler open-handler)
            (lsp:handle-requests handler)
        )
        catch (any_t e)
        (
            #(write debug-file (str e))
            (print e)
            null
        )
    )
)
