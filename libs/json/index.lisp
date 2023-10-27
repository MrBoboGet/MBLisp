(defgeneric write-json)
(import text txt)

(defmethod write-json ((obj string_t) stream)
    (write stream "\"")
    (write stream (txt:json-escape obj) )
    (write stream "\"")
)
(defmethod write-json ((obj object_t) stream)
    (write stream "{")
    (set object-slots (slots obj))
    (set non-null-slots  (list))
    (doit s object-slots
        (if (not (eq (. obj s) null))
            (append non-null-slots s)
        )
    )
    (set object-slots non-null-slots)
    (doit i (range 0 (len object-slots))
        (set value (. obj (. object-slots i)))
        (if (eq value null) (continue))
        (write-json (. object-slots i) stream)
        (write stream ":")
        (write-json value stream)
        (if (< (+ i 1) (len object-slots))
            (write stream ",")
        )
    )
    (write stream "}")
)
(defmethod write-json ((obj symbol_t) stream)
    (write stream "\"")
    (write stream (txt:json-escape (str obj)))
    (write stream "\"")
)
(defmethod write-json ((obj list_t) stream)
    (write stream "[")
    (doit i (range 0 (len obj))
        (write-json (index obj i) stream)
        (if (< (+ i 1) (len obj))
            (write stream ",")
        )
    )
    (write stream "]")
)
(defmethod write-json ((obj dict_t) stream)
    (write stream "{")
    (set hash-keys (keys obj))
    (doit i (range 0 (len hash-keys))
        (set current-key (index hash-keys i))
        (if (not (|| (eq (type current-key) symbol_t) (eq (type current-key) string_t)))
          (error "Only symbols and strings are allowed key-types when serialising dictionary")
        )
        (write-json current-key stream)
        (write stream ":")
        (write-json (index obj current-key) stream)
        (if (< (+ i 1) (len hash-keys))
            (write stream  ",")
        )
    )
    (write stream "}")
)
(defmethod write-json ((obj any_t) stream)
    (write stream (str obj))
)

(defun to-json-string (object-to-convert)
  (set return-value "")
  (write-json object-to-convert (out-stream return-value))
  return-value
)




(defun read-comma-delimited-body (end read-element-func stream)
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
)

(defun read-comma-delimited-object (begin end read-element-func stream)
  (skip-whitespace stream)
  (if (not (eq (peek-byte stream) begin))
    (error (+ "object must begin with a " begin))
  )
  (read-byte stream)
  (read-comma-delimited-body end read-element-func stream)
  (if (eof stream)
    (error "eof while reading json object")
  )
  (read-byte stream)
)

(defun read-json-object (stream)
  (set return-value (dict))
  (read-comma-delimited-object "{" "}" 
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
  (set b (list))
  (read-comma-delimited-object "[" "]" _(append b (read-json _)) stream)
  b
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
(defun read-rpc (stream)
    (set content-line  "")
    (while true
      (set new-line (read-line stream))
      (if (in "Content-Length" new-line) (set content-line new-line))
      (if (eq new-line "") (break))
    )
    (if (eq content-line "")
      (error  "Error reading json-rpc message: no content-length header")
    )
    (set space-position (index-of ": " content-line))
    (if (eq space-position -1) (error "no delimiting ': ' for content length"))
    (set content-length (int (substr content-line (+ space-position 2))))
    (set content (read-bytes stream content-length))
    (if (< (len content) content-length) (error "Insufficient bytes sent for message"))
    (read-json (in-stream content))
)
(defun send-rpc (stream object)
    (set content (to-json-string object))
    (write stream (+ (+ "Content-Length: " (str (len content))) "\r\n"))
    (write stream "\r\n")
    (write stream content)
)



(defun read-json-dict-body (stream out-dict)
    (set current-offset (position stream))
    (set key-string (read-until stream ":"))
    (set key-value (read-term stream))
    (set (index out-dict key-string) key-value)
)
(defun list-literal-reader (stream) 
    (set return-value (list 'list))
    (read-comma-delimited-body "]" (lambda (new-stream) (append return-value (read-term new-stream))) stream)
    (read-byte stream)
    return-value
)
(defun json-literal-reader  (stream)
    (set return-value `(make-dict ))
    (set parsed-dict (dict))
    (skip-whitespace stream)
    (set first-byte (peek-byte stream))
    (read-comma-delimited-body "}" (lambda (new-stream) (read-json-dict-body new-stream parsed-dict))  stream)
    (read-byte stream)
    (doit key (keys parsed-dict)
        (append return-value (list key (. parsed-dict key)))
    )
    return-value
)
(defun add-readers (read-table)
    (add-reader-character read-table "{" json-literal-reader)
    (add-reader-character read-table "[" list-literal-reader)
)
