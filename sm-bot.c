#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "super_morpion.h"

#define DEFAULT_NEGAMAX_DEPTH 8
// #define DEBUG

// implémentation de super_morpion.h spécifique à ce livrable
int convertMoveToIndex(char *move)
{
    char grille = move[0];
    char caseIndex = move[1];

    if (grille == '0' && caseIndex == '0') // premier coup
    {
        return -1;
    }

    return (grille - '1') * 9 + (caseIndex - '1');
}

// implémentation de super_morpion.h spécifique à ce livrable
char *convertIndexToMove(int index)
{
    char *move = malloc(3 * sizeof(char));
    assert(move != NULL);

    int grille = getGrilleIndex(index);
    int caseIndex = getCaseIndex(index);

    move[0] = grille + '1';
    move[1] = caseIndex + '1';
    move[2] = '\0';
    return move;
}

void proccessAlignment(int *count, int cases[3])
{
    if (cases[VIDE] == 1)
    {
        if (cases[NOIR] == 2)
        {
            (*count)++;
        }
        else if (cases[BLANC] == 2)
        {
            (*count)--;
        }
    }
}

// Les blancs gagnent : c'est négatif, les noirs gagnent : c'est positif, égalité : 0
int evaluateTTT(enum T_Couleur cases[])
{
    enum T_Couleur winnerColor = getTTTWinner(cases);

    if (winnerColor != VIDE)
    {
        return (winnerColor == NOIR) ? 8 : -8;
    }

    if (isTTTBoardFull(cases))
    {
        return 0;
    }

    // optimisation conséquente : les grilles avec moins de 2 marques ne sont pas déterminables
    if (isTTTBoardEmpty(cases) || getBoardPopulationCount(cases) == 1)
    {
        return 0;
    }

    int count[3] = {0, 0, 0}; // compter le nombre de pions de chaque couleur

    // vérifier le nombre d'alignement de deux pions de la même couleur alignés avec un pion vide
    int numberOfAlignments = 0;

    // Vérification des lignes et colonnes
    for (int i = 0; i < 3; i++)
    {
        int rowIndex = i * 3;
        int colIndex = i;

        count[NOIR] = 0;
        count[BLANC] = 0;
        count[VIDE] = 0;

        for (int j = 0; j < 3; j++)
        {
            enum T_Couleur couleur = cases[rowIndex + j];
            count[couleur]++;
        }

        proccessAlignment(&numberOfAlignments, count);

        count[NOIR] = 0;
        count[BLANC] = 0;
        count[VIDE] = 0;

        // Vérification des colonnes
        for (int j = 0; j < 3; j++)
        {
            enum T_Couleur couleur = cases[colIndex + j * 3];
            count[couleur]++;
        }

        proccessAlignment(&numberOfAlignments, count);
    }

    // Vérification des diagonales
    count[NOIR] = 0;
    count[BLANC] = 0;
    count[VIDE] = 0;

    count[cases[0]]++;
    count[cases[4]]++;
    count[cases[8]]++;

    proccessAlignment(&numberOfAlignments, count);

    count[NOIR] = 0;
    count[BLANC] = 0;
    count[VIDE] = 0;

    count[cases[2]]++;
    count[cases[4]]++;
    count[cases[6]]++;

    proccessAlignment(&numberOfAlignments, count);

    return numberOfAlignments;
}

int evaluateSmallMorpion(T_Super_Morpion *position, int grilleIndex)
{
    int grilleStart = grilleIndex * 9;

    int eval = evaluateTTT(&position->cases[grilleStart]);
    return eval;
}

int evaluate(T_Super_Morpion *position)
{
    int macroEvaluation = evaluateTTT(position->grilles);

    int numberOfGrillesWon = 0;

    for (int i = 0; i < 9; i++)
    {
        switch (position->grilles[i])
        {
        case NOIR:
            numberOfGrillesWon++;
            break;
        case BLANC:
            numberOfGrillesWon--;
            break;
        default:
            break;
        }
    }

    int microEvaluation = 0;

    for (int i = 0; i < 9; i++)
    {
        microEvaluation += evaluateSmallMorpion(position, i);
    }

    return 40 * macroEvaluation + 3 * numberOfGrillesWon + microEvaluation;
}

T_eval negamax(T_Super_Morpion position, int depth, int alpha, int beta, enum T_Couleur firstPlayer)
{
    if (depth == 0)
    {
        return (T_eval){-1, firstPlayer == NOIR ? evaluate(&position) : -evaluate(&position)}; // on inverse l'évaluation si c'est au tour des blancs
    }

    enum T_Couleur winner = getTTTWinner(position.grilles);

    if (winner == NOIR)
        return (T_eval){-1, firstPlayer == NOIR ? 642 : -642};
    else if (winner == BLANC)
        return (T_eval){-1, firstPlayer == NOIR ? 642 : -642};

    T_LegalMoves legalMoves = getLegalMoves(&position);

    if (legalMoves.count == 0) // match nul
    {
        free(legalMoves.legalMoves);
        return (T_eval){-1, 0};
    }

    int bestEval = -1000;
    int bestMove = -3;

    for (int i = 0; i < legalMoves.count; i++)
    {
        int move = legalMoves.legalMoves[i];

        T_Super_Morpion newPosition = position;
        makeMove(&newPosition, move);

        T_eval childEval = negamax(newPosition, depth - 1, -beta, -alpha, firstPlayer);

        if (-childEval.eval > bestEval)
        {
            bestEval = -childEval.eval;
            bestMove = move;
        }

        if (bestEval > alpha)
        {
            alpha = bestEval;
        }

        if (alpha >= beta)
        {
            break;
        }
    }

    free(legalMoves.legalMoves);
    return (T_eval){bestMove, bestEval};
}

void fillPositionFromFen(T_Super_Morpion *position, char *fen)
{
    for (int i = 0; i < 81; i++)
    {
        position->cases[i] = VIDE;
    }
    for (int i = 0; i < 9; i++)
    {
        position->grilles[i] = VIDE;
    }

    int positionIndex = 0;
    int fenIndex = 0;

    while (positionIndex < 81)
    {
        switch (fen[fenIndex])
        {
        case 'x':
            position->cases[positionIndex] = NOIR;
            positionIndex++;
            break;
        case 'o':
            position->cases[positionIndex] = BLANC;
            positionIndex++;
            break;
        case 'X':
            position->grilles[getGrilleIndex(positionIndex)] = NOIR;
            for (int i = 0; i < 9; i++)
            {
                position->cases[positionIndex + i] = NOIR;
            }
            positionIndex += 9;
            break;
        case 'O':
            position->grilles[getGrilleIndex(positionIndex)] = BLANC;
            for (int i = 0; i < 9; i++)
            {
                position->cases[positionIndex + i] = BLANC;
            }
            positionIndex += 9;
            break;
        default:
            if (isdigit(fen[fenIndex]))
            {
                positionIndex += (fen[fenIndex] - '0');
            }
            else
            {
                fprintf(stderr, "Erreur de notation FEN : %s\n", fen);
                exit(EXIT_FAILURE);
            }
        }
        fenIndex++;
    }
    fenIndex++;
    position->lastCoupId = convertMoveToIndex(&fen[fenIndex]);
    fenIndex += 3; // on saute le coup et l'espace
    position->trait = getCouleur(fen[fenIndex]);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <FEN> <time>\n", argv[0]);
        return 1;
    }
    char *fen = argv[1];
    int secondsLeft = atoi(argv[2]);

    T_Super_Morpion game;
    fillPositionFromFen(&game, fen);

    if (game.lastCoupId == -1)
    {
        char *move = convertIndexToMove(0);
        printf("%s", move);
        free(move);
        return 0;
    }

    T_LegalMoves legalMoves = getLegalMoves(&game);
    int legalMovesCount = legalMoves.count;
    free(legalMoves.legalMoves);

    int depth = DEFAULT_NEGAMAX_DEPTH;

    if ((secondsLeft < 600 && legalMovesCount > 10) || secondsLeft < 300)
    {
        depth = DEFAULT_NEGAMAX_DEPTH - 2;
    }

#ifdef DEBUG

    printf("> lastCoupId : %d c\n", game.lastCoupId);

    printSuperMorpion(&game, evaluate(&game));
    printf("Trait : %s\n", game.trait == NOIR ? "NOIR" : "BLANC");

    T_eval bestMove = negamax(game, depth, -1000, 1000, game.trait);

    printf("move : %s, eval : %d\n", convertIndexToMove(bestMove.moveId), bestMove.eval);

#else

    T_eval bestMove = negamax(game, depth, -1000, 1000, game.trait);

    char *move = convertIndexToMove(bestMove.moveId);
    printf("%s", move);
    free(move);

#endif
}
