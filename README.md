# M2_MetaHeuristique

max 2x1 + 4x2 + 5x3 + 2x4
s.c
	x1 + 2x2 + 2x3 + x4 <= 3
	2x1 + 2x2 + 4x3 + 2x4 <= 6
	2x1 + 3x2 + x3 + 2x4 <= 4
	
	x1, x2, x3, x4 € { 0, 1 }

nb de variables : n =  4
nb de contraintes : m = 3

		mkp
un entier n
un entier m
un tableau 2D a

a

		2	4	5	2	<-- fonction objet
	3	1	2	2	1
	6	2	2	4	2
	4	2	3	1	2

	^
	|
	| membre droit des contraintes
	
	
		Solution
un entier objalue
un tableau x

x
0	1	2	3	4
X	1	0	0	1

un tableau slack (ce qui reste comme places libres dans le sac)
0	1	2	3
	1	2	0	<-- bi - Somme de j=1 à n aijxj
^
| Si valeur négative ici == nombre de contraintes non respectée (must be avoided !)


Hello gentlmen,
Bon je vous propose une solution sahant que je n'ai pas modifier toutes les fonctions, ne soyons pas préssé.
Dites moi ce que vous en pensez, au pire on verra lundi, c'est pas comme si on été en avance :p

Sur ce bon dimanche !