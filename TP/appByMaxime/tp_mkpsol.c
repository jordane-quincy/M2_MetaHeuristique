#include "tp_mkpsol.h"

/************************************************************************
                        Allocation d'une solution
************************************************************************/
    Solution *alloc_sol(tp_Mkp *mkp)
    {
        Solution *sol;
        printf("debut alloc sol\n");
        printf("sizeof solution : %d\n", sizeof(Solution));
        sol = (Solution *)malloc(sizeof(Solution));
        printf("apres alloc sol sizeof solution\n");
        CHECK(sol);
        sol->x = (int *)calloc(mkp->n + 1, sizeof(int));
        printf("apres alloc sol-x\n");
        CHECK(sol->x);
        //for(i = 0; i <= mkp->n; i++)
          //  sol->x[i] = 0;
        sol->slack = (int **)calloc(2, sizeof(int));
        printf("apres alloc slack[]\n");
        CHECK(sol->slack);
        sol->slack[0] = (int *)calloc(mkp->cc + 1, sizeof(int));
        printf("apres alloc slack[0][]\n");
        CHECK(sol->slack[0]);
        sol->slack[1] = (int *)calloc(mkp->cd + 1, sizeof(int));
        printf("apres alloc slack[1][]\n");
        CHECK(sol->slack[1]);
        return sol;
    }

/************************************************************************
                        Liberation d'une solution
************************************************************************/
    void free_sol(Solution *sol)
    {
        int i;

        free(sol->x);
        for(i = 0;i < 2;i++) free(sol->slack[i]);
        free(sol->slack);
        free(sol);
   }

/************************************************************************
                        initialisation d'une solution
************************************************************************/
void init_sol(Solution *sol, tp_Mkp *mkp)
{
    int i;
    //Initialisation de la slack des cc
    for(i=1; i<=mkp->cc; i++) sol->slack[0][i] = mkp->a[i][0];
    //Toutes les cc sont respectées
    sol->slack[0][0] = 0;
    //Initialisation de la slack des cd
    for (i = 1; i <= mkp->cd; i++) {
        sol->slack[1][i] = -(mkp->a[mkp->cc + i][i]);
    }
    //Aucune cd n'est respectée
    sol->slack[1][0] = mkp->cd;
    //Initialisation du sac (on ne met aucun objet)
    for(i=0; i<=mkp->n; i++) sol->x[i] = 0;
    //fonction objective => 0
    sol->objValue = 0;
}

/************************************************************************
retourne 1 si l'ajout de l'objet j dans la solution est faisable, 0 sinon
************************************************************************/
    int isAddPossible(tp_Mkp *mkp, Solution *sol, int j)
    {
        int i;
        if(sol->x[j] == 1)
        {
            printf("Verification si ajout possible mais variable deja a 1 !\n");
            return 0;
        }
        //Restrictions contraintes de capacité
        for(i = 1; i <= mkp->cc; i++)
            if (mkp->a[i][j] > sol->slack[0][i])
            {
                return 0;
            }
        return (1);
    }

/************************************************************************
retourne 1 si le retrait de l'objet j dans la solution est faisable, 0 sinon
*************************************************************************/

    int isRemovePossible(tp_Mkp *mkp, Solution *sol, int j) {
        int i;
        if (sol->x[j] == 0)
        {
            printf("Verification si retrait possible mais l'objet n'est pas present dans le sac !\n");
            return 0;
        }
        //Check des contraintes de demandes
        for(i = 1; i <= mkp->cd; i++)
            if (mkp->a[mkp->cc + i][j] > sol->slack[1][i])
            {
                return 0;
            }
        return (1);
    }

/************************************************************************
                 ajoute l'objet j dans la solution
************************************************************************/

    void Add(tp_Mkp *mkp, Solution *sol, int j)
    {
        int i;

        if(sol->x[j] == 1)
        {
            printf("tentative d'ajout d'une variable deja a 1\n");
            return;
        }
        //On ajoute l'objet dans la solution
        sol->x[j] = 1;
        //On ajoute la valeur de l'objet à notre fonction objectif
        sol->objValue+= mkp->a[0][j];
        //On reset le nbr de contraintes de capacité respectées
        sol->slack[0][0] = 0;
        //On parcours toutes les cc pour mettre à jour la slack de cc
        for(i=1; i<=mkp->cc; i++)
        {
            //On retire le poids de cc de l'objet pour la contrainte en cours pour la slack
            sol->slack[0][i]-= mkp->a[i][j];
            //Si la cc n'est pas respectée on ajoute 1 au nombre de cc non respectées
            if(sol->slack[0][i] < 0) (sol->slack[0][0])++;
        }
        //On reset le nbr de contraintes de demande(cd) respectées
        sol->slack[1][0] = 0;
        //On parcours toutes les cd pour mettre à jour la slack de cd
        for (i = 1; i <= mkp->cd; i++) {
            //On ajoute le poids de cd de l'objet pour la contrainte en cours
            sol->slack[1][i] += mkp->a[mkp->cc + i][j];
            //Si la cd n'est pas respectée on ajoute 1 au nombre de cd non respectées
            if (sol ->slack[1][i] < 0) (sol->slack[1][0])++;
        }
        (sol->x[0])++;
    }

/************************************************************************
               retire l'objet j de la solution
************************************************************************/
    void Drop(tp_Mkp *mkp, Solution *sol, int j)
    {
        int i;

        if(sol->x[j] == 0)
        {
            printf("tentative de retrait d'une variable deja a 0\n");
            return;
        }
        //On retire l'objet de la solution
        sol->x[j] = 0;
        //On retire la valeur de l'objet à notre fonction objectif
        sol->objValue-= mkp->a[0][j];

        //On reset le nbr de contraintes de capacité respectées
        sol->slack[0][0] = 0;
        //On parcours toutes les cc pour mettre à jour la slack de cc
        for(i=1; i<=mkp->cc; i++)
        {
            //On retire le poids de cc de l'objet pour la contrainte en cours pour la slack
            sol->slack[0][i]+= mkp->a[i][j];
            //Si la cc n'est pas respectée on ajoute 1 au nombre de cc non respectées
            if(sol->slack[0][i] < 0) (sol->slack[0][0])++;
        }
        //On reset le nbr de contraintes de demande(cd) respectées
        sol->slack[1][0] = 0;
        //On parcours toutes les cd pour mettre à jour la slack de cd
        for (i = 1; i <= mkp->cd; i++)
        {
            //On ajoute le poids de cd de l'objet pour la contrainte en cours
            sol->slack[1][i] -= mkp->a[mkp->cc + i][j];
            //Si la cd n'est pas respectée on ajoute 1 au nombre de cd non respectées
            if (sol ->slack[1][i] < 0) (sol->slack[1][0])++;
        }

        (sol->x[0])--;
    }

/************************************************************************
               Copie d'une solution
************************************************************************/
    Solution *copieSolution (tp_Mkp *mkp, Solution *sol){
        int i;
        printf("debut copie sol\n");
        Solution *copie = alloc_sol(mkp);
        printf("apres alloc sol\n");
        //On copie la valeur de la fonction objectif
        copie->objValue = sol->objValue;
        //On copie le fait qu'on prenne ou pas les objets
        for (i = 0; i <= mkp->n; i++) {
            copie->x[i] = sol->x[i];
        }
        //On copie les 2 slacks, celle pour les cc et celle pour les cd
        for (i = 0; i <= mkp->cc; i++) {
            //copie slack cc
            copie->slack[0][i] = sol->slack[0][i];

        }
        for (i = 0; i <= mkp->cd; i++) {
            //copie slack cd
            copie->slack[1][i] = sol->slack[1][i];
        }
        return copie;
    }
