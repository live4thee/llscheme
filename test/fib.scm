(define (fib1 n)
  (if (= n 1)
      1
      (if (= n 2)
       1
       (+ (fib1 (- n 1))
	  (fib1 (- n 2))))))

(fib1 5)

(define (fib2 n)
  (fib-iter 1 0 n))

(define (fib-iter a b count)
  (if (= count 0)
      b
      (fib-iter (+ a b) a (- count 1))))

(fib2 40)
