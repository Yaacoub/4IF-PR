#include <stdio.h>
#include <stdlib.h>

int * initialiser_jeu();
void afficher_plateau(int * plateau);

int main(){
    int * plateau = initialiser_jeu();
    if (!plateau) return 1;

    /* exemple : afficher le plateau */
    afficher_plateau(plateau);

    free(plateau);
    return 0;
}

//Fonctions

/* allouer et initialiser le plateau de jeu
   renvoie un pointeur vers un tableau de 10 entiers alloué sur le tas
   l'appelant est responsable de free() lorsque le plateau n'est plus utilisé */
int * initialiser_jeu(){
    int *plateau = malloc(12 * sizeof(int));
    if (!plateau) {
        perror("malloc");
        return NULL;
    }
    /* remplir chaque case avec 4 graines (valeur typique pour l'Awalé) */
    for (int i = 0; i < 12; ++i) plateau[i] = 4;
    return plateau;
}

void afficher_plateau(int * plateau){
    printf("Plateau de jeu\n\n");
    
    printf("a\tb\tc\td\te\tf\n");
    for (int i = 0; i < 6; ++i) printf("%d\t", plateau[i]);
    printf("\n");
    for (int i = 0; i < 6; ++i) printf("%d\t", plateau[i]);
    printf("\n");
    printf("A\tB\tC\tD\tE\tF\n");
}

