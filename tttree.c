#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum T_Couleur // on utilise une enum (on pourrait utiliser aussi des constantes symboliques)
{
    NOIR,
    BLANC,
    VIDE
};

enum T_Couleur getCouleur(char c)
{
    if (c == 'x')
        return NOIR;
    if (c == 'o')
        return BLANC;
    return VIDE;
}

char getSymbol(enum T_Couleur couleur)
{
    if (couleur == NOIR)
        return 'x';
    if (couleur == BLANC)
        return 'o';
    return '.';
}

enum T_Couleur getOther(enum T_Couleur couleur)
{
    switch (couleur)
    {
    case VIDE:
        return VIDE;
    case BLANC:
        return NOIR;
    case NOIR:
        return BLANC;
    }
}

typedef struct
{
    enum T_Couleur cases[9];
    enum T_Couleur trait;
} T_TicTacToe;

void fillPositionFromFen(T_TicTacToe *position, char *fen)
{
    for (int i = 0; i < 9; i++)
    {
        position->cases[i] = VIDE;
    }

    int positionIndex = 0;
    int charIndex = 0;

    char c;
    while (positionIndex < 9)
    {
        c = fen[charIndex];

        enum T_Couleur couleur = getCouleur(c);

        switch (couleur)
        {
        case NOIR:
        case BLANC:
            position->cases[positionIndex] = couleur;
            positionIndex++;
            break;
        case VIDE:
            if (isdigit(c))
            {
                positionIndex += (c - '0');
            }
            break;
        }
        charIndex++;
    }
    position->trait = getCouleur(fen[++charIndex]);
}

// Fonction de debug, non-essentielle au programme final
void printDebugBoard(T_TicTacToe position)
{
    fprintf(stderr, "-------\n");
    fprintf(stderr, " %c %c %c\n", getSymbol(position.cases[0]), getSymbol(position.cases[1]), getSymbol(position.cases[2]));
    fprintf(stderr, " %c %c %c\n", getSymbol(position.cases[3]), getSymbol(position.cases[4]), getSymbol(position.cases[5]));
    fprintf(stderr, " %c %c %c\n", getSymbol(position.cases[6]), getSymbol(position.cases[7]), getSymbol(position.cases[8]));
    fprintf(stderr, "-------\n");
}

void printDotPosition(T_TicTacToe position, int id, int isMax)
{
    printf("m%d [shape=none label=<\n<TABLE border=\"0\" cellspacing=\"10\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"black\">\n", id);

    for (int i = 0; i < 9; i += 3)
    {
        printf("<TR>\n<TD bgcolor=\"white\">%c</TD>\n<TD bgcolor=\"white\">%c</TD>\n<TD bgcolor=\"white\">%c</TD>\n</TR>\n", getSymbol(position.cases[i]), getSymbol(position.cases[i + 1]), getSymbol(position.cases[i + 2]));
    }

    printf("<TR><TD bgcolor=\"%s\" colspan=\"3\">m_%d</TD></TR>\n</TABLE>>];\n\n", isMax ? "green" : "red", id);
}

void printDotLink(int parentId, int currentId, int coup)
{
    printf("m%d-> m%d [label=\"%d,%d\"];\n", parentId, currentId, coup % 3, coup / 3);
}

void printDotLabel(int currentId, int eval)
{
    printf("m%d [xlabel=\"%d\"]\n", currentId, eval);
}

enum T_Couleur getWinner(T_TicTacToe *position)
{
    for (int i = 0; i < 3; i++)
    {
        if (position->cases[i] != VIDE && position->cases[i] == position->cases[i + 3] && position->cases[i] == position->cases[i + 6])
        {
            return position->cases[i];
        }
    }

    for (int i = 0; i < 9; i += 3)
    {
        if (position->cases[i] != VIDE && position->cases[i] == position->cases[i + 1] && position->cases[i] == position->cases[i + 2])
        {
            return position->cases[i];
        }
    }

    if (position->cases[4] == VIDE) // pas de diagonales sans la case du milieu
        return VIDE;

    if (position->cases[0] == position->cases[4] && position->cases[0] == position->cases[8])
    {
        return position->cases[0];
    }

    if (position->cases[2] == position->cases[4] && position->cases[2] == position->cases[6])
    {
        return position->cases[2];
    }

    return VIDE;
}

int isBoardFull(T_TicTacToe *position)
{
    for (int i = 0; i < 9; i++)
    {
        if (position->cases[i] == VIDE)
        {
            return 0;
        }
    }
    return 1;
}

int generateNewId()
{
    static int id = 0;
    return id++;
}

int printMiniMaxDotTreeFrom(T_TicTacToe position, int parentId, int isMax)
{
    enum T_Couleur winnerColor = getWinner(&position);

    if (winnerColor != VIDE)
    {
        return (winnerColor == BLANC) ? -1 : 1;
    }

    if (isBoardFull(&position))
    {
        return 0;
    }

    int eval = isMax ? -1 : 1;

    for (int i = 0; i < 9; i++)
    {
        if (position.cases[i] == VIDE)
        {
            int childId = generateNewId();

            T_TicTacToe newPosition = position;
            newPosition.trait = getOther(position.trait);
            newPosition.cases[i] = position.trait;
            int childEval = printMiniMaxDotTreeFrom(newPosition, childId, 1 - isMax);

            printDotPosition(newPosition, childId, 1 - isMax);
            printDotLink(parentId, childId, i);
            printDotLabel(childId, childEval);
            printf("\n");

            if (isMax)
            {
                eval = (childEval > eval) ? childEval : eval;
            }
            else
            {
                eval = (childEval < eval) ? childEval : eval;
            }
        }
    }

    return eval;
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Une position de départ 'FEN' était attendue en argument.\n");
        return 1;
    }

    T_TicTacToe startPosition; // on pourrait aussi utiliser un malloc, et free à la fin de la fonction main

    fillPositionFromFen(&startPosition, argv[1]); // lecture de la position de départ

    printDebugBoard(startPosition);

    int initialId = generateNewId();

    printf("digraph  {\n");

    printDotPosition(startPosition, initialId, 1);

    int startPositionEval = printMiniMaxDotTreeFrom(startPosition, initialId, 1);
    printDotLabel(initialId, startPositionEval);

    printf("\n}\n");
    return 0;
}