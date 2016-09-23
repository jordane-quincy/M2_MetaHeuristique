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

    for (i = 1; i <= 8; i++) {
        printf("Obj index : %d, obj ratio : %lf\n", tabOrdonne[i].indexObj, tabOrdonne[i].ratio);
    }
    for (j = 1; j<= mkp->n; j++) {
        ordre[j] = tabOrdonne[j].indexObj;
    }
    return ordre;
}

int main(int argc, char *argv[]) {
	Mkp *mkp;
	Solution *s;
	int *ordre;
	int i;

	if(argc != 2) {
		printf("Usage: programme fichier\n");
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

    printf("obj = %d\n", s->objValue);


	/*save_mkp(mkp, "verif.txt");

	s = alloc_sol(mkp);
	Add(mkp, s, 1);
	Add(mkp, s, 10);
	printf("obj= %d\n", s->objValue);
	Drop(mkp, s, 10);
	printf("obj= %d\n", s->objValue);

	free_sol(s);*/
	del_mkp(mkp);

	return 0;
}
