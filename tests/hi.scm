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
  (define mk_hello
    (lambda  (father name)
      (begin
	(display father)
	(display (string_cast  name) )
	(display ( yeCreateString "hello world !" father (string_cast  name ) ))
	( yeCreateString "hello world !" father (string_cast  name ) )
	)
      )
    )

  (define mk_hello2
    (lambda ()
      ( yeCreateString "hello world !"  )
      )
    )
  )

