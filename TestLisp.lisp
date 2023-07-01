(set hej "asdasd")
(print hej)
(cond false (print "true") (print "false"))
(print (print "after"))
(tagbody  
  a (print "a")
  b (go d)
  c (print "c")
  d (print "d"))
(print (+ 1 2))
(set i 0)
(print (tagbody 
  begin (cond (< i 3) (set i (+ i 1)) (go end))
  middle (print i)
  temp (go begin)
  end 1))

(set func (lambda (x) (set temp (+ x 1)) (+ temp 2)   ))
(print (func 17))
