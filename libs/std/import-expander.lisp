(import lsp.types types)
(defun expand-sym-string (symbol-to-expand)
   (set current-pos (position symbol-to-expand))
   (set symbol-parts (map _(symbol (symbol _) (++ current-pos (plus (len _) 1))) (split (str symbol-to-expand) ":")))
   (if (eq (len symbol-parts) 1)
     (return symbol-to-expand)
   )
   (set symbol-symbols (list))
   (doit i (range 1 (len symbol-parts))
         (append symbol-symbols `(,\quote ,(symbol (index symbol-parts i) -1)))
   )
   (set current-envir ((index types 'get-current-scope)))
   (set current-sym null)
   (catch-all  
       (doit sym symbol-parts
          (if (in sym current-envir)
            (set current-envir (. current-envir sym))
            (set current-sym sym)
           else
            (set current-sym null)
            (break)
          )
       )
   )
   (if (not (eq current-sym null))
        (set res (signal ((index types 'semantic-token) current-sym  ((index types 'type-to-string) (type current-envir) ) )))
   )
  `(,\. ,@(index symbol-parts 0) ,@symbol-symbols)
)
(add-character-expander *READTABLE* ":" expand-sym-string)

