#include "mkpkit.h"
#include "mkpsol.h"

typedef struct {
     int indexObj;
     double ratio;
} ObjRatio;

ObjRatio* alloc_tab(int nbrVar) {
    ObjRatio *tab;
    tab = (ObjRatio *)malloc(sizeof (ObjRatio) * nbrVar);
    return tab;
}

ObjRatio* getTableauOrdonne (Mkp mkp, ObjRatio* tabOrdonne) {
    int i;
    int j;
    for (i = 1; i <= mkp.n; i++) {
        ObjRatio objRatio;
        objRatio.indexObj = i;
        double coefObj = mkp.a[0][i];
        double poidsObj = 0;
        for (j = 1; j <= mkp.m; j++) {
            poidsObj += mkp.a[j][i];
        }
        objRatio.ratio = coefObj/poidsObj;

    }
    return tabOrdonne;
}

int main(int argc, char *argv[]) {
	Mkp *mkp;
	Solution *s;
	ObjRatio *tabOrdonne;

	if(argc != 2) {
		printf("Usage: programme fichier\n");
		exit(0);
	}
	mkp = load_mkp(argv[1]);
    tabOrdonne = alloc_tab(mkp->n);
	tabOrdonne = getTableauOrdonne(*mkp, tabOrdonne);

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
