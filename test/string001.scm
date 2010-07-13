;; string->number
(string->number "")
(string->number "0")
(string->number "00")
(string->number "010")

(string->number "1")
(string->number "-1")
(string->number "1.")
(string->number "1.0")
(string->number "-1.0")

(string->number "3/2")
(string->number "-3/2")
(string->number "3/-2")

(string->number "3131353555/20")
(string->number "-3131353555/20")

(string->number "3./2")
(string->number "3./2.")

(string->number "32183579913750293759109735")
(string->number "-32183579913750293759109735")
(string->number "32183579913750293759109735.")
