
gravier: gravier.c perl.h
	gcc -g --std=gnu17 -O3 gravier.c -I../include/ -o gravier

gravier-dbg: gravier.c perl.h
	gcc -g --std=gnu17 -DGRAVIER_ENABLE_DEBUG=1 gravier.c -I../include/ -o gravier-dbg

clean:
	rm -rvf gravier

bench:
	for f in *bench.pl; do time perl $$f; time ./gravier $$f ; done

expect_files:
	for f in *pl; do perl $$f > $$f.expect ; done

tests: gravier
	for f in *pl; do echo -n "test $$f:"; ./gravier $$f > $$f.out ; diff $$f.expect $$f.out ; echo " $$?" | sed 's/0/OK/;s/1/FAIL/'; done

.PHONY : clean
