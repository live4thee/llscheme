;; bind `foo' to a constant
(define foo 4)
foo

(define (bar) 3)
bar
(bar)

;; function definition

;; dynamic
(define sum-of-square
  (lambda (x y)
    (+ (square x)
       (square y))))
(define square
  (lambda (x)
    (* x x)))

(square 10)
(sum-of-square 2 3)

;; syntax suger and,
;; override with bogus
(define (sum-of-square x y)
  (* (square x)
     (square y)))
(sum-of-square 2 3)

(define (square x) (+ x x))
(sum-of-square 2 3)
