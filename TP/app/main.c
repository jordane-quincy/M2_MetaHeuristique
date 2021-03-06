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
                if (j != i && copieS->x[j] == 0 && mkp->a[0][j] > mkp->a[0][i] && Is_Add_F(mkp, copieS, j) == 1) {
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
            //on rajoute l'objet pour en tester un autre si on n'a pas trouv� de meilleures solutions
            Add(mkp, copieS, i);
        }
    }
    //return de la solution
    return s;
}

/**
output_best_solution : G�n�ration du fichier texte de sortie.
s : La meilleure solution trouv�e.
nomFichierEntree : Le nom du fichier en entr�e ainsi que son extension.
nbVariables : Le nombre de variables (d'objets) manipul�es.
nomFichierSortie : Le nom du fichier de sortie ainsi que son extension.
*/
void output_best_solution(Solution *s, char *nomFichierEntree, int nbVariables, char *nomFichierSortie){
    int j;
    FILE* fichier = fopen(nomFichierSortie, "w+"); // option "w+" afin de reset le fichier si pr�sent

    //si on a bien un pointeur vers le fichier
    if(fichier != NULL){
        // ligne 1 : nom du fichier en entr�e
        fprintf(fichier, "%s\n", nomFichierEntree);
        // ligne 2 : nb variables
        fprintf(fichier, "%d\n", nbVariables);
        // ligne 3 : valeur de xj pour chaque j de N (1 si l'objet est pris, 0 sinon)
        for (j = 1; j <= nbVariables; j++) {
            fprintf(fichier, "%d ", s->x[j]);
        }
        fprintf(fichier, "\n");
        // ligne 4 : valeur de la solution
        fprintf(fichier, "%d", s->objValue);

        // fermeture du fichier
        fclose(fichier);
    } else {
        printf("Impossible d'ecrire le fichier de sortie : %s", nomFichierSortie);
    }
}

int main(int argc, char *argv[]) {
	Mkp *mkp;
	Solution *s;
	Solution *sAmeliorante;
	int *ordre;
	int i;
	if(argc != 3) {
		printf("Usage: programme nomFichierEntree nomFichierSortie\n");
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
        output_best_solution(sAmeliorante,argv[1],mkp->n,argv[2]);
        free_sol(sAmeliorante);
    }
    else {
        printf("Pas de solution ameliorante");
    }

	del_mkp(mkp);

	return 0;
}
