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
        for (j = 1; j <= mkp->cd; j++) {
            poidsObj += mkp->a[mkp->cc + j][i];
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

Solution *parcoursVoisin (tp_Mkp *mkp, Solution *s) {
    int i, j;
    Solution *copieS = copieSolution(mkp, s);
    //On parcours une premi�re fois la solution
    for (i = 1; i<= mkp->n; i++) {
        if (copieS->x[i] == 1) {//Si on a pris l'objet i dans la solution
            //On l'enl�ve
            Drop(mkp, copieS, i);
            //Puis on reparcours la liste des objets pour savoir quel objet remettre
            for (j = 1; j <= mkp->n; j++) {
                //Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                //et si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est sup�rieur � celui qu'on vient de retirer
                //et qu'on peut l'ajouter en respectant les contraintes
                if (j != i && copieS->x[j] == 0 && mkp->a[0][j] > mkp->a[0][i] && isAddPossible(mkp, copieS, j) == 1) {
                    printf("DROP/ADD\n");
                    //Alors on ajoute dans le sac
                    Add(mkp, copieS, j);
                    //On regarde si cette nouvelle solution est am�liorante
                    if (copieS->objValue > s->objValue) {
                        //Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution am�liorante.
                        return parcoursVoisin(mkp, copieS);
                    }
                    else {
                        Drop(mkp, copieS, j);
                    }
                }
            }
            //Si on n'a pas trouv� de solution am�liorante, on rajoute l'objet qu'on a enlev� au d�part et on passe � l'objet suivant
            Add(mkp, copieS, i);
        }
    }
    //return de la solution
    return s;
}

int main(int argc, char *argv[]) {
    tp_Mkp *mkp;
    Solution *s, *sAmeliorante = NULL;
    int *ordre, i;
	if(argc != 3) {
		printf("Usage: programme nomFichierEntree nomFichierSortie\n");
		exit(0);
    }
    mkp = tp_load_mkp(argv[1]);
    s = alloc_sol(mkp);
	init_sol(s, mkp);

	//On a initialis� la solution comme �tant un sac vide, on va ajouter tous les objets pour avoir une solution non r�alisable qui poss�de tous les objets
	//Puis on va retirer les objets 1 � 1 pour arriver � une solution r�alisable
	for (i = 1; i <= mkp->n; i++) {
        Add(mkp, s, i);
	}

	printf("Object value : %d\n", s->objValue);
	printf("slack cc : %d\n", s->slack[0][0]);
	printf("slack cd : %d\n", s->slack[1][0]);

	//Il faut maintenant retirer les objets jusqu'� ce que la solution soit r�alisable

	//TODO do what is in the comment juste au dessus

	ordre = (int *)malloc(sizeof (int) * (mkp->n + 1));
    ordre = getTableauOrdonne(mkp, ordre);

    //Check si on peut retirer l'objet, si oui on le retire

    /******** HERE *********/
    //ici on ajoutait l'objet si possible
    /*if(is_add_P(mkp))
    for (i = 1; i <= mkp->n; i++) {
        if (Is_Add_F(mkp, s, ordre[i]) == 1) {
            Add(mkp, s, ordre[i]);
        }
    }
    else {
        printf("Contraintes de demande impossible a resoudre !\n");
        record(argv[1], "w", "Contraintes de demande impossible a resoudre !\n", argv[2]);
    }*/

    //sAmeliorante = parcoursVoisin(mkp, s);

    printf("Ancienne value du sac : %d\n", s->objValue);

    if (sAmeliorante != NULL && s->objValue != 0) {
        printf("Nouvelle value du sac : %d\n", sAmeliorante->objValue);
        output_best_solution(sAmeliorante,argv[1],mkp->n,argv[2]);
        if(s->objValue == sAmeliorante->objValue)
            printf("Solution non ameliorante...\n");
    }
    else {
        if(!s->objValue) {
            printf("Pas de solution...\n");
            if(is_add_P(mkp)) record(argv[1], "w", "Pas de solution � ce probl�me...\n", argv[2]);
            else record(argv[1], "a", "Pas de solution � ce probl�me...\n", argv[2]);
        }
        else {
            output_best_solution(s,argv[1],mkp->n,argv[2]);
            printf("Solution non ameliorante...\n");
        }
    }

    free_sol(s);
    //free_sol(sAmeliorante);
    tp_del_mkp(mkp);

	return 0;
}
