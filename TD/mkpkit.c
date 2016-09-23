#include "mkpkit.h"

/************************************************************************
   allocation mémoire du Mkp
************************************************************************/
   void alloc_mkp(Mkp *mkp)
   {
      int i, j;

      mkp->a = (int **)calloc(mkp->m + 1, sizeof(int *));
      CHECK(mkp->a);
      for (i = 0; i<=mkp->m; i++) {
         mkp->a[i] = (int *)calloc(mkp->n + 1, sizeof(int));
         CHECK(mkp->a[i]);
         for(j=0; j<=mkp->n; j++) mkp->a[i][j] = 0;
      }
   }

/************************************************************************
   chargement des données du Mkp depuis le fichier texte file
************************************************************************/
   Mkp *load_mkp(char *file)
   {
      int i, j;
      Mkp *mkp;
      FILE *fp;

      fp = fopen(file, "r");
      if (fp == NULL)
         return NULL;

      mkp = (Mkp *)malloc(sizeof(Mkp));		/* allocation de la "structure" Mkp */
      CHECK(mkp);
      fscanf(fp, "%d %d", &mkp->n, &mkp->m);
      alloc_mkp(mkp);						/* allocation des tableaux */

      for(j=1; j<=mkp->n; j++)
         fscanf(fp, "%d", *mkp->a + j);
      for(i=1; i<=mkp->m; i++)
         fscanf(fp, "%d", mkp->a[i]);
      for(i=1; i<=mkp->m; i++)
         for(j=1; j<=mkp->n; j++)
            fscanf(fp, "%d", mkp->a[i] + j);
      fclose(fp);
      mkp->a[0][0] = 0;

      return mkp;
   }

/************************************************************************
   sauvegarde du Mkp dans le fichier texte file
************************************************************************/
   void save_mkp(Mkp *mkp, char *file)
   {
      int i, j;
      FILE *fp;

      fp=fopen(file, "w");
      if(fp == NULL) {
    	  printf("Erreur ouverture du fichier %s\n", file);
    	  return ;
      }
      fprintf(fp, "%d %d\n\n", mkp->n, mkp->m);
      for (j = 1; j <= mkp->n; j++)
      {
         fprintf(fp, "%5d", mkp->a[0][j]);
         if(j % 10 == 0) fputs("\n", fp);
      }
      fputs("\n", fp);
      for (i = 1; i <= mkp->m; i++)
      {
         fprintf(fp, "%10d", mkp->a[i][0]);
         if(i % 10 == 0) fputs("\n", fp);
      }
      fputs("\n", fp);
      for (i = 1; i <= mkp->m; i++)
      {
         for (j = 1; j <=mkp->n; j++)
         {
            fprintf(fp, "%5d", mkp->a[i][j]);
            if(j % 10 == 0) fputs("\n", fp);
         }
         fputc('\n', fp);
      }
      fputs("\n", fp);

      fclose(fp);
   }

/************************************************************************
   libératon mémoire du Mkp
************************************************************************/
   void del_mkp(Mkp *mkp)
   {
      int i;

      for(i = 0; i <= mkp->m; i++) free(mkp->a[i]);
      free(mkp->a);
      free(mkp);
   }
