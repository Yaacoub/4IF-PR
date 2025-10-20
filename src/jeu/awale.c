#include <stdio.h>
#include <stdlib.h>

#include "awale.h"

int * initialiser_jeu();
void afficher_plateau(const int * plateau);
int trouver_index(char key);
int * jouer_coup(int * plateau, char case_jouee);

int main(){
    int * plateau = initialiser_jeu();
    if (!plateau) return 1;

    afficher_plateau(plateau);
    char case_jouee;
    printf("Sélectionnez la case à jouer : ");
    scanf("%c",&case_jouee);
    printf("\n");
    plateau = jouer_coup(plateau,case_jouee);
    afficher_plateau(plateau);

    free(plateau);
    return 0;
}

//Fonctions
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

void afficher_plateau(const int * plateau){
    printf("\nPlateau de jeu\n\n");
    
    printf("a\tb\tc\td\te\tf\n");
    for (int i = 0; i < 6; ++i) printf("%d\t", plateau[i]);
    printf("\n");
    for (int i = 11; i >= 6; --i) printf("%d\t", plateau[i]);
    printf("\n");
    printf("A\tB\tC\tD\tE\tF\n");
}

int trouver_index(char key) {
    for(int i = 0; i < 12; i++) {
        if(conversion[i].key == key) {
            return conversion[i].value;
        }
    }
    return -1; // clé non trouvée
}

int * jouer_coup(int * plateau, char case_jouee){
    int index = trouver_index(case_jouee);
    if(index == -1) {
        return NULL; // case invalide
    }
    
    int nb_graines = plateau[index];
    plateau[index] = 0;
    int index_case = (index + 1)%12;
    while (nb_graines > 0){
        if(index_case != index){
            ++plateau[index_case];
            --nb_graines;
        }
        index_case = (index_case + 1)%12;
    }

    return plateau;
}

