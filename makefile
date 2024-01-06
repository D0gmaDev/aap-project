all: tttree sm_refresh sm-bot

tttree: tttree.c
	gcc -o tttree tttree.c

sm-refresh: sm-refresh.c super_morpion.c super_morpion.h
	gcc -o sm-refresh sm-refresh.c super_morpion.c

sm-bot: sm-bot.c super_morpion.c super_morpion.h
	gcc -o sm-bot sm-bot.c super_morpion.c

.PHONY: clean

clean:
	rm -f tttree sm-refresh sm-bot *.dot *.png