;;; evaluation eagerness

(define a 3)
a    ;; should be 3

(set! a 4)
a    ;; should be 4

;; and/or is required to behave short-circuitedly
(or #t (set! a 5)) ;; should return #t
a    ;; still be 4

(or #f (set! a 6)) ;; should return nothing
a    ;; should be 6

(and #f (set! a 7)) ;; should return #f
a    ;; still be 6

(and #t (set! a 8)) ;; should return nothing
a    ;; should be 8


;; scheme evaluates function arguments before calling, i.e. eager
(define (b x y z) (+ x y))
(b 1 2 (set! a 9))  ;; should return 3
a    ;; should be 9


;;; lazy evaluation until we have it
;; (define c (delay (+ 1 2)))
;; c    ;; a promise type
;; (force c)    ;; 3
;; (force c)    ;; 3

;;; an interesting example about lazy
;; (define d 1)
;; (define e (set! d (+ d 1)))
;; d    ;; 2
;; e
;; d    ;; 2
;; (define f (delay (set! d (+ d 1))))
;; d    ;; 2
;; (force f)
;; d    ;; 3
;; (force f)
;; d    ;; 3    ;; force is required to be cached
;; (set! d (+ d 1))
;; d    ;; 4
;; (force f)
;; d    ;; 4
;; (force f)
;; d    ;; 4
