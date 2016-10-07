#ifndef MKPSOL_H_
#define MKPSOL_H_

#include "mkpkit.h"

typedef struct {
     int objValue ;
     int *x;
     int *slack;
} Solution;

		Solution *alloc_sol(Mkp *mkp);						/* allocation memoire d'une solution pour le mkp */

		 void free_sol(Solution *sol);						/* libération memoire de la solution */

		 void init_sol(Solution *sol, Mkp *mkp);			/* initialisation de la solution */

		 int Is_Add_F(Mkp *mkp, Solution *sol, int j);		/* verifie si l'objet j peut etre ajoute dans la solution */

		 void Add(Mkp *mkp, Solution *sol, int j);			/* ajoute l'objet j dans la solution */

		 void Drop(Mkp *mkp, Solution *sol, int j);			/* retire l'objet j de la solution j */

#endif
