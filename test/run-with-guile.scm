#! /usr/bin/guile \
-e main -s
!#

;;;
;;; Copyright (C) 2010 David Lee <live4thee@gmail.com>
;;; Copyright (C) 2010 Qing He <qing.x.he@gmail.com>
;;;
;;; This program is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2 of the License, or
;;; (at your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, write to the Free Software
;;; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;;;

;; This Scheme script below will read an input of Scheme code and
;; print the evaluated result sequentially with GNU/Guile.

(use-modules (ice-9 r5rs))

(define (interpret . arg)
  (let ((port (if (null? arg) (current-input-port) (car arg))))
    (let loop ((sexp (read port)))
      (if (not (eof-object? sexp))
	  (begin (display (eval sexp (scheme-report-environment 5)))
		 (newline)
		 (loop (read port)))
	  (close-input-port port)))))

(define (main args)
  (if (null? (cdr args))
      (interpret)
      (for-each
       (lambda (port) (interpret port))
       (map open-input-file (cdr args)))))
