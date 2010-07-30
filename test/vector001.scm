(vector? 0)
(vector? "string")

(define v (make-vector 1 0))
(vector? v)

(make-vector 0)
(make-vector 0 1)

(make-vector 1 "hi")
(make-vector 1 v)

(make-vector 1 'symbol)
(make-vector 1 #f)
(make-vector 1 '(a list))

(make-vector 4 'symbol)
(make-vector 4 #t)
(make-vector 4 '(another list))

(vector)
(vector 1 2)
(vector 1 'a #f "string" v)

(vector-length (vector))
(vector-length (make-vector 0))
(vector-length (make-vector 1))
(vector-length (vector 1 'a #f "string" v))

(vector-ref v 0)
