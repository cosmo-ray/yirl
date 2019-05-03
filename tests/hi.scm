(begin
  (display "hello file !!")
  (newline)
  (define display_ent
    (lambda (e) (begin
		  (display (ptr_cast (/ (int_cast e) 2) ) )
		  (/ (int_cast e) 2))
	    )
    )
  (define display_eint
    (lambda (e) (begin
		  (display (yeGetInt e) )
		  (yeGetInt e)
		  )
	    )
    )
  )
