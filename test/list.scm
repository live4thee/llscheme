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
(length '(a . 'a)) ; 3
(length '(a . a)) ; should fail

; type check
(null? '())  ; #t
(null? '(1)) ; #f
(null? 1)    ; #f

(pair? '())  ; #f
(pair? '(1)) ; #t
(pair? '(1 . 2)) ; #t

(list? '())  ; #t
(list? '(1 . 2)) ; #f

; list-ref
(list-ref '(1 2 3) 1) ; 2
