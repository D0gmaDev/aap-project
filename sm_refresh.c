#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define DEFAULT_MINIMAX_DEPTH 6
#define CACHE_INVALID_VALUE -2435
#define UPPER_CASE_SHIFT -32

enum T_Couleur // on utilise une enum (on pourrait utiliser aussi des constantes symboliques)
{
    NOIR,
    BLANC,
    VIDE,
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
    int grillesCacheEval[9];
    int grilleEval;
} T_Eval_Cache;

typedef struct
{
    enum T_Couleur cases[81];
    enum T_Couleur grilles[9];
    enum T_Couleur trait;
    int lastCoupId;
    T_Eval_Cache *evalCache;
} T_Super_Morpion;

typedef struct
{
    int moveId;
    int eval;
} T_eval;

void fillNewGame(T_Super_Morpion *position, T_Eval_Cache *evalCache)
{
    for (int i = 0; i < 81; i++)
    {
        position->cases[i] = VIDE;
    }
    for (int i = 0; i < 9; i++)
    {
        position->grilles[i] = VIDE;
    }
    position->trait = BLANC;
    position->lastCoupId = -1;
    position->evalCache = evalCache;
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

void fillEvalCache(T_Eval_Cache *evalCache)
{
    for (int i = 0; i < 9; i++)
    {
        evalCache->grillesCacheEval[i] = CACHE_INVALID_VALUE;
    }
    evalCache->grilleEval = CACHE_INVALID_VALUE;
}

int convertMoveToIndex(char *move)
{
    int grille = move[0] - '1';
    // espace ignoré
    int colonne = move[2] - 'a';
    int ligne = move[3] - '1';

    return grille * 9 + ligne * 3 + colonne;
}

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

enum T_Couleur getTTTWinner(enum T_Couleur *cases)
{
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

int getBoardPopulationCount(enum T_Couleur *cases)
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

int isTTTBoardFull(enum T_Couleur *cases)
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

int isTTTBoardEmpty(enum T_Couleur *cases)
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

void proccessAlignment(int *count, int cases[3])
{
    if (count[VIDE] == 1)
    {
        if (count[NOIR] == 2)
        {
            *count++;
        }
        else if (count[BLANC] == 2)
        {
            *count--;
        }
    }
}

// Les blancs gagnent : c'est négatif, les noirs gagnent : c'est positif, égalité : 0
int evaluateTTT(enum T_Couleur cases[])
{
    enum T_Couleur winnerColor = getTTTWinner(cases);

    if (winnerColor != VIDE)
    {
        return (winnerColor == BLANC) ? -8 : 8;
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
    // cache check
    if (position->evalCache->grillesCacheEval[grilleIndex] != CACHE_INVALID_VALUE)
    {
        return position->evalCache->grillesCacheEval[grilleIndex];
    }

    // calcul
    int grilleStart = grilleIndex * 9;

    int eval = evaluateTTT(&position->cases[grilleStart]);

    position->evalCache->grillesCacheEval[grilleIndex] = eval;
    return eval;
}

int evaluate(T_Super_Morpion *position, int isMax)
{
    int macroEvaluation;

    // cache check
    if (position->evalCache->grilleEval != CACHE_INVALID_VALUE)
    {
        macroEvaluation = position->evalCache->grilleEval;
    }
    else
    {
        macroEvaluation = evaluateTTT(position->cases);
        position->evalCache->grilleEval = macroEvaluation;
    }

    int numberOfGrillesWon = 0;

    for (int i = 0; i < 9; i++)
    {
        if (position->grilles[i] == (isMax ? NOIR : BLANC))
        {
            numberOfGrillesWon++;
        }
    }

    int microEvaluation = 0;

    for (int i = 0; i < 9; i++)
    {
        microEvaluation += evaluateSmallMorpion(position, i);
    }

    return 40 * macroEvaluation + 3 * numberOfGrillesWon + microEvaluation;
}

// renvoie le tableau des coups possibles, avec -1 à la fin du tableau. La mémoire doit être libérée après utilisation
int *getLegalMoves(T_Super_Morpion *position)
{
    if (position->lastCoupId == -1)
    {
        int *legalMoves = malloc(82 * sizeof(int));
        assert(legalMoves != NULL);

        for (int i = 0; i < 81; i++)
        {
            legalMoves[i] = i;
        }
        legalMoves[81] = -1;
        return legalMoves;
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

        legalMoves[legalMovesCount] = -1;
        return legalMoves;
    }
    else
    {
        int *legalMoves = malloc(10 * sizeof(int));
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

        legalMoves[legalMovesCount] = -1;
        return legalMoves;
    }
}

void checkForNoLegalMoves(int *legalMoves)
{
    if (legalMoves[0] == -1)
    {
        printf("Il n'y a plus de coups possibles, la partie est terminée !\n");
        exit(0);
    }
}

void printSuperMorpion(T_Super_Morpion *position)
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
}

void invalidateCache(T_Super_Morpion *position, int grilleIndex)
{
    position->evalCache->grillesCacheEval[grilleIndex] = CACHE_INVALID_VALUE;
}

void makeMove(T_Super_Morpion *position, int move)
{
    position->cases[move] = position->trait;

    int grilleIndex = getGrilleIndex(move);

    invalidateCache(position, grilleIndex);

    enum T_Couleur winner = getTTTWinner(&position->cases[grilleIndex * 9]);

    if (winner != VIDE)
    {
        position->grilles[grilleIndex] = winner;
        position->evalCache->grilleEval = CACHE_INVALID_VALUE; // on invalide le cache de la grille globale

        for (int i = 0; i < 9; i++)
        {
            position->cases[grilleIndex * 9 + i] = winner;
        }
    }

    position->trait = getOther(position->trait);
    position->lastCoupId = move;
}

// une fonction récursive qui utilise l'algorithme minimax avec une profondeur "depth" pour trouver le meilleur coup à jouer
T_eval minimax(T_Super_Morpion position, int depth, int isMax)
{
    if (depth == 0)
    {
        return (T_eval){-1, evaluate(&position, isMax)};
    }

    int *legalMoves = getLegalMoves(&position);

    checkForNoLegalMoves(legalMoves);

    int bestEval = isMax ? -1000 : 1000;
    int bestMove = -3;

    for (int i = 0; legalMoves[i] != -1; i++)
    {
        int move = legalMoves[i];

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

    free(legalMoves);
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
            fprintf(file, "<TD bgcolor=\"white\">\n<TABLE border=\"0\" cellspacing=\"10\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"black\">\n");

            for (int j = 0; j < 3; j++)
            {
                fprintf(file, "<TR>\n");
                fprintf(file, "<TD bgcolor=\"white\">%c</TD>\n", getSymbol(position->cases[i * 9 + j * 3]));
                fprintf(file, "<TD bgcolor=\"white\">%c</TD>\n", getSymbol(position->cases[i * 9 + j * 3 + 1]));
                fprintf(file, "<TD bgcolor=\"white\">%c</TD>\n", getSymbol(position->cases[i * 9 + j * 3 + 2]));
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

// print un tableau de positions jusqu'à -1
void printPositionArray(int *array)
{
    int i = 0;
    while (array[i] != -1)
    {
        char *move = convertIndexToMove(array[i++]);
        printf("%s | ", move);
        free(move);
    }
    printf("\n");
}

// vérifie si un nombre est dans une liste d'entiers se terminant par -1
int isInArray(int *array, int number)
{
    int i = 0;
    while (array[i] != -1)
    {
        if (array[i] == number)
        {
            return 1;
        }
        i++;
    }
    return 0;
}

void playSuperMorpion(int minimaxDepth)
{
    T_Eval_Cache evalCache;
    fillEvalCache(&evalCache);

    T_Super_Morpion position;
    fillNewGame(&position, &evalCache);

    while (1)
    {
        printSuperMorpion(&position);
        drawPositionToFile(&position);

        int *legalMoves = getLegalMoves(&position);

        printf("Coups possibles : ");
        printPositionArray(legalMoves);

        char move[10];
        int moveIndex = -2;

        while (!isInArray(legalMoves, moveIndex))
        {
            printf("Entrez votre coup : ");
            fgets(move, sizeof(move), stdin);
            moveIndex = convertMoveToIndex(move);
        }

        free(legalMoves);

        makeMove(&position, moveIndex);

        printSuperMorpion(&position);
        drawPositionToFile(&position);

        if (getTTTWinner(position.grilles) == BLANC)
        {
            printf("Vous avez gagné !\n");
            return;
        }

        // vérifie si l'ordinateur peut jouer au moins un coup
        int *computerLegalMoves = getLegalMoves(&position);
        printf("Coups possibles pour l'ordinateur : ");
        printPositionArray(computerLegalMoves);
        checkForNoLegalMoves(computerLegalMoves);
        free(computerLegalMoves);

        int start = clock();

        T_eval eval = minimax(position, minimaxDepth, /* les noirs (l'ordinateur) maximisent */ 1);

        int end = clock();

        char *computerMove = convertIndexToMove(eval.moveId);
        printf("Coup joué par l'ordinateur: %s (eval: %d, temps: %.2fs)\n", computerMove, eval.eval, (double)(end - start) / CLOCKS_PER_SEC);
        free(computerMove);

        makeMove(&position, eval.moveId);

        if (getTTTWinner(position.grilles) == NOIR)
        {
            printSuperMorpion(&position);
            drawPositionToFile(&position);
            printf("L'ordinateur a gagné !\n");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    int miniMaxDepth = argc > 1 ? atoi(argv[1]) : DEFAULT_MINIMAX_DEPTH;
    playSuperMorpion(miniMaxDepth);
    return 0;
}
