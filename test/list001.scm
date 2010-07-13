; () should eval to () as well
()
'()

; expansion of quote
'a
''a
'''a

; cons, car and cdr
(cons 'a 'b)
(car '(a . b))
(cdr '(a . b))

(car '(a b c))
(cdr '(a b c))

; length and list
(length '())
(length '(1 2))
(length (list 'a 'b 'c '(a b c) '()))

; complex cases
(cons '() ''(a 'b #t '#t (a . b)))
(length '())
(length '(a . 'a))
(length '(a . a))

; type check
(null? '())
(null? '(1))
(null? 1)

(pair? '())
(pair? '(1))
(pair? '(1 . 2))

(list? '())
(list? '(1 . 2))

; list-ref
(list-ref '(1 2 3) 1)
