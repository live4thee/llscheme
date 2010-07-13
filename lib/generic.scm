;; A simple map which accepts only one list
;; (display (map (lambda (x) (* x x)) '(1 2 3 4)))
(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst)) (map f (cdr lst)))))


;; Left-fold
;; (display (fold + 0 '(1 2 3 4)))
(define (fold f acc lst)
  (if (null? lst)
      acc
      (fold f (f acc (car lst)) (cdr lst))))


;; Right-fold
;; (display (fold-right + 0 '(1 2 3 4)))
(define (fold-right f acc lst)
  (if (null? lst)
      acc
      (f acc (fold-right f (car lst) (cdr lst)))))


;; for-each
;; (for-each (lambda (x) (display (+ x 1))) '(1 2 3 4))
(define (for-each f lst)
  (if (null? lst)
      '()
      (begin (f (car lst)) (for-each f (cdr lst)))))


;; append two lists
;; (display (append '(1 2) '(3 4)))
(define (append l1 l2)
  (if (null? l1)
      l2
      (cons (car l1) (append (cdr l1) l2))))


;; filter
;; (display (filter (lambda (x) (> x 2)) '(1 2 3 4)))
(define (filter p lst)
  (if (null? lst)
      '()
      (if (p (car lst))
	  (cons (car lst) (filter p (cdr lst)))
	  (filter p (cdr lst)))))
