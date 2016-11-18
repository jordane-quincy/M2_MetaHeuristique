#include "tp_mkpkit.h"
#include "tp_mkpsol.h"
#include "tp.h"

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"

typedef struct {
     int indexObj;
     double value;
} ObjRatio;

typedef struct {
    int indiceObjToRemove;
    int indiceObjToAdd;
    int diffApport;
} SolLessDegrading;

ObjRatio* alloc_tab(int nbrVar) {
    ObjRatio *tab;
    tab = (ObjRatio *)malloc(sizeof (ObjRatio) * (nbrVar + 1));
    return tab;
}

/**
Trie du plus grand au plus petit
**/
int sortRatioDesc (const void * a, const void * b)
{
    if (((ObjRatio*)a)->value < ((ObjRatio*)b)->value) {
        return 1;
    }
    else if (((ObjRatio*)a)->value > ((ObjRatio*)b)->value) {
        return -1;
    }
    return 0;
}

/**
Trie du plus petit au plus grand
**/
int sortRatioAsc (const void * a, const void * b)
{
    if (((ObjRatio*)a)->value > ((ObjRatio*)b)->value) {
        return 1;
    }
    else if (((ObjRatio*)a)->value < ((ObjRatio*)b)->value) {
        return -1;
    }
    return 0;
}

/**
Trie du plus petit au plus grand
**/
int sortDemandAsc (const void * a, const void * b) {
    if (((ObjRatio*)a)->value < ((ObjRatio*)b)->value) {
        return 1;
    }
    else if (((ObjRatio*)a)->value > ((ObjRatio*)b)->value) {
        return -1;
    }
    return 0;
}

/**
Méthode permettant de récupérer un tableau ordonnée des indices des objets du problème
Dans ce cas le trie des objets se ferra en fonction du coeff valeur/poid
En mettant en premier l'objet avec le plus petit value
**/
int* getTableauOrdonneByCoeff (tp_Mkp *mkp, int* ordre) {
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
        tabOrdonne[i].value = coefObj/poidsObj;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortRatioAsc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

/**
Méthode permettant de récupérer un tableau ordonnée des indices des objets du problème
Dans ce cas le trie des objets se ferra en fonction du coeff poid/(valuee+valeur)
En mettant en premier l'objet avec le grand petit value
**/
int* getTableauOrdonneByCoeffPoidSurDemandePlusValeur (tp_Mkp *mkp, int* ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);
    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        double coefObj = mkp->a[0][i];
        double poidsObj = 0;
        double valueObj = 0;
        for (j = 1; j <= mkp->cc; j++) {
            poidsObj += mkp->a[j][i];
        }
        for (j = 1; j <= mkp->cd; j++) {
            valueObj += mkp->a[mkp->cc + j][i];
        }
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = poidsObj/(coefObj + valueObj);
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortRatioDesc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

/**
Méthode permettant de récupérer un tableau ordonnée des indices des objets du problème
Dans ce cas le trie des objets se ferra en fonction de la valuee de l'objet
En mettant en premier l'objet qui rapport le moins en valuee
**/
int* getTableauOrdonneByLessDemand (tp_Mkp *mkp, int* ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);

    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        double valueObj = 0;
        for (j = 1; j <= mkp->cd; j++) {
            valueObj += mkp->a[mkp->cc + j][i];
        }
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = valueObj;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortDemandAsc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

/**
Méthode permettant de récupérer un tableau ordonnée des indices des objets du problème
Dans ce cas le trie des objets se ferra en fonction du valuen valuee/poids de l'objet
En mettant en premier l'objet avec le value le plus petit
**/
int* getTableauOrdonneByCoeffDemandeSurPoids(tp_Mkp *mkp, int *ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);

    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        double poidsObj = 0;
        double valueObj = 0;
        for (j = 1; j <= mkp->cc; j++) {
            poidsObj += mkp->a[j][i];
        }
        for (j = 1; j <= mkp->cd; j++) {
            valueObj += mkp->a[mkp->cc + j][i];
        }
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = valueObj/poidsObj;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortRatioAsc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

/**
Méthode permettant de remplir la solution avec tous les objets du sac
**/
    int i;
void *remplirSac (tp_Mkp *mkp, Solution *s) {
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
    //ordre = getTableauOrdonneByCoeff(mkp, ordre);
    //ordre = getTableauOrdonneByCoeffDemandeSurPoids(mkp, ordre);
    //ordre = getTableauOrdonneByCoeffPoidSurDemandePlusValeur(mkp, ordre);
    ordre = getTableauOrdonneByLessDemand(mkp, ordre);
    printf("nbr d'objet : %d\n", mkp->n);
    //Check si on peut retirer l'objet, si oui on le retire
    printf("Nbr de cc non valid : %d\n", s->slack[0][0]);
    while (s->slack[0][0] > 0 && isNoSolution == 0) {
        i++;
        if (i > mkp->n) {
            isNoSolution = 1;
        }
        else if (isRemovePossible(mkp, s, ordre[i])) {
            Drop(mkp, s, ordre[i]);
        }
    }
    return isNoSolution;
}

/**
On cherche une solution améliorante
On doit à chaque étape libérer la mémoire si cela est possible
Il nous faut donc un compteur car on ne veut pas libérer la solution de base, par contre une fois commencé les appels récursif,
si on trouve une solution améliorante on ne veut pas pour l'instant la conserver et donc on peut la libérer
**/
Solution *parcoursVoisin (tp_Mkp *mkp, Solution *s, int compteur, int parcoursAllvoisin) {
    int i, j;
    Solution *copieS = copieSolution(mkp, s);

    //On initialise la solution la moins dégradante pour l'algo tabou
    SolLessDegrading solLessDegrading;
    //On set ce qu'on perdrait à l'infi pour que la première solution non améliorante soit prise en compte
    solLessDegrading.diffApport = INT_MAX;
    //On parcours une première fois la solution
    if (parcoursAllvoisin) {
        //TODO parcours en passant par tous les voisins
    }
    else {
        for (i = 1; i<= mkp->n; i++) {
            if (copieS->x[i] == 1 && isRemovePossible(mkp, copieS, i)) {//Si on a pris l'objet i dans la solution et si on peut enlever l'objet (on doit toujours respecter les cd)
                //On l'enlève
                Drop(mkp, copieS, i);
                //Puis on reparcours la liste des objets pour savoir quel objet remettre
                for (j = 1; j <= mkp->n; j++) {
                    //Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                    //et qu'on peut l'ajouter en respectant les contraintes
                    if (j != i && copieS->x[j] == 0 && isAddPossible(mkp, copieS, j)) {
                        //si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est supérieur à celui qu'on vient de retirer
                        if (mkp->a[0][j] > mkp->a[0][i]) {
                            printf("DROP/ADD\n");
                            //Alors on ajoute dans le sac
                            Add(mkp, copieS, j);
                            //On regarde si cette nouvelle solution est améliorante
                            //(normalement elle est forcément améliorante puisqu'on ajoute uniquement les objets avec une valeur supérieur à l'objet qu'on a enlevé)
                            if (copieS->objValue > s->objValue) {
                                //Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution améliorante.
                                //Si on n'est pas sur le premier appel de la fonction, alors on peut libérer s
                                //Sinon on ne libère rien car cela signifie que s est notre solution initiale
                                if (compteur != 0) {
                                    free_sol(s);
                                    s = NULL;
                                }
                                return parcoursVoisin(mkp, copieS, compteur + 1, parcoursAllvoisin);
                            }
                            else {
                                Drop(mkp, copieS, j);
                            }
                        }
                        else {
                            /**Si en faisant cette échange la solution n'est pas améliorante
                            On va stocker la moins pire des solutions non améliorantes
                            Pour pouvoir partir de cette solution avec l'algorithme tabou
                            (dans le cas où on ne trouve plus de solution améliorante)**/

                            //On calcul ce qu'on perdrait
                            int diffApport = mkp->a[0][i] - mkp->a[0][j];
                            //On garde uniquement lorsque ce qu'on perdrait est plus petit que ce qu'on a sauvegardé durant les itérations précédents
                            if (solLessDegrading.diffApport > diffApport) {
                                //Si cette solution dégrade moins que la précédente alors on garde cette solution
                                solLessDegrading.diffApport = diffApport;
                                solLessDegrading.indiceObjToAdd = j;
                                solLessDegrading.indiceObjToRemove = i;
                            }
                        }
                    }
                }
                //Si on n'a pas trouvé de solution améliorante, on rajoute l'objet qu'on a enlevé au départ et on passe à l'objet suivant
                Add(mkp, copieS, i);
            }
        }
        //A cette étape on n'a pas trouvé de solution améliorante, on va donc continuer avec la solution la moins dégradante toute en interdisant par la suite de revenir à cette solution (algo tabou)
        printf("On devrait faire l'algorithme tabou ici\n");
        printf("L'objet a enlever serait l'objet %d\n", solLessDegrading.indiceObjToRemove);
        printf("L'objet a ajouter serait l'objet %d\n", solLessDegrading.indiceObjToAdd);
        printf("On perdrait : %d\n", solLessDegrading.diffApport);

    }
    //return de la solution
    return s;
}



int main(int argc, char *argv[]) {
    tp_Mkp *mkp;
    Solution *s, *sAmeliorante = NULL;
	if(argc != 3) {
		printf("Usage: programme nomFichierEntree nomFichierSortie\n");
		exit(0);
    }
    mkp = tp_load_mkp(argv[1]);
    s = alloc_sol(mkp);
	init_sol(s, mkp);
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


    /*Maintenant on recherche une solution améliorante*/


    sAmeliorante = parcoursVoisin(mkp, s, 0, 0);

    //Affichage du résultat de la recherche de solution améliorante
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
            if(is_add_P(mkp)) record(argv[1], "w", "Pas de solution à ce problème...\n", argv[2]);
            else record(argv[1], "a", "Pas de solution à ce problème...\n", argv[2]);
        }
        else {
            output_best_solution(s,argv[1],mkp->n,argv[2]);
            printf("Solution non ameliorante...\n");
        }
    }

    //Libévaluen de la mémoire
    free_sol(s);
    free_sol(sAmeliorante);
    tp_del_mkp(mkp);
	return 0;
}
