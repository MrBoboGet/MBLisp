(import index)
(set current-tokenizer (get-tokenizer))

(set test-text "first.map(second).third")
(setl stream (in-stream test-text))
(print (MBPerl stream))

#(set-stream current-tokenizer stream)
#(set result (ParseExpr_Operators current-tokenizer))
#(print (to-json-string result))



