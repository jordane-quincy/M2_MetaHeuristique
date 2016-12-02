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

typedef struct {
    int indiceObjToRemove;
    int indiceObjToAdd;
    int objValue;
} TabouMouvement;

typedef struct {
    int size;
    int sizeMax;
    TabouMouvement *list;
} ListTabou;

ObjRatio* alloc_tab(int nbrVar) {
    ObjRatio *tab;
    tab = malloc(sizeof (ObjRatio) * (nbrVar + 1));
    return tab;
}

ListTabou* init_tabou_list (int sizeTabouList) {
    ListTabou *listTabou;
    listTabou = malloc(sizeof (ListTabou));
    listTabou->list = calloc(sizeTabouList, sizeof(TabouMouvement));
    listTabou->size = 0;
    listTabou->sizeMax = sizeTabouList;
    return listTabou;
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
M�thode permettant de r�cup�rer un tableau ordonn�e des indices des objets du probl�me
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
M�thode permettant de r�cup�rer un tableau ordonn�e des indices des objets du probl�me
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
M�thode permettant de r�cup�rer un tableau ordonn�e des indices des objets du probl�me
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
M�thode permettant de r�cup�rer un tableau ordonn�e des indices des objets du probl�me
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
M�thode permettant de remplir la solution avec tous les objets du sac
**/
    int i;
void *remplirSac (tp_Mkp *mkp, Solution *s) {
    for (i = 1; i <= mkp->n; i++) {
        Add(mkp, s, i);
	}
	return 0;
}

/**
M�thode permettant d'obtenir une solution r�alisable � partir d'une solution non r�alisable o� tout les objets sont pris dans la solution
Renvoie 1 si on n'a pas trouv� de solution r�alisable, 0 sinon
**/
int obtenirSolutionRealisable (tp_Mkp *mkp, Solution *s) {
    int isNoSolution = 0;
    int i = 0;
    //On r�cup�re l'ordre par lequel on va retirer les objets
    int *ordre;
    ordre = malloc(sizeof (int) * (mkp->n + 1));
    //ordre = getTableauOrdonneByCoeff(mkp, ordre);
    //ordre = getTableauOrdonneByCoeffDemandeSurPoids(mkp, ordre);
    //ordre = getTableauOrdonneByCoeffPoidSurDemandePlusValeur(mkp, ordre);
    ordre = getTableauOrdonneByCoeff(mkp, ordre);
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
    free(ordre);
    return isNoSolution;
}

/**
Permet d'ajouter un mouvement � la liste tabou
Si la liste tabou est pleine, on supprime le premier mouvement de la liste, on d�calle tous les mouvements
Et on ajoute enfin le nouveau mouvement en fin de liste
(comportement d'une FIFO
**/
ListTabou *updateListTabou (ListTabou *listTabou, TabouMouvement* mouvement) {
    if (listTabou->size == listTabou->sizeMax) {
        //Si on est � la taille max, on d�calle la liste tabou et on ajoute le nouveau mouvement � la fin et on free le mouvement qu'on retire de la liste
        int i = 0;
        for (i = 1; i < listTabou->sizeMax; i++) {
            listTabou->list[i-1].indiceObjToAdd = listTabou->list[i].indiceObjToAdd;
            listTabou->list[i-1].indiceObjToRemove = listTabou->list[i].indiceObjToRemove;
            listTabou->list[i-1].objValue = listTabou->list[i].objValue;
        }
        listTabou->list[listTabou->size - 1].indiceObjToAdd = mouvement->indiceObjToAdd;
        listTabou->list[listTabou->size - 1].indiceObjToRemove = mouvement->indiceObjToRemove;
        listTabou->list[listTabou->size - 1].objValue = mouvement->objValue;
    }
    else {
        //Si la liste n'est pas pleine on ajoute le nouveau mouvement en fin de liste
        listTabou->list[listTabou->size].indiceObjToAdd = mouvement->indiceObjToAdd;
        listTabou->list[listTabou->size].indiceObjToRemove = mouvement->indiceObjToRemove;
        listTabou->list[listTabou->size].objValue = mouvement->objValue;
        listTabou->size++;
    }
    free(mouvement);
    return listTabou;
}

/**
Permet de savoir si le mouvement drop/add des indices en param�tre est pr�sent dans la liste tabou ou non
Ainsi que de savoir si l'objValue li� au mouvement n'a pas d�j� �t� trouv�
renvoie 1 si le mouvement n'est pas pr�sent
0 sinon
**/
int mouvementNotInTabouList (int indiceObjToDrop, int indiceObjToAdd, ListTabou *listTabou, int objValue) {
    int i = 0;
    for (i = 0; i < listTabou->size; i++) {
        //On ne fait pas le mouvement car le mouvement a d�j� �t� fait
        if (listTabou->list[i].indiceObjToAdd == indiceObjToAdd && listTabou->list[i].indiceObjToRemove == indiceObjToDrop) {
            return 0;
        }
        //On ne fait pas le mouvement car il am�ne � un objValue �gale � un mouvement tabou de la liste
        //C'est donc potentiellement une solution d�j� parcourue
        if (objValue == listTabou->list[i].objValue) {
            return 0;
        }
    }
    return 1;
}

void affichageListTabou (ListTabou listTabou) {
    int i;
    for (i = 0; i < listTabou.size; i++) {
        printf("objValueTabou : %d\n", listTabou.list[i].objValue);
    }
}

/**
On cherche une solution am�liorante
On doit � chaque �tape lib�rer la m�moire si cela est possible
Il nous faut donc un compteur car on ne veut pas lib�rer la solution de base, par contre une fois commenc� les appels r�cursif,
si on trouve une solution am�liorante on ne veut pas pour l'instant la conserver et donc on peut la lib�rer
**/
Solution *parcoursVoisin (tp_Mkp *mkp, Solution *sInitiale, int parcoursAllvoisin, Solution *bestS, ListTabou *listTabou, int cptForTabou, int cptTotal) {
    int i, j;
    Solution *copieS = copieSolution(mkp, sInitiale);
    SolutionAll *solutionall;
    int ameliorant = 0;
    //On initialise la solution la moins d�gradante pour l'algo tabou
    SolLessDegrading *solLessDegrading;
    //On set ce qu'on perdrait � l'infi pour que la premi�re solution non am�liorante soit prise en compte
    solLessDegrading = malloc(sizeof (SolLessDegrading));
    solLessDegrading->diffApport = INT_MAX;
    solLessDegrading->indiceObjToAdd = -1;
    solLessDegrading->indiceObjToRemove = -1;
    //On parcours une premi�re fois la solution
    if (parcoursAllvoisin) {
        //TODO parcours en passant par tous les voisins
        for (i = 1; i<= mkp->n; i++) {
            if (copieS->x[i] == 1 && isRemovePossible(mkp, copieS, i)) {//Si on a pris l'objet i dans la solution et si on peut enlever l'objet (on doit toujours respecter les cd)
                //On l'enl�ve
                Drop(mkp, copieS, i);
                //Puis on reparcours la liste des objets pour savoir quel objet remettre
                for (j = 1; j <= mkp->n; j++) {
                    //Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                    //et si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est sup�rieur � celui qu'on vient de retirer
                    //et qu'on peut l'ajouter en respectant les contraintes
                    if (j != i && copieS->x[j] == 0 && mkp->a[0][j] > mkp->a[0][i] && isAddPossible(mkp, copieS, j)) {
                        printf("DROP/ADD\n");
                        //Alors on ajoute dans le sac
                        Add(mkp, copieS, j);
                        //On regarde si cette nouvelle solution est am�liorante
                        //(normalement elle est forc�ment am�liorante puisqu'on ajoute uniquement les objets avec une valeur sup�rieur � l'objet qu'on a enlev�)
                        printf("OBJVALUE %d %d\n",copieS->objValue, sInitiale->objValue);
                        if (copieS->objValue > sInitiale->objValue) {

                            printf("solution 0: %d %d\n", i, j);
                            solutionall = malloc(sizeof (SolutionAll));
                            solutionall->index_deleted_obj = i;
                            solutionall->index_added_obj = j;
                            solutionall->difference = copieS->objValue - sInitiale->objValue;

                            printf("solution 1: %d %d %d\n", solutionall->index_deleted_obj, solutionall->index_added_obj, solutionall->difference);
                            ameliorant = 1;
                        }
                        //else {
                            Drop(mkp, copieS, j);
                        //}
                    }
                }
                //Si on n'a pas trouv� de solution am�liorante, on rajoute l'objet qu'on a enlev� au d�part et on passe � l'objet suivant
                Add(mkp, copieS, i);
            }
        }
        free_sol(sInitiale);
        sInitiale = NULL;

        if(!ameliorant) return copieS;

        Drop(mkp, copieS, solutionall->index_deleted_obj);
        Add(mkp, copieS, solutionall->index_added_obj);
        printf("**************\n\n");
        return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou, cptTotal);
    }
    else {
        for (i = 1; i<= mkp->n; i++) {
            if (copieS->x[i] == 1 && isRemovePossible(mkp, copieS, i)) {//Si on a pris l'objet i dans la solution et si on peut enlever l'objet (on doit toujours respecter les cd)
                //On l'enl�ve
                Drop(mkp, copieS, i);
                //printf("juste apr�s drop\n");
                //Puis on reparcours la liste des objets pour savoir quel objet remettre
                for (j = 1; j <= mkp->n; j++) {
                    //Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                    //et qu'on peut l'ajouter en respectant les contraintes
                    //et que le mouvement drop i / add j n'est pas dans la liste tabou
                    if (j != i && copieS->x[j] == 0 && isAddPossible(mkp, copieS, j) && mouvementNotInTabouList(i, j, listTabou, copieS->objValue + mkp->a[0][j])) {
                        //si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est sup�rieur � celui qu'on vient de retirer
                        if (mkp->a[0][j] > mkp->a[0][i]) {
                            //printf("DROP/ADD de %d/%d\n", i, j);
                            //Alors on ajoute dans le sac
                            Add(mkp, copieS, j);
                            //On regarde si cette nouvelle solution est am�liorante
                            //(normalement elle est forc�ment am�liorante puisqu'on ajoute uniquement les objets avec une valeur sup�rieur � l'objet qu'on a enlev�)
                            if (copieS->objValue > sInitiale->objValue) {
                                //Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution am�liorante.
                                //Si la solution am�liorante qu'on vient de trouver est meilleure que la meilleure solution qu'on stoque avec l'algo tabou alors on remplace notre bestS par la solution am�liorante
                                //Qui sera alors notre nouvelle meilleure solution
                                if (copieS->objValue > bestS->objValue) {
                                    if (bestS->objValue != sInitiale->objValue) {
                                        free_sol(bestS);
                                    }

                                    bestS = copieS;
                                    //reset du compteur pour la recherche tabou pour indiquer qu'il faut continuer de chercher
                                    cptForTabou = 0;
                                }
                                //On lib�re sInitiale
                                //printf("free sol s\n");
                                free_sol(sInitiale);
                                sInitiale = NULL;
                                return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou, cptTotal);
                            }
                            else {
                                Drop(mkp, copieS, j);
                            }
                        }
                        else {
                            /**Si en faisant cette �change la solution n'est pas am�liorante
                            On va stocker la moins pire des solutions non am�liorantes
                            Pour pouvoir partir de cette solution avec l'algorithme tabou
                            (dans le cas o� on ne trouve plus de solution am�liorante)**/

                            //On calcul ce qu'on perdrait
                            int diffApport = mkp->a[0][i] - mkp->a[0][j];
                            //On garde uniquement lorsque ce qu'on perdrait est plus petit que ce qu'on a sauvegard� durant les it�rations pr�c�dents
                            if (solLessDegrading->diffApport > diffApport) {
                                //Si cette solution d�grade moins que la pr�c�dente alors on garde cette solution
                                solLessDegrading->diffApport = diffApport;
                                solLessDegrading->indiceObjToAdd = j;
                                solLessDegrading->indiceObjToRemove = i;
                            }
                        }
                    }
                }
                //Si on n'a pas trouv� de solution am�liorante, on rajoute l'objet qu'on a enlev� au d�part et on passe � l'objet suivant
                Add(mkp, copieS, i);
            }
        }
        /**A cette �tape on n'a pas trouv� de solution am�liorante, on va donc continuer avec la solution la moins d�gradante toute en interdisant par la suite de revenir � cette solution (algo tabou)
        Il faut �galement stocker la solution en cours puisque c'est la meilleure solution du voisinage
        (on la compara � la fin de l'algo avec nos autres "meilleures solutions" pour voir quelle est la meilleure des meilleures solutions)
        **/
        //printf("On devrait faire l'algorithme tabou ici\n");
        //printf("L'objet a enlever serait l'objet %d\n", solLessDegrading.indiceObjToRemove);
        //printf("L'objet a ajouter serait l'objet %d\n", solLessDegrading.indiceObjToAdd);
        //printf("On perdrait : %d\n", solLessDegrading.diffApport);
        //Si notre meilleur solution (des pr�c�dents parcours de voisin) est moins bonne que celle-ci on la conserve pour le r�sultat final
        if (bestS->objValue < sInitiale->objValue) {
            //on lib�re la m�moire de la solution bestS pour mettre bestS � s (uniquement si bestS est diff�rent de la sol initial car on veut garder notre solution initiale
            //free_sol(bestS);
                free_sol(bestS);
            bestS = sInitiale;
            //on reset �galement le timer de la recherche tabou pour continuer de rechercher
            cptForTabou = 0;

        }
        else {
            //on lib�re la m�moire de s uniquement si s et bestS sont diff�rent (en se basant sur l'objValue
            if (bestS->objValue != sInitiale->objValue) {
                free_sol(sInitiale);
                sInitiale = NULL;
            }

        }

        //On ajoute le mouvement � la lite des mouvements tabou
        //Le mouvement tabou est l'inverse du mouvement qu'on va faire pour d�grader la solution ainsi que l'obj value de la solution en cours (du minimum local)
        //Si pour d�grad� la solution en fait drop i / add j alors le mouvement tabou est add i / drop j
        TabouMouvement *tabouMouvement;
        tabouMouvement = malloc(sizeof (TabouMouvement));
        tabouMouvement->indiceObjToAdd = solLessDegrading->indiceObjToRemove;
        tabouMouvement->indiceObjToRemove = solLessDegrading->indiceObjToAdd;
        tabouMouvement->objValue = copieS->objValue;
        listTabou = updateListTabou(listTabou, tabouMouvement);
        //on fait le mouvement d�gradant sur copieS pour parcourir ensuite copieS
        Drop(mkp, copieS, solLessDegrading->indiceObjToRemove);
        Add(mkp, copieS, solLessDegrading->indiceObjToAdd);
        //Puis on parcours les voisins de la solution la moins d�gradante

        if (cptForTabou < 15000) {
            //printf("On applique l'algo tabou en parcourant les voisins d'une solution degradante\n");
            //printf("copieS : %d\n", copieS->objValue);
            //printf("bestS : %d\n", bestS->objValue);
            //printf("cpt total : %d\n", cptTotal);
            return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou + 1, cptTotal + 1);
        }
        return bestS;
    }
    //return de la solution
    return sInitiale;
}



int main(int argc, char *argv[]) {
    tp_Mkp *mkp;
    Solution *sol = NULL, *sAmeliorante = NULL, *sInitiale = NULL;
	if(argc != 4) {
		printf("Usage: programme nomFichierEntree nomFichierSortie\n");
		exit(0);
    }
    mkp = tp_load_mkp(argv[1]);
    int sizeListTabou = atoi(argv[3]);
    //Init liste tabou
    ListTabou *listTabou = init_tabou_list(sizeListTabou);

    sol = alloc_sol(mkp);
	init_sol(sol, mkp);
	printf("Probleme sac a dos : nbr objets : %d, nbr cc : %d, nbr cd : %d\n", mkp->n, mkp->cc, mkp->cd);
	//On a initialis� la solution comme �tant un sac vide, on va ajouter tous les objets pour avoir une solution non r�alisable qui poss�de tous les objets
	//Puis on va retirer les objets 1 � 1 pour arriver � une solution r�alisable
	remplirSac(mkp, sol);

	printf("Object value : %d\n", sol->objValue);
	printf("slack cc : %d\n", sol->slack[0][0]);
	printf("slack cd : %d\n", sol->slack[1][0]);

	//Il faut maintenant retirer les objets jusqu'� ce que la solution soit r�alisable
	int isNoSolution = obtenirSolutionRealisable(mkp, sol);

    printf("Pas de solution ? %d\n", isNoSolution);
    printf("Object value : %d\n", sol->objValue);
	printf("slack cc : %d\n", sol->slack[0][0]);
	printf("slack cd : %d\n", sol->slack[1][0]);


    /*Maintenant on recherche une solution am�liorante*/

    //copie de la sol initiale parce que s va surement �tre d�sallou� par parcoursVoisin
    sInitiale = copieSolution(mkp, sol);
    printf("Calcul en cours, patientez...\n");
    //sAmeliorante = NULL;
    sAmeliorante = parcoursVoisin(mkp, sol, 0, sol, listTabou, 0, 0);
    //Affichage du r�sultat de la recherche de solution am�liorante
    printf("Ancienne value du sac : %d\n", sInitiale->objValue);

    if (sAmeliorante != NULL && sInitiale->objValue != 0) {
        printf("Nouvelle value du sac : %d\n", sAmeliorante->objValue);
        output_best_solution(sAmeliorante,argv[1],mkp->n,argv[2]);
        if(sInitiale->objValue == sAmeliorante->objValue)
            printf("Solution non ameliorante...\n");
    }
    else {
        if(!sInitiale->objValue) {
            printf("Pas de solution...\n");
            if(is_add_P(mkp)) record(argv[1], "w", "Pas de solution � ce probl�me...\n", argv[2]);
            else record(argv[1], "a", "Pas de solution � ce probl�me...\n", argv[2]);
        }
        else {
            output_best_solution(sInitiale,argv[1],mkp->n,argv[2]);
            printf("Solution non ameliorante...\n");
        }
    }

    //Lib�valuen de la m�moire
    //printf("test :%d", s->objValue);
    free(listTabou->list);
    listTabou->list = NULL;
    free(listTabou);
    free(sol);
    free_sol(sInitiale);
    //free_sol(sAmeliorante);
    tp_del_mkp(mkp);
	return 0;
}
