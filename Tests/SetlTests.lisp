(import testing)

(set test 123)
(set test2 (list 1 2 3))

(new-module "setl")

(defun recursive-setl ()
    (setl test1 1)
    (set return-value 0)
    ((lambda () (setl test2 2) ((lambda () (set test1 2) (setl test2 3) (set return-value (+ test2 test1) )))))
    return-value
)
(set global-value 2)
(defun top-test ()
    (set global-value 3)
    ((lambda () (setl global-value 4)))
    global-value
)
(assert_eq (recursive-setl) 5)
(assert_eq (top-test) 3)
(display-result)
