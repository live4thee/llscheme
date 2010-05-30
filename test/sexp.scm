;; bind `foo' to a constant
(define foo 4)

;; function definition
(define square
  (lambda (x)
    (* x x)))

(define sum-of-square
  (lambda (x y)
    (+ (square x)
       (square y))))

;; define syntactic sugar
(define (sum-of-square x y)
  (+ (square x)
     (square y)))

;; `varargs' Function
(define (foo . lst)
  (length lst))

(define (foo a . lst)
  (+ a (length lst)))

;; `if' expression
(define abs
  (lambda (x)
    (if (>= x 0)
	x
	(- x))))

;; quote
'(define foo 2)	;; should pass
'(define 2 3)	;; should pass, too.
'()		;; shoud pass

(quote (define foo 2))
(quote (define 2 3))
(quote (quote (abc)))

(+ '1 '2)  ;; should return 3 as well
;; we need a unit test that can test failure for the following
; (+ ''1 ''2)  ;; should fail
