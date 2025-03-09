gravier: gravier.c perl.h
	gcc -g --std=gnu17 gravier.c -I../include/ -o gravier # -lasan -lubsan

lib-gr-tst: perl.h lib-test/main.c
	gcc -g --std=gnu17 -I./ lib-test/main.c -I../include/ -o lib-gr-tst

PERL_CFLAGS=$(shell perl -MExtUtils::Embed -e ccopts)
PERL_LD=$(shell perl -MExtUtils::Embed -e ldopts)

lib-perl-tst: lib-test/main.c
	gcc -g --std=gnu17 $(PERL_CFLAGS)  lib-test/main.c $(PERL_LD) -o lib-perl-tst

gravier-dbg: gravier.c perl.h
	gcc -g --std=gnu17 -DGRAVIER_ENABLE_DEBUG=1 gravier.c -I../include/ -o gravier-dbg

clean:
	rm -rvf gravier

bench:
	for f in *bench.pl; do time perl $$f; time ./gravier $$f ; done

expect_files: lib-perl-tst
	for f in *pl; do perl $$f > $$f.expect ; done
	./lib-perl-tst lib-test/add.pl > lib-test/add.pl.expect
	./lib-perl-tst lib-test/subcall.pl > lib-test/subcall.pl.expect

tests: gravier lib-perl-tst
	for f in *pl; do echo -n "test $$f:"; ./gravier $$f > $$f.out ; diff $$f.expect $$f.out ; echo " $$?" | sed 's/0/OK/;s/1/FAIL/'; done
	./lib-gr-tst lib-test/add.pl > lib-test/add.pl.out
	diff lib-test/add.pl.expect lib-test/add.pl.out ; echo " $$?" | sed 's/0/OK/;s/1/FAIL/'
	./lib-gr-tst lib-test/subcall.pl > lib-test/subcall.pl.out
	diff lib-test/subcall.pl.expect lib-test/subcall.pl.out ; echo " $$?" | sed 's/0/OK/;s/1/FAIL/'

.PHONY : clean
