;; A simple map which accepts only one list
(define (map f lst)
  (if (null? lst)
      '()
      (cons (f (car lst)) (map f (cdr lst)))))

(display (map (lambda (x) (* x x)) '(1 2 3 4)))

;; Left-fold
(define (foldl f acc lst)
  (if (null? lst)
      acc
      (foldl f (f acc (car lst)) (cdr lst))))

(display (foldl + 0 '(1 2 3 4)))

;; Right-fold
(define (foldr f acc lst)
  (if (null? lst)
      acc
      (f acc (foldr f (car lst) (cdr lst)))))

(display (foldr + 0 '(1 2 3 4)))

;; for-each
(define (for-each f lst)
  (if (null? lst)
      '()
      (begin (f (car lst)) (for-each f (cdr lst)))))

(for-each (lambda (x) (display (+ x 1))) '(1 2 3 4))

;; append two lists
(define (append l1 l2)
  (if (null? l1)
      l2
      (cons (car l1) (append (cdr l1) l2))))

(display (append '(1 2) '(3 4)))

;; filter
(define (filter p lst)
  (if (null? lst)
      '()
      (if (p (car lst))
	  (cons (car lst) (filter p (cdr lst)))
	  (filter p (cdr lst)))))

(display (filter (lambda (x) (> x 2)) '(1 2 3 4)))
