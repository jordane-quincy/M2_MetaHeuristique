#ifndef MKPSOL_H_
#define MKPSOL_H_

#include "tp_mkpkit.h"

typedef struct {
     int objValue ;
     int *x;
     int **slack;
} Solution;

        Solution *alloc_sol(tp_Mkp *mkp);

        void free_sol(Solution *sol);						        /* libération memoire de la solution */

        void init_sol(Solution *sol, tp_Mkp *mkp);			        /* initialisation de la solution */

        int isAddPossible(tp_Mkp *mkp, Solution *sol, int j);		/* verifie si l'objet j peut etre ajoute dans la solution */

        int isRemovePossible(tp_Mkp *mkp, Solution *sol, int j);	/* verifie si l'objet j peut etre retirer de la solution */

        void Add(tp_Mkp *mkp, Solution *sol, int j);			    /* ajoute l'objet j dans la solution */

        void Drop(tp_Mkp *mkp, Solution *sol, int j);			    /* retire l'objet j de la solution j */

        Solution *copieSolution (tp_Mkp *mkp, Solution *sol);       /* effectue une copie d'une solution */

#endif
