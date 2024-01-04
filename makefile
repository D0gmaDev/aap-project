all: tttree sm_refresh

tttree: tttree.c
	gcc -o tttree tttree.c

sm_refresh: sm_refresh.c super_morpion.c super_morpion.h
	gcc -o sm_refresh sm_refresh.c super_morpion.c

.PHONY: clean

clean:
	rm -f tttree sm_refresh *.dot *.png