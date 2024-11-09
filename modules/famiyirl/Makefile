
hello-yirl.ybin: hello-yirl.o
	ld65 -o hello-yirl.ybin -C yirl-cc64.cfg hello-yirl.o

hello-yirl.o: hello-yirl.asm
	ca65 hello-yirl.asm --cpu 6502

clean:
	rm hello-yirl.o hello-yirl.ybin
