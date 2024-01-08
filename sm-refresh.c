#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "super_morpion.h" // toutes les fonctions et structures relatives au super morpion

#define DEFAULT_MINIMAX_DEPTH 6

// implémentation de super_morpion.h spécifique à ce livrable
int convertMoveToIndex(char *move)
{
    int grille = move[0] - '1';
    // espace move[1] ignoré
    int colonne = move[2] - 'a';
    int ligne = move[3] - '1';

    return grille * 9 + ligne * 3 + colonne;
}

// implémentation de super_morpion.h spécifique à ce livrable
char *convertIndexToMove(int index)
{
    char *move = malloc(5 * sizeof(char));
    assert(move != NULL);

    int grille = getGrilleIndex(index);
    int caseIndex = getCaseIndex(index);
    int colonne = caseIndex % 3;
    int ligne = caseIndex / 3;

    move[0] = grille + '1';
    move[1] = ' ';
    move[2] = colonne + 'a';
    move[3] = ligne + '1';
    move[4] = '\0';
    return move;
}

void proccessAlignmentAndClear(int *i, int count[3])
{
    if (count[VIDE] == 1)
    {
        if (count[NOIR] == 2)
        {
            (*i)++;
        }
        else if (count[BLANC] == 2)
        {
            (*i)--;
        }
    }

    count[NOIR] = 0;
    count[BLANC] = 0;
    count[VIDE] = 0;
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

    // calculer le nombre d'alignement de deux pions de la même couleur avec un pion vide
    int numberOfAlignments = 0;

    // Vérification des lignes et colonnes
    for (int i = 0; i < 3; i++)
    {
        int rowIndex = i * 3;
        int colIndex = i;

        for (int j = 0; j < 3; j++)
        {
            enum T_Couleur couleur = cases[rowIndex + j];
            count[couleur]++;
        }

        proccessAlignmentAndClear(&numberOfAlignments, count);

        // Vérification des colonnes
        for (int j = 0; j < 3; j++)
        {
            enum T_Couleur couleur = cases[colIndex + j * 3];
            count[couleur]++;
        }

        proccessAlignmentAndClear(&numberOfAlignments, count);
    }

    // Vérification des diagonales
    count[cases[0]]++;
    count[cases[4]]++;
    count[cases[8]]++;

    proccessAlignmentAndClear(&numberOfAlignments, count);

    count[cases[2]]++;
    count[cases[4]]++;
    count[cases[6]]++;

    proccessAlignmentAndClear(&numberOfAlignments, count);

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

void checkForNoLegalMoves(T_LegalMoves *legalMoves)
{
    if (legalMoves->count == 0)
    {
        printf("Il n'y a plus de coups possibles, la partie est terminée !\n");
        exit(0);
    }
}

// une fonction récursive qui utilise l'algorithme minimax avec une profondeur "depth" pour trouver le meilleur coup à jouer
T_eval minimax(T_Super_Morpion position, int depth, int isMax)
{
    if (depth == 0)
    {
        // au niveau de recherche 0, on ne propose pas de coup (-1) mais on évalue juste la position courante
        return (T_eval){-1, evaluate(&position)};
    }

    enum T_Couleur winner = getTTTWinner(position.grilles);

    if (winner == NOIR)
        return (T_eval){-1, 642};
    else if (winner == BLANC)
        return (T_eval){-1, -642};

    T_LegalMoves legalMoves = getLegalMoves(&position);

    if (legalMoves.count == 0) // match nul
    {
        free(legalMoves.legalMoves);
        return (T_eval){-1, 0};
    }

    int bestEval = isMax ? -1000 : 1000;
    int bestMove = -3;

    for (int i = 0; i < legalMoves.count; i++)
    {
        int move = legalMoves.legalMoves[i];

        T_Super_Morpion newPosition = position;
        makeMove(&newPosition, move);

        T_eval childEval = minimax(newPosition, depth - 1, 1 - isMax);

        if (isMax)
        {
            if (childEval.eval > bestEval)
            {
                bestEval = childEval.eval;
                bestMove = move;
            }
        }
        else
        {
            if (childEval.eval < bestEval)
            {
                bestEval = childEval.eval;
                bestMove = move;
            }
        }
    }

    free(legalMoves.legalMoves);
    return (T_eval){bestMove, bestEval};
}

char *getImagePath()
{
    char *smPath = getenv("SMPATH");
    return smPath == NULL ? "g.png" : smPath;
}

void drawPositionToFile(T_Super_Morpion *position)
{
    char *imagePath = getImagePath();

    char *dotPath = malloc(strlen(imagePath) + 5);
    assert(dotPath != NULL);
    strcpy(dotPath, imagePath);
    strcat(dotPath, ".dot");

    FILE *file = fopen(dotPath, "w");
    assert(file != NULL);

    int lastCoupId = position->lastCoupId;

    fprintf(file, "digraph  {\na0 [shape=none label=<\n<TABLE border=\"0\" cellspacing=\"10\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"black\">\n");

    for (int i = 0; i < 9; i++)
    {
        if (i % 3 == 0) // on ouvre une ligne
        {
            fprintf(file, "<TR>\n");
        }

        enum T_Couleur winner = position->grilles[i];

        if (winner == VIDE)
        {
            fprintf(file, "<TD bgcolor=\"%s\">\n<TABLE border=\"0\" cellspacing=\"10\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"black\">\n", lastCoupId % 9 == i ? "grey" : "white");

            for (int j = 0; j < 3; j++)
            {
                int caseIndex = i * 9 + j * 3;
                fprintf(file, "<TR>\n");
                fprintf(file, "<TD bgcolor=\"%s\">%c</TD>\n", caseIndex == lastCoupId ? "grey" : "white", getSymbol(position->cases[caseIndex]));
                fprintf(file, "<TD bgcolor=\"%s\">%c</TD>\n", caseIndex + 1 == lastCoupId ? "grey" : "white", getSymbol(position->cases[caseIndex + 1]));
                fprintf(file, "<TD bgcolor=\"%s\">%c</TD>\n", caseIndex + 2 == lastCoupId ? "grey" : "white", getSymbol(position->cases[caseIndex + 2]));
                fprintf(file, "</TR>\n");
            }
        }
        else
        {
            char *color = (winner == BLANC) ? "white" : "black";
            char *oppositeColor = (winner == BLANC) ? "black" : "white";

            fprintf(file, "<TD bgcolor=\"%s\">\n<TABLE border=\"0\" cellspacing=\"10\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"%s\">\n", oppositeColor, color);

            for (int j = 0; j < 3; j++)
            {
                fprintf(file, "<TR>\n");
                fprintf(file, "<TD bgcolor = \"%s\"> &nbsp;</TD>\n", color);
                fprintf(file, "<TD bgcolor = \"%s\"> &nbsp;</TD>\n", color);
                fprintf(file, "<TD bgcolor = \"%s\"> &nbsp;</TD>\n", color);
                fprintf(file, "</TR>\n");
            }
        }
        fprintf(file, "</TABLE>\n</TD>\n");

        if ((i + 1) % 3 == 0) // on coupe les lignes
        {
            fprintf(file, "</TR>\n");
        }
    }

    fprintf(file, "</TABLE>\n>];\n}");
    fclose(file);

    char *dotCommand = malloc(strlen(dotPath) + strlen(imagePath) + 15);
    assert(dotCommand != NULL);
    sprintf(dotCommand, "dot -Tpng %s -o %s", dotPath, imagePath);
    system(dotCommand);

    free(dotPath);
    free(dotCommand);
}

void playSuperMorpion(int minimaxDepth)
{
    T_Super_Morpion position;
    fillNewGame(&position, BLANC);

    while (1)
    {
        printSuperMorpion(&position, evaluate(&position));
        drawPositionToFile(&position);

        T_LegalMoves legalMoves = getLegalMoves(&position);

        checkForNoLegalMoves(&legalMoves);

        printf("Coups possibles : ");
        printLegalMovesArray(&legalMoves);

        char move[10];
        int moveIndex = -2;

        while (!isInLegalMoves(&legalMoves, moveIndex))
        {
            printf("Entrez votre coup : ");
            fgets(move, sizeof(move), stdin);
            moveIndex = convertMoveToIndex(move);
        }

        free(legalMoves.legalMoves);

        makeMove(&position, moveIndex);

        printSuperMorpion(&position, evaluate(&position));
        drawPositionToFile(&position);

        if (getTTTWinner(position.grilles) == BLANC)
        {
            printf("Vous avez gagné !\n");
            return;
        }

        // vérifie si l'ordinateur peut jouer au moins un coup
        T_LegalMoves computerLegalMoves = getLegalMoves(&position);

        if (getenv("DEBUG") != NULL)
        {
            printf("[DEBUG] Coups possibles pour l'ordinateur : ");
            printLegalMovesArray(&computerLegalMoves);
        }

        checkForNoLegalMoves(&computerLegalMoves);
        free(computerLegalMoves.legalMoves);

        int start = clock();

        T_eval eval = minimax(position, minimaxDepth, /* les noirs (l'ordinateur) maximisent */ 1);

        int end = clock();

        char *computerMove = convertIndexToMove(eval.moveId);
        printf("Coup joué par l'ordinateur: %s (maxEval: %d, temps: %.2fs)\n", computerMove, eval.eval, (double)(end - start) / CLOCKS_PER_SEC);
        free(computerMove);

        makeMove(&position, eval.moveId);

        if (getTTTWinner(position.grilles) == NOIR)
        {
            printSuperMorpion(&position, evaluate(&position));
            drawPositionToFile(&position);
            printf("L'ordinateur a gagné !\n");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    int miniMaxDepth = argc > 1 ? atoi(argv[1]) : DEFAULT_MINIMAX_DEPTH;
    assert(miniMaxDepth > 0);

    if (getenv("DEBUG") != NULL)
    {
        printf("[DEBUG] Profondeur de recherche du minimax : %d\n", miniMaxDepth);
    }

    playSuperMorpion(miniMaxDepth);
    return 0;
}
