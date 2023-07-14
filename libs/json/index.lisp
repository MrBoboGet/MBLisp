(defgeneric write-json)


(defmethod write-json ((obj string_t) stream)
    (write "\"" stream)
    (write obj stream)
    (write "\"" stream)
)
(defmethod write-json ((obj symbol_t) stream)
    (write "\"" stream)
    (write (str obj) stream)
    (write "\"" stream)
)
(defmethod write-json ((obj list_t) stream)
    (write "[" stream)
    (doit i (range 0 (len obj))
        (write-json (index obj i) stream)
        (if (< (+ i 1) (len obj))
            (write "," stream)
        )
    )
    (write "]" stream)
)
(defmethod write-json ((obj dict_t) stream)
    (write "{" stream)
    (set hash-keys (keys obj))
    (doit i (range 0 (len hash-keys))
        (set current-key (index hash-keys i))
        (if (not (|| (eq (type current-key) symbol_t) (eq (type current-key) string_t)))
          (error "Only symbols and strings are allowed key-types when serialising dictionary")
        )
        (write-json current-key stream)
        (write ":" stream)
        (write-json (index obj current-key) stream)
        (if (< (+ i 1) (len hash-keys))
            (write "," stream)
        )
    )
    (write "}" stream)
)
(defmethod write-json ((obj any_t) stream)
    (write (str obj) stream)
)

(defmacro ref (symbol-to-ref)
  `(index (environment) (quote ,symbol-to-ref))
)

(defun to-json-string (object-to-convert)
  (set return-value "")
  (write-json object-to-convert (out-stream (ref return-value)))
  return-value
)





(defun read-comma-delimted-object (begin end read-element-func stream)
  (skip-whitespace stream)
  (if (not (eq (peek-byte stream) begin))
    (error (+ "object must begin with a " begin))
  )
  (read-byte stream)
  (skip-whitespace stream)
  (while (&& (not (eof stream)) (not (eq (peek-byte stream) end)))
    (read-element-func stream)
    (skip-whitespace stream)
    (if (eof stream) (error "eof while reading json object"))
    (if (eq (peek-byte stream) end) 
      (break)
     else if (eq (peek-byte stream) ",")
      (read-byte stream)
      (skip-whitespace stream)
     else 
      (error (+ "unexpected character in json object: " (peek-byte stream)))
    )
  )
  (if (eof stream)
    (error "eof while reading json object")
  )
  (read-byte stream)
)

(defun read-json-object (stream)
  (set return-value (dict))
  (read-comma-delimted-object "{" "}" 
    (lambda (stream)
      (set new-key (read-string stream))
      (skip-whitespace stream)
      (if (eof stream) (error "eof while reading key value in json object"))
      (if (not (eq (peek-byte stream) ":")) (error "only : can occur after json key"))
      (read-byte stream)
      (set new-value (read-json stream))
      (set (index return-value new-key) new-value)
    )
    stream
  )
  return-value
)

(defun read-json-list (stream)
  (set return-value (list))
  (read-comma-delimted-object "[" "]" (lambda (stream) (append return-value (read-json stream))) stream)
  return-value
)


(defun read-json (stream)
    (set return-value "")
    (skip-whitespace stream)
    (set first-byte (peek-byte stream))
    (if (eq first-byte "{")
        (set return-value (read-json-object stream))
     else if (eq first-byte "[")
        (set return-value (read-json-list stream))
     else if (eq first-byte "\"")
        (set return-value (read-string stream))
     else if (eq first-byte "f")
        (set return-value (read-bytes stream 5))
        (if (not (eq return-value "false")) (error (+ "invalid sequence as start of json object: " return-value)))
        (set return-value false)
     else if (eq first-byte "t")
        (set return-value (read-bytes stream 4))
        (if (not (eq return-value "true")) (error (+ "invalid sequence as start of json object: " return-value)))
        (set return-value true)
     else if (eq first-byte "n" )
        (set return-value (read-bytes stream 4))
        (if (not (eq return-value "null")) (error (+ "invalid sequence as start of json object: " return-value)))
        (set return-value null)
     else 
        (set return-value (read-number stream))
    )
    return-value
)