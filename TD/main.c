#include "mkpkit.h"
#include "mkpsol.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct {
     int indexObj;
     double ratio;
} ObjRatio;

ObjRatio* alloc_tab(int nbrVar) {
    ObjRatio *tab;
    tab = (ObjRatio *)malloc(sizeof (ObjRatio) * (nbrVar + 1));
    return tab;
}


int compareRatio (const void * a, const void * b)
{
    if (((ObjRatio*)a)->ratio < ((ObjRatio*)b)->ratio) {
        return 1;
    }
    else if (((ObjRatio*)a)->ratio > ((ObjRatio*)b)->ratio) {
        return -1;
    }
    return 0;
}

int* getTableauOrdonne (Mkp *mkp, int* ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);

    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        double coefObj = mkp->a[0][i];
        double poidsObj = 0;
        for (j = 1; j <= mkp->m; j++) {
            poidsObj += mkp->a[j][i];
        }
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].ratio = coefObj/poidsObj;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), compareRatio);

    /*for (i = 1; i <= 8; i++) {
        printf("Obj index : %d, obj ratio : %lf\n", tabOrdonne[i].indexObj, tabOrdonne[i].ratio);
    }*/
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    return ordre;
}

Solution *parcoursVoisin (Mkp *mkp, Solution *s) {
    int i, j;
    Solution *copieS = copieSolution(mkp, s);
    //On parcours une première fois la solution
    for (i = 1; i<= mkp->n; i++) {
        if (copieS->x[i] == 1) {//Si on a pris l'objet i dans la solution
            //On l'enlève
            Drop(mkp, copieS, i);
            //Puis on reparcours la liste des objets pour savoir quel objet remettre
            for (j = 1; j <= mkp->n; j++) {
                //Si l'objet j n'est pas dans le sac, et qu'on peut l'ajouter en respectant les contraintes et que ce n'est pas l'objet qu'on vient de retirer
                if (copieS->x[j] == 0 && Is_Add_F(mkp, copieS, j) == 1 && j != i) {
                    //Alors on ajoute dans le sac
                    Add(mkp, copieS, j);
                    //On regarde si cette nouvelle solution est améliorante
                    if (copieS->objValue > s->objValue) {
                        //Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution améliorante.
                        return parcoursVoisin(mkp, copieS);
                    }
                }
            }
        }
    }
    //return de la solution
    return s;
}

int main(int argc, char *argv[]) {
	Mkp *mkp;
	Solution *s;
	Solution *sAmeliorante;
	int *ordre;
	int i;
	if(argc != 2) {
		printf("Usage: programme fichier\n");
		exit(0);
	}
	mkp = load_mkp(argv[1]);
	s = alloc_sol(mkp);
	init_sol(s, mkp);
	ordre = (int *)malloc(sizeof (int) * (mkp->n + 1));
    ordre = getTableauOrdonne(mkp, ordre);
    for (i = 1; i <= mkp->n; i++) {
        if (Is_Add_F(mkp, s, ordre[i]) == 1) {
            Add(mkp, s, ordre[i]);
        }
    }

    sAmeliorante = parcoursVoisin(mkp, s);


    printf("Ancienne value du sac : %d\n", s->objValue);
    free_sol(s);
    if (sAmeliorante != NULL) {
        printf("Nouvelle value du sac : %d\n", sAmeliorante->objValue);
        free_sol(sAmeliorante);
    }
    else {
        printf("Pas de solution ameliorante");
    }

	del_mkp(mkp);

	return 0;
}
