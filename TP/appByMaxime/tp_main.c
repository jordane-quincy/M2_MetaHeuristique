#include "tp_mkpkit.h"
#include "tp_mkpsol.h"
#include "tp.h"

#include "stdio.h"
#include "stdlib.h"
#include "limits.h"
#include "time.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

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

int timeout(int startTime, int tempsMax, Solution *sAmeliorante, Solution *currentSolution, char *instance, char *outputFileName, tp_Mkp *mkp){
    int currentTime = (int)time(NULL);
    if( (currentTime - startTime) >= tempsMax ){
        printf("\n\n**********************************************\nTemps ecoule !\n");
        /*On �crit la meilleure solution qu'on ait*/
        if (sAmeliorante->objValue >= currentSolution->objValue) {
            printf("La meilleure solution trouvee a pour resultat : %d\n", sAmeliorante->objValue);
            output_best_solution(sAmeliorante,instance,mkp->n,outputFileName);
        }
        else {
            printf("La meilleure solution trouvee a pour resultat : %d\n", sAmeliorante->objValue);
            output_best_solution(currentSolution,instance,mkp->n,outputFileName);
        }
        free_sol(sAmeliorante);
        free_sol(currentSolution);
        printf("Bye Bye\n");
        /* Our code run out of time friends :-( */
        exit(999);
    }
    return 1;
}

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

int* getTableauOrdonneByIndex(tp_Mkp *mkp, int *ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);
    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = i;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortRatioAsc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

int* getTableauOrdonneByReverseIndex(tp_Mkp *mkp, int *ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);
    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = i;
    }
    qsort((void *)(tabOrdonne + 1), (size_t) mkp->n, (size_t)sizeof(ObjRatio), sortRatioDesc);
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    free(tabOrdonne);
    return ordre;
}

int* getTableauOrdonneRandom(tp_Mkp *mkp, int *ordre) {
    ObjRatio *tabOrdonne;
    tabOrdonne = alloc_tab(mkp->n);
    int i;
    int j;
    for (i = 1; i <= mkp->n; i++) {
        tabOrdonne[i].indexObj = i;
        tabOrdonne[i].value = rand();
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
int obtenirSolutionRealisable (tp_Mkp *mkp, Solution *s, int algoToUse) {
    int isNoSolution = 0;
    int i = 0;

    /*On r�cup�re l'ordre par lequel on va retirer les objets*/
    int *ordre;
    ordre = malloc(sizeof (int) * (mkp->n + 1));
    if (algoToUse == 0)
        ordre = getTableauOrdonneByCoeff(mkp, ordre);
    else if (algoToUse == 1)
        ordre = getTableauOrdonneByCoeffDemandeSurPoids(mkp, ordre);
    else if (algoToUse == 2)
        ordre = getTableauOrdonneByCoeffPoidSurDemandePlusValeur(mkp, ordre);
    else if (algoToUse == 3)
        ordre = getTableauOrdonneByLessDemand(mkp, ordre);
    else if (algoToUse == 4)
        ordre = getTableauOrdonneByIndex(mkp, ordre);
    else if (algoToUse == 5)
        ordre = getTableauOrdonneByReverseIndex(mkp, ordre);
    else
        ordre = getTableauOrdonneRandom(mkp, ordre);

    /*Check si on peut retirer l'objet, si oui on le retire*/
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
        /*Si on est � la taille max, on d�calle la liste tabou et on ajoute le nouveau mouvement � la fin et on free le mouvement qu'on retire de la liste*/
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
        /*Si la liste n'est pas pleine on ajoute le nouveau mouvement en fin de liste*/
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
        /*On ne fait pas le mouvement car le mouvement a d�j� �t� fait*/
        if (listTabou->list[i].indiceObjToAdd == indiceObjToAdd && listTabou->list[i].indiceObjToRemove == indiceObjToDrop) {
            return 0;
        }
        /*On ne fait pas le mouvement car il am�ne � un objValue �gale � un mouvement tabou de la liste
        C'est donc potentiellement une solution d�j� parcourue*/
        if (objValue == listTabou->list[i].objValue) {
            return 0;
        }
    }
    return 1;
}

void affichageListTabou (ListTabou *listTabou) {
    int i;
    for (i = 0; i < listTabou->size; i++) {
        printf("objValueTabou : %d\n", listTabou->list[i].objValue);
    }
}

/**
On cherche une solution am�liorante
On doit � chaque �tape lib�rer la m�moire si cela est possible
Il nous faut donc un compteur car on ne veut pas lib�rer la solution de base, par contre une fois commenc� les appels r�cursif,
si on trouve une solution am�liorante on ne veut pas pour l'instant la conserver et donc on peut la lib�rer
**/
Solution *parcoursVoisin (tp_Mkp *mkp, Solution *sInitiale, int parcoursAllvoisin, Solution *bestS, ListTabou *listTabou, int cptForTabou, int cptTotal, int startTime, int tempsMax, char *instance, char *outputFileName, Solution *globalBestS) {
    timeout(startTime, tempsMax, globalBestS, bestS, instance, outputFileName, mkp);
    int i, j;
    Solution *copieS = copieSolution(mkp, sInitiale);
    SolutionAll *solutionall;
    solutionall = malloc(sizeof (SolutionAll));
    solutionall->difference = -INT_MAX;
    solutionall->index_added_obj = -1;
    solutionall->index_deleted_obj = -1;
    int ameliorant = 0;
    /*On initialise la solution la moins d�gradante pour l'algo tabou*/
    SolLessDegrading *solLessDegrading;
    /*On set ce qu'on perdrait � l'infi pour que la premi�re solution non am�liorante soit prise en compte*/
    solLessDegrading = malloc(sizeof (SolLessDegrading));
    solLessDegrading->diffApport = INT_MAX;
    solLessDegrading->indiceObjToAdd = -1;
    solLessDegrading->indiceObjToRemove = -1;
    /*On parcours une premi�re fois la solution*/
    if (parcoursAllvoisin) {
        for (i = 1; i<= mkp->n; i++) {
            /*timeout(startTime, tempsMax);*/
            if (copieS->x[i] == 1 && isRemovePossible(mkp, copieS, i)) {/*Si on a pris l'objet i dans la solution et si on peut enlever l'objet (on doit toujours respecter les cd)*/
                /*On l'enl�ve*/
                Drop(mkp, copieS, i);
                /*Puis on reparcours la liste des objets pour savoir quel objet remettre*/
                for (j = 1; j <= mkp->n; j++) {
                    /*timeout(startTime, tempsMax);*/
                    /*Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                    et si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est sup�rieur � celui qu'on vient de retirer
                    et qu'on peut l'ajouter en respectant les contraintes*/
                    if (j != i && copieS->x[j] == 0 && isAddPossible(mkp, copieS, j) && mouvementNotInTabouList(i, j, listTabou, copieS->objValue + mkp->a[0][j])) {
                        if (mkp->a[0][j] > mkp->a[0][i]) {
                            /*Alors on ajoute dans le sac*/
                            Add(mkp, copieS, j);
                            /*On regarde si cette nouvelle solution est am�liorante
                            (normalement elle est forc�ment am�liorante puisqu'on ajoute uniquement les objets avec une valeur sup�rieur � l'objet qu'on a enlev�)*/
                            if (copieS->objValue - sInitiale->objValue > solutionall->difference) {
                                /*Si ce qu'on gagne avec copieS est sup�rieur � ce qu'on gagnait avec notre meilleure solution am�liorante
                                alors notre meilleure solution am�liorante doit valoir copieS*/
                                solutionall->index_deleted_obj = i;
                                solutionall->index_added_obj = j;
                                solutionall->difference = copieS->objValue - sInitiale->objValue;
                                ameliorant = 1;
                            }
                            Drop(mkp, copieS, j);
                        }
                        else {
                            /**Si en faisant cette �change la solution n'est pas am�liorante
                            On va stocker la moins pire des solutions non am�liorantes
                            Pour pouvoir partir de cette solution avec l'algorithme tabou
                            (dans le cas o� on ne trouve plus de solution am�liorante)**/

                            /*On calcul ce qu'on perdrait*/
                            int diffApport = mkp->a[0][i] - mkp->a[0][j];
                            /*On garde uniquement lorsque ce qu'on perdrait est plus petit que ce qu'on a sauvegard� durant les it�rations pr�c�dents*/
                            if (solLessDegrading->diffApport > diffApport) {
                                /*Si cette solution d�grade moins que la pr�c�dente alors on garde cette solution*/
                                solLessDegrading->diffApport = diffApport;
                                solLessDegrading->indiceObjToAdd = j;
                                solLessDegrading->indiceObjToRemove = i;
                            }
                        }
                    }
                }
                /*Si on n'a pas trouv� de solution am�liorante, on rajoute l'objet qu'on a enlev� au d�part et on passe �l'objet suivant*/
                Add(mkp, copieS, i);
            }
        }
        if(!ameliorant) {
            /**A cette �tape on n'a pas trouv� de solution am�liorante, on va donc continuer avec la solution la moins d�gradante toute en interdisant par la suite de revenir � cette solution (algo tabou)
            Il faut �galement stocker la solution en cours puisque c'est la meilleure solution du voisinage
            (on la compara � la fin de l'algo avec nos autres "meilleures solutions" pour voir quelle est la meilleure des meilleures solutions)
            **/
            if (bestS != sInitiale) {
                free_sol(sInitiale);
                sInitiale = NULL;
            }

            /*On ajoute le mouvement � la lite des mouvements tabou
            Le mouvement tabou est l'inverse du mouvement qu'on va faire pour d�grader la solution ainsi que l'obj value de la solution en cours (du minimum local)
            Si pour d�grader la solution en fait drop i / add j alors le mouvement tabou est add i / drop j*/
            TabouMouvement *tabouMouvement;
            tabouMouvement = malloc(sizeof (TabouMouvement));
            tabouMouvement->indiceObjToAdd = solLessDegrading->indiceObjToRemove;
            tabouMouvement->indiceObjToRemove = solLessDegrading->indiceObjToAdd;
            tabouMouvement->objValue = copieS->objValue;
            listTabou = updateListTabou(listTabou, tabouMouvement);
            /*on fait le mouvement d�gradant sur copieS pour parcourir ensuite copieS*/
            Drop(mkp, copieS, solLessDegrading->indiceObjToRemove);
            Add(mkp, copieS, solLessDegrading->indiceObjToAdd);
            /*Puis on parcours les voisins de la solution la moins d�gradante*/

            free(solLessDegrading);
            free(solutionall);
            if (cptForTabou < 3000) {
                return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou + 1, cptTotal + 1, startTime, tempsMax, instance, outputFileName, globalBestS);
            }
            if (bestS != copieS) {
                free(copieS);
            }
            return bestS;
        }
        else {
            /*Si on a trouv� une solution am�liorante on la prend et on reparcours les voisins*/
            Drop(mkp, copieS, solutionall->index_deleted_obj);
            Add(mkp, copieS, solutionall->index_added_obj);

            /*Si copieS est meilleure que bestS alors on passe bestS � copieS*/
            if (copieS->objValue > bestS->objValue) {
                if (bestS != sInitiale) {
                    free_sol(bestS);
                }
                bestS = copieS;
                /*reset du compteur pour la recherche tabou pour indiquer qu'il faut continuer de chercher*/
                cptForTabou = 0;
            }
            free_sol(sInitiale);
            sInitiale = NULL;
            free(solLessDegrading);
            free(solutionall);
            return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou, cptTotal, startTime, tempsMax, instance, outputFileName, globalBestS);
        }
    }
    else {
        for (i = 1; i<= mkp->n; i++) {
            /*timeout(startTime, tempsMax);*/
            if (copieS->x[i] == 1 && isRemovePossible(mkp, copieS, i)) {/*Si on a pris l'objet i dans la solution et si on peut enlever l'objet (on doit toujours respecter les cd)*/
                /*On l'enl�ve*/
                Drop(mkp, copieS, i);
                /*Puis on reparcours la liste des objets pour savoir quel objet remettre*/
                for (j = 1; j <= mkp->n; j++) {
                    /*timeout(startTime, tempsMax);*/
                    /*Si l'objet n'est pas celui qu'on vient de retirer et si l'objet j n'est pas dans le sac,
                    et qu'on peut l'ajouter en respectant les contraintes
                    et que le mouvement drop i / add j n'est pas dans la liste tabou*/
                    if (j != i && copieS->x[j] == 0 && isAddPossible(mkp, copieS, j) && mouvementNotInTabouList(i, j, listTabou, copieS->objValue + mkp->a[0][j])) {
                        /*si la valeur (dans la fonction objectif) de l'objet qu'on veut tenter d'ajouter est sup�rieur � celui qu'on vient de retirer*/
                        if (mkp->a[0][j] > mkp->a[0][i]) {
                            /*Alors on ajoute dans le sac*/
                            Add(mkp, copieS, j);
                            /*On regarde si cette nouvelle solution est am�liorante
                            (normalement elle est forc�ment am�liorante puisqu'on ajoute uniquement les objets avec une valeur sup�rieur � l'objet qu'on a enlev�)*/
                            if (copieS->objValue > sInitiale->objValue) {
                                /*Si oui, on parcours les voisins de la nouvelle solution afin de retrouver une potentielle autre solution am�liorante.
                                Si la solution am�liorante qu'on vient de trouver est meilleure que la meilleure solution qu'on stoque avec l'algo tabou alors on remplace notre bestS par la solution am�liorante
                                Qui sera alors notre nouvelle meilleure solution*/
                                if (copieS->objValue > bestS->objValue) {
                                    if (bestS != sInitiale) {
                                        free_sol(bestS);
                                    }

                                    bestS = copieS;
                                    /*reset du compteur pour la recherche tabou pour indiquer qu'il faut continuer de chercher*/
                                    cptForTabou = 0;
                                }
                                /*On lib�re sInitiale*/
                                free_sol(sInitiale);
                                sInitiale = NULL;
                                free(solLessDegrading);
                                free(solutionall);
                                return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou, cptTotal, startTime, tempsMax, instance, outputFileName, globalBestS);
                            }
                        }
                        else {
                            /*Si en faisant cette �change la solution n'est pas am�liorante
                            On va stocker la moins pire des solutions non am�liorantes
                            Pour pouvoir partir de cette solution avec l'algorithme tabou
                            (dans le cas o� on ne trouve plus de solution am�liorante)*/

                            /*On calcul ce qu'on perdrait*/
                            int diffApport = mkp->a[0][i] - mkp->a[0][j];
                            /*On garde uniquement lorsque ce qu'on perdrait est plus petit que ce qu'on a sauvegard� durant les it�rations pr�c�dents*/
                            if (solLessDegrading->diffApport > diffApport) {
                                /*Si cette solution d�grade moins que la pr�c�dente alors on garde cette solution*/
                                solLessDegrading->diffApport = diffApport;
                                solLessDegrading->indiceObjToAdd = j;
                                solLessDegrading->indiceObjToRemove = i;
                            }
                        }
                    }
                }
                /*Si on n'a pas trouv� de solution am�liorante, on rajoute l'objet qu'on a enlev� au d�part et on passe � l'objet suivant*/
                Add(mkp, copieS, i);
            }
        }
        /*A cette �tape on n'a pas trouv� de solution am�liorante, on va donc continuer avec la solution la moins d�gradante toute en interdisant par la suite de revenir � cette solution (algo tabou)
        Il faut �galement stocker la solution en cours puisque c'est la meilleure solution du voisinage
        (on la compara � la fin de l'algo avec nos autres "meilleures solutions" pour voir quelle est la meilleure des meilleures solutions)*/

        /*on lib�re la m�moire de s uniquement si s et bestS sont diff�rent (en se basant sur l'objValue)*/
        if (bestS != sInitiale) {
            free_sol(sInitiale);
            sInitiale = NULL;
        }

        /*On ajoute le mouvement � la lite des mouvements tabou
        Le mouvement tabou est l'inverse du mouvement qu'on va faire pour d�grader la solution ainsi que l'obj value de la solution en cours (du minimum local)
        Si pour d�grad� la solution en fait drop i / add j alors le mouvement tabou est add i / drop j*/
        TabouMouvement *tabouMouvement;
        tabouMouvement = malloc(sizeof (TabouMouvement));
        tabouMouvement->indiceObjToAdd = solLessDegrading->indiceObjToRemove;
        tabouMouvement->indiceObjToRemove = solLessDegrading->indiceObjToAdd;
        tabouMouvement->objValue = copieS->objValue;
        listTabou = updateListTabou(listTabou, tabouMouvement);
        /*on fait le mouvement d�gradant sur copieS pour parcourir ensuite copieS*/
        Drop(mkp, copieS, solLessDegrading->indiceObjToRemove);
        Add(mkp, copieS, solLessDegrading->indiceObjToAdd);
        /*Puis on parcours les voisins de la solution la moins d�gradante*/
        free(solLessDegrading);
        free(solutionall);
        if (cptForTabou < 3000) {
            return parcoursVoisin(mkp, copieS, parcoursAllvoisin, bestS, listTabou, cptForTabou + 1, cptTotal + 1, startTime, tempsMax, instance, outputFileName, globalBestS);
        }
        if (bestS != copieS) {
            free(copieS);
        }
        return bestS;
    }
    /*return de la solution*/
    return sInitiale;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int startTime = (int)time(NULL);
    tp_Mkp *mkp = NULL;
    Solution *sol = NULL, *sAmeliorante = NULL, *bestS = NULL;
	if(argc != 4) {
		printf("Usage: programme nomFichierEntree nomFichierSortie tempsEnSeconde\n");
		exit(0);
    }
    mkp = tp_load_mkp(argv[1]);
    int tempsMax = atoi(argv[3]);
    char *outputFileName = argv[2];
    char *instance = argv[1];

    /*Init liste tabou*/
    ListTabou *listTabou = init_tabou_list(15);

    sol = alloc_sol(mkp);
	init_sol(sol, mkp);
	printf("Probleme sac a dos : nbr objets : %d, nbr cc : %d, nbr cd : %d\n", mkp->n, mkp->cc, mkp->cd);
	/*On a initialis� la solution comme �tant un sac vide, on va ajouter tous les objets pour avoir une solution non r�alisable qui poss�de tous les objets
	Puis on va retirer les objets 1 � 1 pour arriver � une solution r�alisable*/
	remplirSac(mkp, sol);

	/*Il faut maintenant retirer les objets jusqu'� ce que la solution soit r�alisable*/
	obtenirSolutionRealisable(mkp, sol, 0);

    /*Maintenant on recherche une solution am�liorante*/

    /*copie de la sol initiale parce que s va surement �tre d�sallou� par parcoursVoisin*/
    bestS = copieSolution(mkp, sol);
    printf("Calcul en cours, patientez...\n");

    /**
    on va chercher des solutions, tant qu'on a du temps, on va d'abord utiliser la solution initiale qu'on vient de construire
    Puis on utilisera d'autres algorithmes afin de partir avec de nouvelles solutions initiales
    Une fois qu'on aura utilis� tous nos algos, on fera du random
    **/
    /*on utilise un compteur pour le choix des tris pour la solution initiale � plus de 5 en passe en choix random*/
    int cpt = 1;
    while(timeout(startTime, tempsMax, bestS, sAmeliorante, instance, outputFileName, mkp)) {
        timeout(startTime, tempsMax, bestS, sAmeliorante, instance, outputFileName, mkp);
        printf("recherche d'une solution...\n");
        sAmeliorante = parcoursVoisin(mkp, sol, 1, sol, listTabou, 0, 0, startTime, tempsMax, instance, outputFileName, bestS);
        printf("solution trouvee resultat de la fonction objectif : %d\n\n", sAmeliorante->objValue);
        /*On garde la meilleure des solutions entre bestS et sAmeliorante*/
        if (sAmeliorante->objValue > bestS->objValue) {
            /*On a trouv� mieux*/
            free_sol(bestS);
            bestS = sAmeliorante;
            sAmeliorante = NULL;
        }
        else {
            free_sol(sAmeliorante);
            sAmeliorante = NULL;
        }

        /*On reconstruit une nouvelle solution initiale, normalement sol est lib�r�e durant le parcours*/
        sol = NULL;
        sol = alloc_sol(mkp);
        init_sol(sol, mkp);
        remplirSac(mkp, sol);
        obtenirSolutionRealisable(mkp, sol, cpt);
        cpt++;
        /*On est pr�t pour recommencer*/
    }
    /*Lib�ration de la m�moire*/
    /*On ne devrait plus passer ici...*/
    free(listTabou->list);
    listTabou->list = NULL;
    free(listTabou);
    free_sol(bestS);
    free_sol(sAmeliorante);
    free(sol);
    tp_del_mkp(mkp);
	return 0;
}
