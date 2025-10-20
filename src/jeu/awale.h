//Interface pour le fichier contenant le jeu

#ifndef AWALE_H
#define AWALE_H

//Structure pour contenir la conversion entre nom des case et valeur dans le tableau
typedef struct{
    char key;
    int value;
} KeyValuePair;

static const KeyValuePair conversion[] = {
    {'a', 0}, {'b', 1}, {'c', 2}, {'d', 3}, {'e', 4}, {'f', 5},
    {'A', 11}, {'B', 10}, {'C', 9}, {'D', 8}, {'E', 7}, {'F', 6}
};

/* 
    Alloue et initialise le plateau.
    Renvoie un pointeur sur un tableau d'entiers de taille 12 ou NULL en cas d'erreur.
*/
int * initialiser_jeu(void);

/* Affiche le plateau dans la console */
void afficher_plateau(const int *plateau);

/* Parcours la structure de conversion pour convertir le nom d'une case en l'index correspondant dans le tableau */
int trouver_index(char key);

/* Permet de jouer un coup */
int * jouer_coup(int * plateau, char case_jouee);



#endif /* AWALE_H */
