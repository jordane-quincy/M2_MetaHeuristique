#include "tp_mkpsol.h"

/************************************************************************
                        Allocation d'une solution
************************************************************************/
     Solution *alloc_sol(tp_Mkp *mkp)
 	 {
		  Solution *sol;
		  int i;

		  sol = (Solution *)malloc(sizeof(Solution));
		  CHECK(sol);
          sol->x = (int *)calloc(mkp->n + 1, sizeof(int));
		  CHECK(sol->x);
          for(i=0; i<=mkp->cc; i++)
			  sol->x[i] = 0;
          sol->slack = (int *)calloc(mkp->cc + 1, sizeof(int));
		  CHECK(sol->slack);
          for(i=0; i<=mkp->cc; i++)
			  sol->slack[i] = 0;

		  sol->objValue = 0;
		  return sol;
 	 }

/************************************************************************
                        Liberation d'une solution
************************************************************************/
   void free_sol(Solution *sol)
   {
      free(sol->x);
      free(sol->slack);
      free(sol);
   }

/************************************************************************
                        initialisation d'une solution
************************************************************************/
   void init_sol(Solution *sol, tp_Mkp *mkp)
   {
      int i;

      for(i=1; i<=mkp->cc; i++) sol->slack[i] = mkp->a[i][0];
      sol->slack[0] = 0;
      for(i=0; i<=mkp->n; i++) sol->x[i] = 0;
      sol->objValue = 0;
   }

/************************************************************************
retourne 1 si l'ajout de l'objet j dans la solution est faisable, 0 sinon
************************************************************************/
   int Is_Add_F(tp_Mkp *mkp, Solution *sol, int j)
   {
      int i;

      if(sol->x[j] == 1) {
         printf("verification si ajout possible mais variable deja a 1\n");
         return 0;
      }
      for(i=1; i<=mkp->cc; i++)
         if (mkp->a[i][j] > sol->slack[i])
            return (0);
      return (1);
   }

/************************************************************************
                 ajoute l'objet j dans la solution
************************************************************************/
   void Add(tp_Mkp *mkp, Solution *sol, int j)
   {
      int i;

      if(sol->x[j] == 1) {
         printf("tentative d'ajout d'une variable deja a 1\n");
         return;
      }
      sol->x[j] = 1;
      sol->objValue+= mkp->a[0][j];
      sol->slack[0] = 0;
      for(i=1; i<=mkp->cc; i++) {
         sol->slack[i]-= mkp->a[i][j];
         if(sol->slack[i] < 0) (sol->slack[0])++;
      }
      (sol->x[0])++;
   }

/************************************************************************
               retire l'objet j de la solution
************************************************************************/
   void Drop(tp_Mkp *mkp, Solution *sol, int j)
   {
      int i;

      if(sol->x[j] == 0) {
         printf("tentative de retrait d'une variable deja a 0\n");
         return;
      }
      sol->x[j] = 0;
      sol->slack[0] = 0;
      sol->objValue-= mkp->a[0][j];
      for(i=1; i<=mkp->cc; i++)
      {
         sol->slack[i]+= mkp->a[i][j];
         if(sol->slack[i] < 0) (sol->slack[0])++;
      }
      (sol->x[0])--;
   }

/************************************************************************
               Copie d'une solution
************************************************************************/
    Solution *copieSolution (tp_Mkp *mkp, Solution *sol) {
        int i;
        Solution *copie = alloc_sol(mkp);
        copie->objValue = sol->objValue;
        for (i = 0; i <= mkp->n; i++) {
            copie->x[i] = sol->x[i];
        }
        for (i = 0; i <= mkp->cc; i++) {
            copie->slack[i] = sol->slack[i];
        }
        return copie;
    }
