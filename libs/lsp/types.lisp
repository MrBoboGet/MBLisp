(defclass symbol-location ()
    (symbols (list))
    (constructor (lambda (res source dest) (set (slot res symbols) (list source dest)) res))
)
(defclass semantic-token ()
    (content null)
    (constructor (lambda (res token token-type) (set (slot res content) (list token token-type)) res))
)
(defclass jump-location ()
    (content null)
    (constructor (lambda (res token token-type) (set (slot res content) (list token token-type)) res))
)
(defun type-to-string (type-to-check)
    (set return-value "var")
    (if (eq type-to-check macro_t)
        (set return-value "macro")
     else if (eq type-to-check function_t)
        (set return-value "function")
     else if (eq type-to-check lambda_t)
        (set return-value "function")
     else if (eq type-to-check generic_t)
        (set return-value "function")
     else if (eq type-to-check type_t)
        (set return-value "class")
     else 
        (set return-value "var")
    )
    return-value
)
(set current-scope (new-environment))

(defun get-current-scope ()
    current-scope
)
(defun set-current-scope (new-scope)
    (set current-scope new-scope)
)
