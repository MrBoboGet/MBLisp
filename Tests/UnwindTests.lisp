(import testing)

(defun test-unwind-goto ()
    (set unwind-value false)
    (while true
        (unwind-protect (break) (set unwind-value true))
    )
    unwind-value
)
(defun test-unwind-return ()
    (set unwind-value (list))
    (while true
        (unwind-protect (return unwind-value) (append unwind-value 1))
    )
    unwind-value
)
(defun test-unwind-unwind ()
    (set unwind-value false)
    (catch-signals (progn (signal 123) (set unwind-value "slugma")) catch (any_t val) ((unwind-protect (unwind) (set unwind-value true))))
    unwind-value
)
(defun test-unwind-signal ()
    (set unwind-value true)
    (unwind-protect (set a 1) (progn (set unwind-value false) (signal "asad") (set unwind-value true)))
    unwind-value
)
(new-module "unwind")
(assert_eq (test-unwind-goto) true)
(assert_eq (test-unwind-return) (list 1))
(assert_eq (test-unwind-unwind) true)
(assert_eq (test-unwind-signal) false)


(display-result)
