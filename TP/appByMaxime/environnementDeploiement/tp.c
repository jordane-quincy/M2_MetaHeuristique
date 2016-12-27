#include "tp.h"
/****************************************************************
 *                      LOG FUNCTIONS
****************************************************************/
/**
output_best_solution : Génération du fichier texte de sortie.
s : La meilleure solution trouvée.
nomFichierEntree : Le nom du fichier en entrée ainsi que son extension.
nbVariables : Le nombre de variables (d'objets) manipulées.
nomFichierSortie : Le nom du fichier de sortie ainsi que son extension.
*/
void output_best_solution(Solution *s, char *nomFichierEntree, int nbVariables, char *nomFichierSortie){
    int j;
    FILE* fichier = fopen(nomFichierSortie, "w"); /* option "w" afin de reset le fichier si présent*/

    /*si on a bien un pointeur vers le fichier*/
    if(fichier != NULL){
        /* ligne 1 : nom du fichier en entrée*/
        fprintf(fichier, "%s\n", nomFichierEntree);
        /* ligne 2 : nb variables*/
        fprintf(fichier, "%d\n", nbVariables);
        /* ligne 3 : valeur de xj pour chaque j de N (1 si l'objet est pris, 0 sinon)*/
        for (j = 1; j <= nbVariables; j++) {
            fprintf(fichier, "%d ", s->x[j]);
        }
        fprintf(fichier, "\n");
        /* ligne 4 : valeur de la solution*/
        fprintf(fichier, "%d", s->objValue);

        /* fermeture du fichier*/
        fclose(fichier);
    } else {
        printf("Impossible d'ecrire le fichier de sortie : %s", nomFichierSortie);
    }
}
void record(char *input_file, char *option, char *string, char *output_file){
    FILE* file;
    if(strcmp(option, "w") != 0 && strcmp(option, "a") != 0) option = "w";
    file = fopen(output_file, option);
    if(file != NULL){
        fprintf(file, "%s\n", input_file);
        fprintf(file, "%s\n", string);
        /*Fermeture du fichier*/
        fclose(file);
    }
}
/*********************************************************************
 * Fonction verification problème possible (contraintes de demande).
 * *******************************************************************/
int is_add_P(tp_Mkp *mkp){
    int i, j, somme_coef;
    for(i = mkp->cc + 1;i <= mkp->cc + mkp->cd;i++){
        somme_coef = 0;
        for(j = 1;j <= mkp->n;j++){
            somme_coef += mkp->a[i][j];
        }
        if(somme_coef < mkp->a[i][0]) return 0;
    }
    return 1;
}
