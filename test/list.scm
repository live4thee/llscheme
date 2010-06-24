; () should eval to () as well
()
'()

; expansion of quote
'a
''a
'''a

; cons, car and cdr
(cons 'a 'b)

(define a (cons 'a '(b c d e)))
a
(car a)
(cdr a)

; defining cadr and caddr as library
(define (cadr x) (car (cdr x)))
(define (caddr x)
  (car (cdr (cdr x))))

(cadr a)
(caddr a)

; length and list
(length a)
(define b (list 'a 'b 'c '(a b c) '()))
b
(length b)

; complex cases
(cons () ''(a 'b #t '#t (a . b)))
(length '())
(length '(a . 'a))
(length '(a. a))
; should fail
; (length '(a . a))
