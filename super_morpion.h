#ifndef _SUPER_MORPION_H_
#define _SUPER_MORPION_H_

#define UPPER_CASE_SHIFT -32

enum T_Couleur // on utilise une enum (on pourrait utiliser aussi des constantes symboliques)
{
    NOIR,
    BLANC,
    VIDE,
};

enum T_Couleur getCouleur(char c);

char getSymbol(enum T_Couleur couleur);

enum T_Couleur getOther(enum T_Couleur couleur);

typedef struct
{
    enum T_Couleur cases[81];
    enum T_Couleur grilles[9];
    enum T_Couleur trait;
    int lastCoupId;
} T_Super_Morpion;

typedef struct
{
    int moveId;
    int eval;
} T_eval;

typedef struct
{
    int *legalMoves;
    int count;
} T_LegalMoves;

void fillNewGame(T_Super_Morpion *position, enum T_Couleur firstPlayer);

int getGrilleIndex(int positionIndex);

int getCaseIndex(int positionIndex);

int convertMoveToIndex(char *move);

char *convertIndexToMove(int index);

int getBoardPopulationCount(enum T_Couleur cases[]);

int isTTTBoardFull(enum T_Couleur cases[]);

int isTTTBoardEmpty(enum T_Couleur cases[]);

enum T_Couleur getTTTWinner(enum T_Couleur cases[]);

// renvoie la structure des coups possibles. Le pointeur moves.legalMoves doit être libéré après utilisation
T_LegalMoves getLegalMoves(T_Super_Morpion *position);

void printSuperMorpion(T_Super_Morpion *position, int evaluation);

void printLegalMovesArray(T_LegalMoves *legalMoves);

int isInLegalMoves(T_LegalMoves *legalMoves, int moveId);

#endif // _SUPER_MORPION_H_