#include "tp_mkpkit.h"
#include "tp_mkpsol.h"
#include "tp.h"

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
    if (((ObjRatio*)a)->ratio > ((ObjRatio*)b)->ratio) {
        return 1;
    }
    else if (((ObjRatio*)a)->ratio < ((ObjRatio*)b)->ratio) {
        return -1;
    }
    return 0;
}

int* getTableauOrdonne (tp_Mkp *mkp, int* ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);

    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        double coefObj = mkp->a[0][i];
        double poidsObj = 0;
        for (j = 1; j <= mkp->cc; j++) {
            poidsObj += mkp->a[j][i];
        }
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].ratio = coefObj/poidsObj;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), compareRatio);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    return ordre;
}

/**
Méthode permettant de remplir la solution avec tous les objets du sac
**/
void *remplirSac (tp_Mkp *mkp, Solution *s) {
    int i;
    for (i = 1; i <= mkp->n; i++) {
        Add(mkp, s, i);
	}
	return 0;
}

/**
Méthode permettant d'obtenir une solution réalisable à partir d'une solution non réalisable où tout les objets sont pris dans la solution
Renvoie 1 si on n'a pas trouvé de solution réalisable, 0 sinon
**/
int obtenirSolutionRealisable (tp_Mkp *mkp, Solution *s) {
    int isNoSolution = 0;
    int i = 0;
    //On récupère l'ordre par lequel on va retirer les objets
    int *ordre;
    ordre = (int *)malloc(sizeof (int) * (mkp->n + 1));
    ordre = getTableauOrdonne(mkp, ordre);

    printf("nbr d'objet : %d\n", mkp->n);
    //Check si on peut retirer l'objet, si oui on le retire
    printf("Nbr de cc non valid : %d\n", s->slack[0][0]);
    while (s->slack[0][0] > 0 && isNoSolution == 0) {
        i++;
        if (i > mkp->n) {
            isNoSolution = 1;
        }
        else if (isRemovePossible(mkp, s, ordre[i])) {
            printf("Do a drop : %d\n", ordre[i]);
            Drop(mkp, s, ordre[i]);
            printf("solx[%d] = %d\n", ordre[i], s->x[ordre[i]]);

        }
    }
    return isNoSolution;
}

Solution *parcoursVoisin (tp_Mkp *mkp, Solution *s) {
    int i, j;
    Solution *copieS = copieSolution(mkp, s);
    //On parcours une première fois la solution
    for (i = 1; i<= mkp->n; i++) {
        if (copieS->x[i] == 1) {//Si on a pris l'objet i dans la solution
            //On l'enlève
            Drop(mkp, copieS, i);
            //Puis on reparcours la liste des objets pour savoir quel objet remettre
            for (j = 1; j <= mkp->n; j++) {
                //Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                //et si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est supérieur à celui qu'on vient de retirer
                //et qu'on peut l'ajouter en respectant les contraintes
                if (j != i && copieS->x[j] == 0 && mkp->a[0][j] > mkp->a[0][i] && isAddPossible(mkp, copieS, j) == 1) {
                    printf("DROP/ADD\n");
                    //Alors on ajoute dans le sac
                    Add(mkp, copieS, j);
                    //On regarde si cette nouvelle solution est améliorante
                    if (copieS->objValue > s->objValue) {
                        //Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution améliorante.
                        return parcoursVoisin(mkp, copieS);
                    }
                    else {
                        Drop(mkp, copieS, j);
                    }
                }
            }
            //Si on n'a pas trouvé de solution améliorante, on rajoute l'objet qu'on a enlevé au départ et on passe à l'objet suivant
            Add(mkp, copieS, i);
        }
    }
    //return de la solution
    return s;
}



int main(int argc, char *argv[]) {
    tp_Mkp *mkp;
    Solution *s, *sAmeliorante = NULL;
    int i, j;
	if(argc != 3) {
		printf("Usage: programme nomFichierEntree nomFichierSortie\n");
		exit(0);
    }
    mkp = tp_load_mkp(argv[1]);
    s = alloc_sol(mkp);
	init_sol(s, mkp);
    printf("%d\n", mkp->a[6][0]);
	printf("Probleme sac a dos : nbr objets : %d, nbr cc : %d, nbr cd : %d\n", mkp->n, mkp->cc, mkp->cd);
	//On a initialisé la solution comme étant un sac vide, on va ajouter tous les objets pour avoir une solution non réalisable qui possède tous les objets
	//Puis on va retirer les objets 1 à 1 pour arriver à une solution réalisable
	remplirSac(mkp, s);

	printf("Object value : %d\n", s->objValue);
	printf("slack cc : %d\n", s->slack[0][0]);
	printf("slack cd : %d\n", s->slack[1][0]);

	//Il faut maintenant retirer les objets jusqu'à ce que la solution soit réalisable
	int isNoSolution = obtenirSolutionRealisable(mkp, s);

    printf("Pas de solution ? %d\n", isNoSolution);
    printf("Object value : %d\n", s->objValue);
	printf("slack cc : %d\n", s->slack[0][0]);
	printf("slack cd : %d\n", s->slack[1][0]);


    /******** HERE *********/
    /*Maintenant on recherche une solution améliorante*/


    //sAmeliorante = parcoursVoisin(mkp, s);

    /*printf("Ancienne value du sac : %d\n", s->objValue);

    if (sAmeliorante != NULL && s->objValue != 0) {
        printf("Nouvelle value du sac : %d\n", sAmeliorante->objValue);
        output_best_solution(sAmeliorante,argv[1],mkp->n,argv[2]);
        if(s->objValue == sAmeliorante->objValue)
            printf("Solution non ameliorante...\n");
    }
    else {
        if(!s->objValue) {
            printf("Pas de solution...\n");
            if(is_add_P(mkp)) record(argv[1], "w", "Pas de solution à ce problème...\n", argv[2]);
            else record(argv[1], "a", "Pas de solution à ce problème...\n", argv[2]);
        }
        else {
            output_best_solution(s,argv[1],mkp->n,argv[2]);
            printf("Solution non ameliorante...\n");
        }
    }

    free_sol(s);*/
    //free_sol(sAmeliorante);
    tp_del_mkp(mkp);

	return 0;
}
