#ifndef TP_H_
#define TP_H_
#include "tp_mkpsol.h"

    /*
     * Ecrit dans un fichier les solutions possibles.
     */
    void output_best_solution(Solution *s, char *nomFichierEntree, int nbVariables, char *nomFichierSortie);
    void record(char *input_file, char *option, char *string, char *output_file);

    /*
     * Verifie si les contraintes de demande impossible à réaliser.
     */
    int is_add_P(tp_Mkp *mkp);
#endif //TP
