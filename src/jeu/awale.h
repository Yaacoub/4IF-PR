/* awale.h — interface minimale pour le jeu Awalé
 * Commentaires en français comme demandé.
 */

#ifndef AWALE_H
#define AWALE_H

#include <stddef.h>

/* Nombre total de cases (2 côtés × 6) */
#define AWALE_CASES 12
#define AWALE_CASES_PER_SIDE 6

/* Alloue et initialise le plateau.
 * Renvoie un pointeur sur un tableau d'entiers de taille 12 ou NULL en cas d'erreur.
 * L'appelant doit free() ce pointeur.
 */
int * initialiser_jeu(void);

/* Affiche le plateau dans la console (outil de debug). */
void afficher_plateau(const int *plateau);

/* Libère le plateau (simple wrapper) */
void liberer_plateau(int *plateau);

#endif /* AWALE_H */
