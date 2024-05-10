(import json)
(import eval-lsp)

(eval-lsp (add-readers *READTABLE*))

(defun type-to-string (type)
    (if (eq type function_t)
        (return "function")
     else if (eq type type_t)
        (return "type")
     else if (eq type lambda_t)
        (return "function")
     else if (eq type generic_t)
        (return "function")
     else if (eq type macro_t)
        (return "macro")
     else 
        (return "var")
    )
)

(defclass dummy-class ()

)
(defun dummy-fun ()
    null
)
(defmacro dummy-macro ()
    null
)

(defun dump-to-envir (dump)
    (set return-value (new-environment))
    (set parsed-dump (read-json dump))
    (doit var parsed-dump
        (set new-object null)
        (set obj-type :"type" var)
        (if (eq obj-type "function")
            (set new-object dummy-fun)
         else if (eq obj-type "type")
            (set new-object dummy-class)
         else if (eq obj-type "macro")
            (set new-object dummy-macro)
        )
        (set-var return-value (symbol :"name" var) new-object)
    )
    return-value
)

(defun dump-envir (envir)
    (set return-value (list))
    (set variables (vars envir))
    (doit var variables
        (append return-value {name: (str var), type: (type-to-string (type (. envir var)))})
    )
    (to-json-string return-value)
)
