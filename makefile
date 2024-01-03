all: tttree sm_refresh

tttree: tttree.c
	gcc -o tttree tttree.c

sm_refresh: sm_refresh.c
	gcc -o sm_refresh sm_refresh.c

.PHONY: clean

clean:
	rm -f tttree sm_refresh *.dot *.png