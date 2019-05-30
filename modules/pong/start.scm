(begin
  (define pong_action
    (lambda (p eve)
      (let (
	    (chk_y (lambda (p y) (if (< (+ (ywPosY p) y) 0) 1
				     (if (> (+ (+ (ywPosY p) 50) y)
					    (yWindowHeight)
					    )
					 -1 y
					 )
				     ) ) )
	    (flip_dy (lambda (p y pos)
		       (begin  (yeSetIntAt p "b_y_dir" (- y) )
			       (- y)
			       ) ) )
	    (chk_by (lambda (pw p y flip_dy)
		      (if (< (+ (ywPosY p) y) 0) (flip_dy pw y p)
			  (if (> (+ (+ (ywPosY p) 5) y)
				 (yWindowHeight)
				 )
			      (flip_dy pw y p) y
			      )
			  ) ) )
	    (chk_bx (lambda (p bl x)
		      (if (ywCanvasCheckCollisions p bl)
			  (yeSetIntAt p "b_x_dir" (- x) ))))
	    (do_mv (lambda (p pl x y) (ywCanvasMoveObjXY pl x y) ))
	    (goal (lambda (p bl who)
		    (begin
		      (display "\nGOAL !!!\n")
		      (if (= who 0) (yeIncrAt p "p0-score")
			  (yeIncrAt p "p1-score"))
		      (ywCanvasObjSetPos bl (/ (yWindowWidth) 2) 20)
		      )
		    ))
	    (ai (lambda (p p1 p1p blp) 1 ))
	    (p0 (yeGet p "p0"))
	    (p1 (yeGet p "p1"))
	    (bl (yeGet p "bl"))
	    (bx (lambda (p) (yeGetIntAt p "b_x_dir")))
	    (by (lambda (p) (yeGetIntAt p "b_y_dir")))
	    (blp (ywCanvasObjPos (yeGet p "bl")))
	    (p0p (ywCanvasObjPos (yeGet p "p0")))
	    (p1p (ywCanvasObjPos (yeGet p "p1")))
	    )
	(begin
	  (if (yevIsGrpDown eve (yeGet p "down_k") ) (yeSetIntAt p "down" 1))
	  (if (yevIsGrpDown eve (yeGet p "up_k") ) (yeSetIntAt p "up" 1))
	  (if (yevIsGrpUp eve (yeGet p "down_k") ) (yeSetIntAt p "down" 0))
	  (if (yevIsGrpUp eve (yeGet p "up_k") ) (yeSetIntAt p "up" 0))
	  (if (= (yeGetIntAt p "up") 1) (do_mv p p0 0 (chk_y p0p -4)))
	  (if (= (yeGetIntAt p "down") 1) (do_mv p p0 0 (chk_y p0p 4)))
	  (chk_bx p bl (bx p))
	  (do_mv p bl (bx p) (chk_by p blp (by p) flip_dy))
	  (if (< (ywPosX blp) 0) (goal p bl 0))
	  (if (> (ywPosX blp) (yWindowWidth)) (goal p bl 1))
	  (do_mv p p1 0 (chk_y p1p (ai p p1 p1p blp)))

					; reprint score:
	  (ywCanvasStringSet
	   (yeGet p "score")
	   (yeStringAddInt
	    (yeStringAdd (yeStringAddInt
			  (yeCreateString  "score: ") (yeGetIntAt p "p0-score"))
			 " - ") (yeGetIntAt p "p1-score"))
	   )
	  YEVE_ACTION)
	)
      )
    )

  (define init_pong
    (lambda (p)
      (letrec (
	       (mk_pong_bar
		(lambda ()
		  (init_c_rect (yeCreateArray) 10 50 "rgba: 255 255 255 255" )
		  ))
	       (mk_pong_ball
		(lambda ()
		  (init_c_rect (yeCreateArray) 5 5 "rgba: 255 255 255 255" )
		  ))
	       (init_c_rect
		(lambda (b w h c)
		  (begin
		    (ywPosCreate w h b) (yeCreateString c b)
		    b)
		  ))
	    )
	(begin
	  (yeCreateString "canvas" p "<type>")
	  (yeCreateArray  p "objs")
	  (yeCreateString "rgba: 0 0 100 255" p "background")
	  (yeCreateInt 15000 p "turn-length")
	  (yeCreateInt 0 p "p_dead")
	  (yePushBack p (ywCanvasNewRect  p (/ (yWindowWidth) 2) 50 (mk_pong_ball))
		      "bl")
	  (yePushBack p (ywCanvasNewRect  p 20 50 (mk_pong_bar)) "p0")
	  (yePushBack p (ywCanvasNewRect  p (- (yWindowWidth) 30) 50 (mk_pong_bar))
		      "p1")
	  (yePushBack p (ywCanvasNewTextByStr p
					      (- (/ (yWindowWidth) 2) 70)
					      5 "" ) "score")
	  (yeCreateFunction "pong_action" p "action")
	  (yeCreateInt 0 p "down")
	  (yeCreateInt 0 p "up")
	  (yeCreateInt -4 p "b_x_dir")
	  (yeCreateInt -1 p "b_y_dir")
	  (yeCreateInt 0 p "p0-score")
	  (yeCreateInt 0 p "p1-score")
	  (yePushBack p (yevCreateGrp Y_DOWN_KEY) "down_k")
	  (yePushBack p (yevCreateGrp Y_UP_KEY) "up_k")
	  p)
	)
      )
    )

  (define mod_init
    (lambda (mod)
      (begin
	(yePushBack mod (init_pong (yeCreateArray)) "start")
	(yeCreateString "start" mod "starting_widget")
	(yeCreateString "pong" mod "name")
	(display (yeGet mod "starting_widget"))
	mod
	)
      )
    )
  )
