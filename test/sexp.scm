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
