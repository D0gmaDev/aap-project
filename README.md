
# Fil Rouge 2023 AAP : Equipe Piks & Co
*Maieul Brisquet, Baptiste Drouet, David Marembert & Manon Perez*

## Introduction
Ce projet s'inscrit dans le cadre du projet AAP **Fil Rouge 2023**. Il consiste en l'implémentation de trois programmes : `tttree`, `sm-refresh`, et `sm-bot`. Chaque programme a des exigences et des fonctionnalités spécifiques liées aux jeux de morpion et de super-morpion.

## Organisation
Le projet doit être réalisé en équipes de 4 étudiants maximum, appartenant au même groupe de TD. Chaque équipe doit soumettre une archive unique sur Moodle, qui doit comprendre les éléments suivants :

- Les fichiers du code source du programme
- Un fichier makefile pour compiler le programme
- Un rapport écrit (Compte Rendu) décrivant l'organisation du programme et l'organisation de l'équipe, y compris la répartition des tâches, la planification, les défis rencontrés, etc.

## Évaluation
Le projet sera évalué sur la base des critères suivants :

- Qualité du code livré et sa compétitivité
- Qualité du rapport écrit
- Qualité de la présentation lors de la soutenance

## Programme 1 : tttree
Le programme `tttree` génère la représentation graphique d'un arbre de décision tic-tac-toe en utilisant un algorithme minimax de base. La position initiale de l'arbre est déterminée à partir d'un argument de la ligne de commande au format FEN. Le programme doit produire le fichier dot généré sur la sortie standard.

Pour tester ce programme, executez le script `tttree_test.sh` :

```bash
make  tttree
./tttree_test.sh
```

Si vous voulez obtenir l'arbre de décision à partir d'une position de départ particulière, executez directement `tttree` avec `./tttree`.

## Programme 2 : sm-refresh
Le programme `sm-refresh` permet de jouer au jeu du super-morpion contre l'ordinateur en utilisant un algorithme minimax de base. Les mouvements sont entrés en notation algébrique sur l'entrée standard. Le programme doit afficher les positions à la fois dans un format pseudo-graphique sur la sortie standard et dans un format graphique dans un fichier nommé `g.png` en utilisant Graphviz.

La sortie graphique peut être lue à l'aide du fichier `refresh.html` fourni par les enseignants d'AAP.

## Programme 3 : sm-bot
Le programme `sm-bot` calcule le meilleur coup pour le jeu du super-morpion basé sur la position d'entrée en utilisant les principes de négamax et d'élagage alpha-beta. Il devra fournir le coup calculé sur la sortie standard.

## Tournoi
Un tournoi de bots sera organisé en utilisant le programme `sm-bot`, avec un contrôle de temps de 15 minutes plus 30 secondes par coup. Le tournoi déterminera un classement qui contribuera à la note finale du projet.

  
*Pour plus de détails et d'exemples, veuillez vous référer au Cahier des Charges.*