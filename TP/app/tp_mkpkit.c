#include "tp_mkpkit.h"

/************************************************************************
   Allocation mémoire du Mkp
************************************************************************/
   void tp_alloc_mkp(tp_Mkp *mkp)
   {
      int i, j;

      mkp->a = (int **)calloc((mkp->cc + mkp->cd) + 1, sizeof(int *));
      CHECK(mkp->a);
      for (i = 0; i <= (mkp->cc + mkp->cd); i++) {
         mkp->a[i] = (int *)calloc(mkp->n + 1, sizeof(int));
         CHECK(mkp->a[i]);
         for(j=0; j<=mkp->n; j++) mkp->a[i][j] = 0;
      }
   }

/************************************************************************
   Chargement des données du Mkp depuis le fichier texte file
************************************************************************/
   tp_Mkp *tp_load_mkp(char *file)
   {
      int i, j;
      tp_Mkp *mkp;
      FILE *fp;

      fp = fopen(file, "r");
      if (fp == NULL)
         return NULL;

      mkp = (tp_Mkp *)malloc(sizeof(tp_Mkp));		/* allocation de la "structure" Mkp */
      CHECK(mkp);
      fscanf(fp, "%d %d %d", &mkp->n, &mkp->cc, &mkp->cd);
      printf("ERROR ?\n%d %d %d\n", mkp->n, mkp->cc, mkp->cd);
      tp_alloc_mkp(mkp);						/* allocation des tableaux */

      for(j = 1; j <= mkp->n; j++)//getting n values
         fscanf(fp, "%d", *mkp->a + j);
      for(i = 1; i <= mkp->cc; i++)//Capacités
         fscanf(fp, "%d", mkp->a[i]);
      for(i = mkp->cc + 1; i <= mkp->cd + mkp->cc; i++)//Demandes
         fscanf(fp, "%d", mkp->a[i]);

      for(i = 1; i <= mkp->cc; i++)
         for(j = 1; j <= mkp->n; j++)
            fscanf(fp, "%d", mkp->a[i] + j);
      for(i = mkp->cc + 1; i <= mkp->cd + mkp->cc; i++)
         for(j=1; j <= mkp->n; j++)
            fscanf(fp, "%d", mkp->a[i] + j);
      fclose(fp);
      mkp->a[0][0] = 0;

      return mkp;
   }

/************************************************************************
   Sauvegarde du Mkp dans le fichier texte file
************************************************************************/
   void tp_save_mkp(tp_Mkp *mkp, char *file)
   {
      int i, j;
      FILE *fp;

      fp=fopen(file, "w");
      if(fp == NULL) {
    	  printf("Erreur ouverture du fichier %s\n", file);
    	  return ;
      }
      fprintf(fp, "%d %d\n\n", mkp->n, mkp->cc);
      for (j = 1; j <= mkp->n; j++)
      {
         fprintf(fp, "%5d", mkp->a[0][j]);
         if(j % 10 == 0) fputs("\n", fp);
      }
      fputs("\n", fp);
      for (i = 1; i <= mkp->cc; i++)
      {
         fprintf(fp, "%10d", mkp->a[i][0]);
         if(i % 10 == 0) fputs("\n", fp);
      }
      fputs("\n", fp);
      for (i = mkp->cc + 1; i <= mkp->cd + mkp->cc; i++)
      {
         fprintf(fp, "%10d", mkp->a[i][0]);
         if(i % 10 == 0) fputs("\n", fp);
      }
      fputs("\n", fp);
      for (i = 1; i <= mkp->cc; i++)
      {
         for (j = 1; j <=mkp->n; j++)
         {
            fprintf(fp, "%5d", mkp->a[i][j]);
            if(j % 10 == 0) fputs("\n", fp);
         }
         fputc('\n', fp);
      }
      fputs("\n", fp);
      for (i = mkp->cc + 1; i <= mkp->cd + mkp->cc; i++)
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
   void tp_del_mkp(tp_Mkp *mkp)
   {
      int i;

      for(i = 0; i <= mkp->cc; i++) free(mkp->a[i]);
      for(i = mkp->cc + 1; i <= mkp->cd + mkp->cc; i++) free(mkp->a[i]);
      free(mkp->a);
      free(mkp);
   }
