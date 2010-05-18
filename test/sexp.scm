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

;; `if' expression
(define abs
  (lambda (x)
    (if (>= x 0)
	x
	(- x))))
