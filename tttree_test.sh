#! /bin/bash 

echo "Test de ttree pour le morpion 1o11o1oxx x"
./tttree "1o11o1oxx x" > g1.dot
dot g1.dot -T png -o g1.png

echo "Test de ttree pour le morpion  x21o11xo o"
./tttree "x21o11xo o" > g2.dot
dot g2.dot -T png -o g2.png
