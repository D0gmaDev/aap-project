#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "super_morpion.h"

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

void fillNewGame(T_Super_Morpion *position, enum T_Couleur firstPlayer)
{
    for (int i = 0; i < 81; i++)
    {
        position->cases[i] = VIDE;
    }
    for (int i = 0; i < 9; i++)
    {
        position->grilles[i] = VIDE;
    }
    position->trait = firstPlayer;
    position->lastCoupId = -1;
}

int getGrilleIndex(int positionIndex)
{
    int grilleIndex = positionIndex / 9;
    return grilleIndex;
}

int getCaseIndex(int positionIndex)
{
    int caseIndex = positionIndex % 9;
    return caseIndex;
}

int getBoardPopulationCount(enum T_Couleur cases[])
{
    int count = 0;
    for (int i = 0; i < 9; i++)
    {
        if (cases[i] != VIDE)
        {
            count++;
        }
    }
    return count;
}

int isTTTBoardFull(enum T_Couleur cases[])
{
    for (int i = 0; i < 9; i++)
    {
        if (cases[i] == VIDE)
        {
            return 0;
        }
    }
    return 1;
}

int isTTTBoardEmpty(enum T_Couleur cases[])
{
    for (int i = 0; i < 9; i++)
    {
        if (cases[i] != VIDE)
        {
            return 0;
        }
    }
    return 1;
}

enum T_Couleur getTTTWinner(enum T_Couleur cases[])
{
    if (isTTTBoardEmpty(cases))
        return VIDE;

    for (int i = 0; i < 3; i++)
    {
        if (cases[i] != VIDE && cases[i] == cases[i + 3] && cases[i] == cases[i + 6])
        {
            return cases[i];
        }
    }

    for (int i = 0; i < 9; i += 3)
    {
        if (cases[i] != VIDE && cases[i] == cases[i + 1] && cases[i] == cases[i + 2])
        {
            return cases[i];
        }
    }

    if (cases[4] == VIDE) // pas de diagonales sans la case du milieu
        return VIDE;

    if (cases[0] == cases[4] && cases[0] == cases[8])
    {
        return cases[0];
    }

    if (cases[2] == cases[4] && cases[2] == cases[6])
    {
        return cases[2];
    }

    return VIDE;
}

// renvoie la structure des coups possibles. Le pointeur moves.legalMoves doit être libéré après utilisation
T_LegalMoves getLegalMoves(T_Super_Morpion *position)
{
    if (position->lastCoupId == -1)
    {
        int *legalMoves = malloc(81 * sizeof(int));
        assert(legalMoves != NULL);

        for (int i = 0; i < 81; i++)
        {
            legalMoves[i] = i;
        }
        return (T_LegalMoves){legalMoves, 81};
    }

    int nextGrille = position->lastCoupId % 9;

    int legalMovesCount = 0;

    if (position->grilles[nextGrille] != VIDE || isTTTBoardFull(&position->cases[9 * nextGrille]))
    {
        int *legalMoves = malloc(72 * sizeof(int));
        assert(legalMoves != NULL);

        for (int i = 0; i < 81; i++)
        {
            if (position->cases[i] == VIDE)
            {
                legalMoves[legalMovesCount] = i;
                legalMovesCount++;
            }
        }
        return (T_LegalMoves){legalMoves, legalMovesCount};
    }
    else
    {
        int *legalMoves = malloc(9 * sizeof(int));
        assert(legalMoves != NULL);

        int baseIndex = 9 * nextGrille;

        for (int i = 0; i < 9; i++)
        {
            if (position->cases[baseIndex + i] == VIDE)
            {
                legalMoves[legalMovesCount] = baseIndex + i;
                legalMovesCount++;
            }
        }
        return (T_LegalMoves){legalMoves, legalMovesCount};
    }
}

void makeMove(T_Super_Morpion *position, int move)
{
    position->cases[move] = position->trait;

    int grilleIndex = getGrilleIndex(move);

    enum T_Couleur winner = getTTTWinner(&position->cases[grilleIndex * 9]);

    if (winner != VIDE)
    {
        position->grilles[grilleIndex] = winner;

        for (int i = 0; i < 9; i++)
        {
            position->cases[grilleIndex * 9 + i] = winner;
        }
    }

    position->trait = getOther(position->trait);
    position->lastCoupId = move;
}

void printSuperMorpion(T_Super_Morpion *position, int evaluation)
{
    printf("-- -- -- -- -- -- -- --\n");
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            char firstChar = position->grilles[i * 3] != VIDE ? getSymbol(position->grilles[i * 3]) + UPPER_CASE_SHIFT : ' ';
            char secondChar = position->grilles[i * 3 + 1] != VIDE ? getSymbol(position->grilles[i * 3 + 1]) + UPPER_CASE_SHIFT : ' ';
            char thirdChar = position->grilles[i * 3 + 2] != VIDE ? getSymbol(position->grilles[i * 3 + 2]) + UPPER_CASE_SHIFT : ' ';

            printf(" %c %c %c | %c %c %c | %c %c %c \n",
                   firstChar != ' ' ? firstChar : getSymbol(position->cases[27 * i + 3 * j + 0]),
                   firstChar != ' ' ? firstChar : getSymbol(position->cases[27 * i + 3 * j + 1]),
                   firstChar != ' ' ? firstChar : getSymbol(position->cases[27 * i + 3 * j + 2]),
                   secondChar != ' ' ? secondChar : getSymbol(position->cases[27 * i + 3 * j + 9 + 0]),
                   secondChar != ' ' ? secondChar : getSymbol(position->cases[27 * i + 3 * j + 9 + 1]),
                   secondChar != ' ' ? secondChar : getSymbol(position->cases[27 * i + 3 * j + 9 + 2]),
                   thirdChar != ' ' ? thirdChar : getSymbol(position->cases[27 * i + 3 * j + 18 + 0]),
                   thirdChar != ' ' ? thirdChar : getSymbol(position->cases[27 * i + 3 * j + 18 + 1]),
                   thirdChar != ' ' ? thirdChar : getSymbol(position->cases[27 * i + 3 * j + 18 + 2]));
        }
        printf("-- -- -- -- -- -- -- --\n");
    }
    printf("Evaluation de la position : %d\n", evaluation);
}

void printLegalMovesArray(T_LegalMoves *legalMoves)
{
    for (int i = 0; i < legalMoves->count; i++)
    {
        char *move = convertIndexToMove(legalMoves->legalMoves[i]);
        printf("%s | ", move);
        free(move);
    }
    printf("\n");
}

int isInLegalMoves(T_LegalMoves *legalMoves, int moveId)
{
    for (int i = 0; i < legalMoves->count; i++)
    {
        if (legalMoves->legalMoves[i] == moveId)
        {
            return 1;
        }
    }
    return 0;
}