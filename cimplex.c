
/*
 * Simplex algorithm by Dantzig
 * Written by Chouri Soulaymen (email: sorakun@kegtux.org website: http://sorakun.kegtux.org)
 *
 * Copyright:
 *
 * This software is in the public
 * domain and is provided without express or implied warranty.  Permission to
 * use, modify, or distribute the software for any purpose is hereby granted.
 *
 */

/* La méthode de résolution implémenté est la méthode des 2 phases */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Epsilon, la précision pour la comparaison des nombres flottants */
#define EPSILON 0.000001
#define USE_DL_PREFIX
int constCount; /* Nombre des contraintes */
int varCount;   /* Nombre des variables */

int X; /* X de la matrice */
int Y; /* Y de la matrice */

float ** mat;     /* La matrice */
int * base; /* Les indices des variables dans la base */
float * obj; /* Fonction objective originale, en cas ou méthode de deux phases */

char ** names;

int max; /* Maximisation (= 1) ou minimization (= -1)*/

int varArt;   /* Nombre des variables artificielles */
int varEcart; /* Nombre des variables d'écart */

int isComplex; /* Si on doit passer par la méthode de deux phases, ou pas */

int count;

/*
 * Lecture des nombre des contraintes et variables,
 * et initialisation de la matrice
 */

void readVars()
{
	varArt = 0;
	varEcart = 0;

    isComplex = 0;

	printf("Donner le nombre des contraintes: ");
	scanf("%d", &constCount);
	printf("\nDonner le nombre des variables: ");
	scanf("%d", &varCount);

	/* Initialisation du matrice, et du vecteur base */

	X = varCount + constCount + 1;
	Y = constCount+1;

	mat = calloc(Y, sizeof(float*));
	base = calloc(Y-1, sizeof(float));

	int i = 0;
	for (; i < Y; i++)
		mat[i] = calloc(X, sizeof(float));


	for (i = 0; i <= Y-1; i++)
		base[i] = varCount+i;
}

/*
 * Lecture de la matrice
 */

 void readmatrix()
 {
 	/* Lecture de la fonction objective */
 	printf("Probleme de maximisation ou minimisation? (respectivement 1 ou 0):\n");
 	scanf("%d", &max);
 	if (max == 0)
 		max = -1;
 	else
 		max = 1;

 	printf("Lecture de la fonction objective:\n");
 	int i = 0;
 	for(; i < varCount; i++)
 	{
 	    float v = 0;
 		printf("X%d * ", i+1);
 		scanf("%f", &v);
 		mat[Y-1][i] = v*max;
    }

 	/* Normallement c'est init à 0 par calloc, mais en cas d'échec, on fait l'init */
 	for(i = varCount; i < X; i++)
 		mat[Y-1][i] = 0;
 	printf("Lecture des contraintes:\n");
 	/* Lecture des contraintes */
 	for(i = 0; i < Y-1; i++)
 	{
 		printf("Contrainte Numero %d:\n", i+1);
 		int j = 0;
 		for(; j < varCount; j++)
 		{
 			printf("X%d * ", j +1);
 			scanf("%f", &mat[i][j]);
 		}


 		printf(" =, <= or >= (0, 1, 2) : ");
 		int mode;
 		scanf("%d", &mode);

 		if (mode == 0)
 		{
 			mat[i][j+i] = 0;
 			isComplex = 1;
 		}
 		else if (mode == 1)
 		{
 			mat[i][j+i] = 1;
 		}
 		else
 		{
 			mat[i][j+i] = -1;
 			isComplex = 1;
 		}

 		scanf("%f", &mat[i][X-1]);
 		printf("\n");
 	}
 }


void printMatrix(int c)
{
    printf("\n\n");
    if (c != -1)
        printf("Cycle %d:\n", c);
	int i = 0, j = 0;
	for(; i < Y; i++)
	{
		if(i < Y-1)
			printf("%d", base[i]+1);
        printf("\t");

		for(j = 0; j < X; j++)
			printf("X%d * %4.2f\t", j+1, mat[i][j]);

		printf("\n");
	}
}

/*
 * Cherche si un vecture s'exprime en fonction des variables artificielles ou d'écarts.
 * Si oui, cette case va ette additionner a fin d'avoir la nouvelle
 * fonction objective a resoudre.
 */

int toSum(int raw, int ecart)
{
    int i = 0;
    if (!ecart)
    {
    for(i = varCount+varEcart; i < X-1; i++)
        if(mat[raw][i] == 1) /* Il fait partie des variables artificielles */
            return 1;
    return 0;
    }

    for(i = varCount; i < varCount+varEcart; i++)
        if(mat[raw][i] == 1) /* Il fait partie des variables artificielles */
            return 1;
    return 0;
}
/*
 * Permet d'allouer et d'initializer de l'espace pour les
 * variables articificielles (s'in existent)
 */

void rewriteMatrix()
{
    obj = NULL;

    if(!isComplex)
        return;

    //printMatrix(0);
	int i = 0;
	int j = 0;
	int toAdd = 0; /* Nombre des cases a rajouter */

	for(; i < Y-1; i++)
	{
		for(j = varCount; j < X-1; j++)
		{
			/* <=, rien a alloué, on incrémente le nombre des variables d'écart */
			if (mat[i][j] == 1)
			{
				varEcart++;
				break;
			}
			/* >=, on a une variable d'écart, déja alloué, et on a besoin d'une variable artificielle a alloué */
			else if (mat[i][j] == -1)
			{
				varEcart++;
				toAdd++;
				break;
			}
			/* =, rien a réalloué, la variable artificielle a déja de l'espace, donc on n'icrémente pas le nombre des variable d'écart */
		}
	}

	//printf("On va ajouter %d cases\n", toAdd);

	/* Allocation */
	float ** mat2 = calloc(Y, sizeof(float*));

	int cArt = 0; /* compteur */

	for(i = 0; i < Y; i++)
		mat2[i] = calloc(X+toAdd, sizeof(float));

	/* Transfert des élément de base */

    for(i = 0; i < Y; i++)
    {
        for(j = 0; j < varCount; j++)
        {
            mat2[i][j] = mat[i][j];
            //printf("copying %d, %d, %4.2f\n", i, j, mat[i][j]);
        }
    }

	for(i = 0; i < Y-1; i++)
	{
		/* transfert de la dérniére colonne */
		mat2[i][X+toAdd-1] = mat[i][X-1];
		//printf("transfrt [%d][%d] to [%d][%d] value %4.2f\n", i, X+toAdd-1, i, X-1);
	}

	for(i = 0; i < Y-1 ; i++)
    {
        int found = 0;
		for(j = varCount; j < X-1; j++)
		{
			if (mat[i][j] == 1)
			{
			    //printf("writing 1 to %d, %d\n", i, j);
				mat2[i][j] = 1;
				found = 1;
			}
			else if (mat[i][j] == -1)
			{
				mat2[i][j] = -1;
				//printf("writing %d, %d\n", i, varCount+varEcart+cArt);
				mat2[i][varCount+varEcart+cArt] = 1;
				//printf("Written %4.2f\n", mat2[i][varCount+varEcart+cArt]);
				cArt++;
				found = 1;
			}
		}
		/* Si tout les variables d'écart sont a 0, on ajoute une variable artificielle */
		if (!found)
        {
            //printf("not found writing %d, %d\n", i, varCount+varEcart+cArt);
			mat2[i][varCount+varEcart+cArt++] = 1;
        }
    }

	free(mat);
	X += toAdd;
	varArt = X-varCount - varEcart - 1;
	mat = mat2;
	/* Sauvegarde de l'ancienne fonction objective, et vidage de l'ancienne */
    obj = calloc(varCount, sizeof(float));

    for(i = 0; i < varCount; i++)
    {
        obj[i] = mat[Y-1][i];
        mat[Y-1][i] = 0;
    }


    /* Mettre a jour la nouvelle valeur de la fonction objective */
    for(i = 0; i < Y-1; i++)
    {
        if (toSum(i, 0))
        {
            /* Modification de la fonction */
            //printf("Summing line %d\n", i);
            for(j = 0; j < varCount+varEcart; j++)
                mat[Y-1][j] += mat[i][j];
            /* modification de la constante */
            mat[Y-1][X-1] += mat[i][X-1];
        }
    }

    /* On cherche les éléments de base */
    int bPtr = 0; /* pointeur a la case du vecteur base a inserer */
    for(i = 0; i < Y-1; i++)
        for(j = varCount; j < X-1; j++)
            if (mat2[i][j] == 1)
                base[bPtr++] = j;

	printMatrix(1);

}

/*
 * Permet de chercher le vecteur sortant
 * renvoie -1 en cas de saturation
 */

int findInVect()
{
    /* On suppose que le 1er element est le max */
	int i = 1;
	int posMax = 0;
	for(; i<X-1; i++)
    {
        //printf("%4.2f ", mat[Y-1][i]);
		if (mat[Y-1][i] >= mat[Y-1][posMax])
				posMax = i;
    }
    printf("\n");
    printf("VectIn: %d\n", posMax);

    /* Epsilon trés petit, le nombre si < EPS, il est considéré nul */
	if(mat[Y-1][posMax] < EPSILON)
        return -1;
	return posMax;
}

/*
 * Cherche le vecteur sortant
 * A partir du parametre donné qui
 * est le vecteur entrant
 */

int findOutVect(int inVect)
{
	int i = 0;
	int minPos = -1;

	for(i = 0; i < Y-1; i++)
    {
        if (mat[i][X-1] / mat[i][inVect] < 0)
        {
            continue;
        }

        if (mat[i][inVect] == 0)
            continue;

        if((mat[i][inVect] != 0) && (minPos == -1))
        {
            minPos = i;
            continue;
        }

        //printf("comparing %4.2f %d with %4.2f %d\n", mat[i][X-1] / mat[i][inVect], i, mat[minPos][X-1] / mat[minPos][inVect], minPos);
		if (mat[i][X-1] / mat[i][inVect] <= mat[minPos][X-1] / mat[minPos][inVect])
		if (mat[i][X-1] / mat[i][inVect] <= mat[minPos][X-1] / mat[minPos][inVect])
			minPos = i;
    }
	/* minPos c'est n'est que l'indice du tableau, l'indice du vecteur sortant est dans le vecteur base */
	printf("VectOut: %d\n", minPos);
	return minPos;
}

/*
 * Fait la mise à jour du vecteur pivot
 * x et y sont les coordonnés du vecteur pivot
 */

void updatePivotVect(int x, int y)
{
	printf("pivot = %4.2f\n", mat[x][y]);
	float pivot = mat[x][y];
	int i = 0;
	//printf("x = %d, y = %d\n", x, y);
	for(; i < X; i++)
    {
        //printf("%4.2f / %4.2f\t", mat[x][i], pivot);
        mat[x][i] = mat[x][i] / pivot;
    }
    //printf("\n");
}

/*
 * Elle change les valeurs de la matrice selon la ligne du pivot à 0
 * x et y sont les coordonnés du vecteur pivot
 */

void updateMatrix(int x, int y)
{
	float pivot = mat[x][y];
	int i = 0, j = 0;
	float valeur = 0; /* C'est la valeur dont on va utiliser pour annuler les colonnes du pivot valeur = - M[i][j] / pivot */

	for(; i < Y; i++)
	{
		if (i == x)
			continue;

		valeur = - mat[i][y];
        printf("L%d += %4.2f * %4.2f\n", i+1, valeur, pivot);
		for(j = 0; j < X; j++)
			mat[i][j] += valeur * mat[x][j];
//        printf("L%d = ", i+1);
//        for(j = 0; j < X; j++)
//			printf("%4.2f\t", mat[i][j]);
//        printf("\n");
	}
}

/* Vérifie si un indice n'existe pas dans la base */
int outsideBase(int index)
{
    int i = 0;
    for(i = 0; i < Y-1; i++)
        if (base[i] == index)
            return 0; /* Existe */
    return 1; /* n'existe pas */
}

/*
 * Supprime les colonnes des variables artificielles
 * et fait la mise à jour des delta-j.
 */
void resetMatrix()
{
    int i = 0, j= 0;
    for(i = 0; i < X-1; i++)
        mat[Y-1][i] = 0;

    printf("\tOn a %d variable artificielle.\n", varArt);
    for(i = 0; i < Y; i++)
    {
        mat[i][X-1-varArt] = mat[i][X-1];
        mat[i]= realloc(mat[i], (X-varArt+1)*sizeof(int*));
    }


    X -= varArt;

    /* mettre la fonction objective en fonction des variables hors base */
    for(i = varCount; i < X-1; i++)
        mat[Y-1][i] = 0;

    printMatrix(-1);

    for(i = 0; i < Y-1; i++)
    {
        if (base[i] >= varCount)
            continue;
        for(j = 0; j < varCount; j++)
        {
            if (outsideBase(j))
            {
                if (mat[i][j] != 0)
                    mat[Y-1][X-1] += obj[base[i]]*mat[i][X-1];

                mat[Y-1][j] += -obj[base[i]]*mat[i][j]+obj[j];
            }
            else
                continue;
        }
    }
    mat[Y-1][X-1] *= -1;

    printMatrix(-1);
}
void doCycle()
{
	int vectIn = 0;
	int vectOut = 0;
	int i = 0, j = 0;

	if (isComplex)
    {
        isComplex = 0;
        doCycle();
        if ( mat[Y-1][X-1] >= EPSILON )
        {
            printf("Ce système est irréalisable.\n");
            exit(0);
        }
        /* Mettre a jour de la base */
        resetMatrix();
        isComplex = 1;
    }

    i = 0;
	while(-1 != (vectIn = findInVect()))
	{
		vectOut = findOutVect(vectIn);
		updatePivotVect(vectOut, vectIn);
		updateMatrix(vectOut, vectIn);
		/* Mise à jour de la base */
		base[vectOut] = vectIn;
        printMatrix(i++);
//        int x;
//        scanf("%d", &x);
	}
	count = i;
	/* Vérifier s'il existe des variables */
	//if (isComplex)
}

void finalise()
{
	printf("Les variables dans la base sont: \n");
	int i = 0;
	printf("\tX = (");
	for(; i < Y-1; i++)
    {
		printf("X%d = %4.2f", base[i]+1, mat[i][X-1]);
		if (i < Y-2)
            printf(", ");
    }
    printf(")\nAvec Z = %f\n", -mat[Y-1][X-1]);
}


int main(int argc, char ** argv)
{
    count = 0;
    readVars();
    readmatrix();
    rewriteMatrix();
    finalise();
    //printMatrix(-1);
    doCycle();
    finalise();
    return 0;
}
