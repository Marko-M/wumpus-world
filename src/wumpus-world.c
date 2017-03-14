/*
 ============================================================================
 Name        : wumpus-world.c
 Author      : Marko Martinović
 Version     : 1.0
 Description : Wumpus world in C
 License : GPLv2

My Wumpus world exists inside matrix. Every element of that matrix represents
one field and is represented using 12 bit flag vector with "1" in place of the
field property that is true for that field. Initially all fields inside Wumpus
world are init to "000000000000" because their status is unknown. The real
Wumpus world lives separated from Wumpus world being processed and its
properties are assigned at the beginning of the program according to the defined
params.

	Dec	 -  Hex  - Bin			 -  Description	    - Symbol
	-----------------------------------------
	2048 -  800  - 100000000000  -  Wumpus			- W
	1024 -  400  - 010000000000  -  Cave			- C
	512  -  200  - 001000000000  -  Glow			- G
	256  -  100  - 000100000000  -  WumpusSus		- w
	128  -  80   - 000010000000  -  CaveSus			- s
	64   -  40   - 000001000000  -  GlowSus			- g
	32   -  20   - 000000100000  -  Breeze			- B
	16   -  10   - 000000010000  -  Stench			- T
	8    -  8    - 000000001000  -  Glowing			- L
	4    -  4    - 000000000100  -  Visited			- V
	2    -  2    - 000000000010  -  Safe			- S
	1    -  1    - 000000000001  -  Current			- A
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>			
#include <math.h>
#define NOROWS 		4		
#define NOCOLS 		4
#define NOWUMPUS 	1		
#define NOGLOW   	1
#define NOFBITS	   	12
#define WUMPUS      2048
#define CAVE        1024
#define GLOW        512
#define WUMPUSSUS   256
#define CAVESUS     128
#define GLOWSUS     64
#define BREEZE      32
#define STENCH      16
#define GLOWING     8
#define VISITED     4
#define SAFE 	    2
#define CURRENT     1
struct coord{
	int x;
	int y;
};
int arrow=1;
int NOCAVES;
int performance=0;
int realWorld[NOCOLS][NOROWS];
void displayWorldSymbols(int world[NOROWS][NOCOLS]);
void displayWorldAll(int world[NOROWS][NOCOLS]);
void neighborFieldsCoords(struct coord field, struct coord results[4]);
void setFlag(int world[NOCOLS][NOROWS], struct coord field, int action);
int randInt(int min, int max, int *except, int noExcept);
void generateRealWorld(int realWorld[NOCOLS][NOROWS]);
void evaluateNeighbors(int agentWorld[NOCOLS][NOROWS], struct coord field);
void moveOneField(int agentWorld[NOCOLS][NOROWS], int direction, struct coord srcField );
void delFlag(int world[NOCOLS][NOROWS], struct coord field, int action);
void copyFlags(int fromWorld[NOCOLS][NOROWS], int toWorld[NOCOLS][NOROWS], struct coord field);
int testFlag(int world[NOCOLS][NOROWS], struct coord field, int action);
struct coord getCurrentCoord(int world[NOCOLS][NOROWS]);
int calcDirrection(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField);
double calcDistance(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField);
int takeGlowAction(int agentWorld[NOCOLS][NOROWS], struct coord field);
int killWumpus(int agentWorld[NOCOLS][NOROWS], struct coord field);
int takeSafeAction(int agentWorld[NOCOLS][NOROWS], struct coord field);
int takeRollTheDiceAction(int agentWorld[NOCOLS][NOROWS], struct coord field);
int takeSuicideAction(int agentWorld[NOCOLS][NOROWS], struct coord field);
void myPause (void);

int main(int argc, char *argv[]) {
	int realWorld[NOCOLS][NOROWS];	
	int agentWorld[NOCOLS][NOROWS];	
	int x, y;						
	struct coord currentCoord;		
	struct coord start = {0, 0};	
	srand((unsigned int)time(0));	
	for(y=0; y<NOROWS; y++)
	{
		for(x=0; x<NOCOLS; x++){
			realWorld[x][y] = 0;
			agentWorld[x][y] = 0;
		}
	}
	generateRealWorld(realWorld);
	setFlag(agentWorld, start, CURRENT);
	currentCoord = start;	
	puts("R E A L  W O R L D");
	displayWorldAll(realWorld);
	int count = 0;
	while(1){
		printf("\n============================================================\n");
		count++;		
		currentCoord = getCurrentCoord(agentWorld);
		if(testFlag(agentWorld, currentCoord, VISITED) == 0){
			copyFlags(realWorld,agentWorld,currentCoord);
			if(testFlag(agentWorld, currentCoord, CAVE)){
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);
				performance=performance-1000;
				printf("\nWent down the cave. GAME OVER! Performance measure: %d",performance);
				break;
			} else if(testFlag(agentWorld,currentCoord,WUMPUS)){
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);
				performance=performance-1000;
				printf("\nWumpus had a good meal. GAME OVER! Performance measure: %d",performance);
				break;
			} else if(testFlag(agentWorld,currentCoord,GLOW)){
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);
				performance=performance+1000;
				printf("\nCongratulation! You have found gold!\n Agent goes back to (0,0) and exits cave!\nPerformance measure: %d",performance);
				break;
			} else{
				setFlag(agentWorld, currentCoord, VISITED);
				evaluateNeighbors(agentWorld,currentCoord);
			}
		}
		printf("\nA G E N T  W O R L D (Step %d)\n",count);
		printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
		displayWorldAll(agentWorld);

		if(takeGlowAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeGlowAction method!\n");
		} else if(takeSafeAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeSafeAction method!\n");
		} else if(killWumpus(agentWorld, currentCoord));
		else if(takeRollTheDiceAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeRollTheDiceAction method!\n");
		} else if(takeSuicideAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeSuicideAction method, situation was hopeless!\n");
		}
		myPause();
	}
	myPause();
	return EXIT_SUCCESS;
}
struct coord getCurrentCoord(int world[NOCOLS][NOROWS]){
	int x,y;
	struct coord currentCoord = { -1, -1 };
	for(x=0; x<NOROWS; x++){
		for(y=0; y<NOCOLS; y++){
			struct coord testCoord = { x, y };
			if(testFlag(world, testCoord, CURRENT)){
				currentCoord = testCoord;
			}
		}
	}
	if(currentCoord.x == -1 || currentCoord.y == -1){
		fprintf(stderr, "Something went wrong! We have managed to misplace our agent.\n");
		exit(EXIT_FAILURE);
	}else{
		return currentCoord;
	}
}

void displayWorldAll(int world[NOROWS][NOCOLS]){
	displayWorldSymbols(world);
	printf("\nLEGEND\nWumpus (W), Cave (C), Glow (G), WumpusSus (w), CaveSus (s), GlowSus (g), Breeze (B), Stench (T), Glowing (L), Visited (V), Safe (S), Current (A)\n\n");
	fflush(stdout);
}
void displayWorldSymbols(int world[NOROWS][NOCOLS]){
	int x, y;
	for(y=NOROWS-1; y>=0; y--){
		for(x=0; x<NOCOLS; x++){
			struct coord current = {x, y};
			if(testFlag(world, current, WUMPUS)){
				printf("W");
			} else{
				printf("_");
			}
			if(testFlag(world, current, CAVE)){
				printf("C");
			} else{
				printf("_");
			}
			if(testFlag(world, current, GLOW)){
				printf("G");
			} else{
				printf("_");
			}
			if(testFlag(world, current, WUMPUSSUS)){
				printf("w");
			} else{
				printf("_");
			}
			if(testFlag(world, current, CAVESUS)){
				printf("c");
			} else{
				printf("_");
			}
			if(testFlag(world, current, GLOWSUS)){
				printf("g");
			} else{
				printf("_");
			}
			if(testFlag(world, current, BREEZE)){
				printf("B");
			} else{
				printf("_");
			}
			if(testFlag(world, current, STENCH)){
				printf("T");
			} else{
				printf("_");
			}
			if(testFlag(world, current, GLOWING)){
				printf("L");
			} else{
				printf("_");
			}
			if(testFlag(world, current, VISITED)){
				printf("V");
			} else{
				printf("_");
			}
			if(testFlag(world, current, SAFE)){
				printf("S");
			} else{
				printf("_");
			}
			if(testFlag(world, current, CURRENT)){
				printf("A");
			} else{
				printf("_");
			}
			printf("\t");
		}
		printf("\n");
	}
}
void neighborFieldsCoords(struct coord field, struct coord results[4]){
	results[0].x = field.x;
	results[0].y = field.y + 1;
	results[1].x = field.x + 1;
	results[1].y = field.y;
	results[2].x = field.x;
	results[2].y = field.y - 1;
	results[3].x = field.x - 1;
	results[3].y = field.y;
}
void delFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	if(field.x <0 || field.y<0){
		printf("Del flag: Negative numbers!\n");
		exit(EXIT_FAILURE);
	}
	world[field.x][field.y] = world[field.x][field.y] & ~action;
}
void copyFlags(int fromWorld[NOCOLS][NOROWS], int toWorld[NOCOLS][NOROWS], struct coord field){
	if(field.x <0 || field.y<0){
		printf("Copy flag: Negative numbers!\n");
		exit(EXIT_FAILURE);
	}
	toWorld[field.x][field.y] |= fromWorld[field.x][field.y];
}
int testFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	if(((world[field.x][field.y] & action) == action)){
		return 1;
	} else {
		return 0;
	}
}
void setFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	if(field.x <0 || field.y<0){
		printf("setFlag: Negative numbers!\n");
		exit(EXIT_FAILURE);
	}
	if(action == VISITED){
		delFlag(world, field, SAFE);
		delFlag(world, field, CAVESUS);
		delFlag(world, field, GLOWSUS);
		delFlag(world, field, WUMPUSSUS);
	}
	world[field.x][field.y] = world[field.x][field.y] | action;
}
int randInt(int min, int max, int *except, int noExcept){
	int result, i;
	result = rand() % (max - min + 1) + min;
	if(except != NULL){
		for(i=0;i<noExcept;i++){
			if(result == except[i]){
				result = randInt(min,max,except,noExcept);
				break;
			}
		}
	}
	return result;
}
void generateRealWorld(int realWorld[NOCOLS][NOROWS]){
	int k, z, w;			
	struct coord xy;
	struct coord cavePos[NOCAVES];
	struct coord wumpusPos[NOWUMPUS]; 
	struct coord glowPos[NOGLOW];		
	struct coord neighbors[4];
	printf("\nEnter the number of caves: ");	
	scanf("%d", &NOCAVES);	
	for(k=0;k<NOWUMPUS;k++)
	{
		   printf("\n Enter the x (0 to 3) coordinate of Wumpus: ");
		   scanf("%d",&(wumpusPos[k].x));
		   printf("\n Enter the y (0 to 3) coordinate of Wumpus: ");
		   scanf("%d",&(wumpusPos[k].y));
	}	
	for(k=0;k<NOCAVES;k++)
	{
		   printf("\n Enter the x (0 to 3) coordinate of Cave %d: ", k+1);
		   scanf("%d",&(cavePos[k].x));
		   printf("\n Enter the y (0 to 3) coordinate of Cave %d: ",k+1);
		   scanf("%d",&(cavePos[k].y));
	}	
	for(k=0;k<NOGLOW;k++)
	{
		   printf("\n Enter the x (0 to 3) coordinate of gold: ");
		   scanf("%d",&(glowPos[k].x));
		   printf("\n Enter the y (0 to 3) coordinate of gold: ");
		   scanf("%d",&(glowPos[k].y));
	}
	for(xy.x=0; xy.x<NOROWS; xy.x++){
		for(xy.y=0; xy.y<NOCOLS; xy.y++){
			for(k=0; k< NOCAVES; k++){
				if(xy.x == cavePos[k].x && xy.y == cavePos[k].y){
					setFlag(realWorld,cavePos[k], CAVE);
					neighborFieldsCoords(xy, neighbors);
					for(z=0; z<4; z++){
						if	(
								((neighbors[z].y < NOROWS) && (z == 0))
								||
								((neighbors[z].x < NOCOLS) && (z == 1))
								||
								((neighbors[z].y >= 0) && (z == 2))
								||
								((neighbors[z].x >= 0) && (z == 3))
							){
							setFlag(realWorld, neighbors[z], BREEZE);
						}
					}
				}
			}
			for(k=0; k< NOWUMPUS; k++){
				if(xy.x == wumpusPos[k].x && xy.y == wumpusPos[k].y){
					setFlag(realWorld, wumpusPos[k], WUMPUS);
					neighborFieldsCoords(xy, neighbors);
					for(z=0; z<4; z++){
						if	(
								((neighbors[z].y < NOROWS) && (z == 0))
								||
								((neighbors[z].x < NOCOLS) && (z == 1))
								||
								((neighbors[z].y >= 0) && (z == 2))
								||
								((neighbors[z].x >= 0) && (z == 3))
							){
							setFlag(realWorld, neighbors[z], STENCH);
						}
					}
				}
			}
			for(k=0; k< NOGLOW; k++){
				if(xy.x == glowPos[k].x && xy.y == glowPos[k].y){
					setFlag(realWorld, glowPos[k], GLOW);
					neighborFieldsCoords(xy, neighbors);
					for(z=0; z<4; z++){
						if	(
								((neighbors[z].y < NOROWS) && (z == 0))
								||
								((neighbors[z].x < NOCOLS) && (z == 1))
								||
								((neighbors[z].y >= 0) && (z == 2))
								||
								((neighbors[z].x >= 0) && (z == 3))
							){
							setFlag(realWorld, neighbors[z], GLOWING);
						}
					}
				}
			}
		}
	}
}
void evaluateNeighbors(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int z;
	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	for(z=0; z<4; z++){
		if	(	(
					((neighbors[z].y < NOROWS) && (z == 0))
					||
					((neighbors[z].x < NOCOLS) && (z == 1))
					||
					((neighbors[z].y >= 0) && (z == 2))
					||
					((neighbors[z].x >= 0) && (z == 3))
				)
				&&
				(testFlag(agentWorld, neighbors[z], SAFE) == 0)
				&&
				(testFlag(agentWorld, neighbors[z], VISITED) == 0)
			){
			if(testFlag(agentWorld, field, BREEZE)){
				if(testFlag(agentWorld, neighbors[z], CAVESUS)){
					setFlag(agentWorld, neighbors[z], CAVE);
				}else{
					setFlag(agentWorld, neighbors[z], CAVESUS);
				}
			}

			if(testFlag(agentWorld, field, STENCH)){
				if(testFlag(agentWorld, neighbors[z], WUMPUSSUS)){
					setFlag(agentWorld, neighbors[z], WUMPUS); 
					printf("\nAttempting to kill Wumpus!\n");				
					struct coord send=neighbors[z];
					if(testFlag(realWorld,send,WUMPUS)&& arrow==1)
					{
						arrow=0;
						performance=performance-10;
						printf("\nArrow shot at Wumpus. SCREAM percieved!!! WUMPUS KILLED\n");
						delFlag(agentWorld,send,WUMPUS);
						delFlag(agentWorld,send,WUMPUSSUS);
						delFlag(agentWorld,send,STENCH);
						copyFlags(agentWorld,realWorld,send); return;
					}
					else if(arrow==1)
					{
						 arrow=0;
						 performance=performance-10;
						 printf("\nArrow wasted");
						 return;
					}
					else 
					{
						printf("\nNo arrows left. Beware of Wumpus!\n");
					        return;
					}
					    
				}else{
					setFlag(agentWorld, neighbors[z], WUMPUSSUS);
				}
			}
			if(testFlag(agentWorld, field, GLOWING)){
				if(testFlag(agentWorld, neighbors[z], GLOWSUS)){
					setFlag(agentWorld, neighbors[z], GLOW);
				}else{
					setFlag(agentWorld, neighbors[z], GLOWSUS);
				}
			}
			if((testFlag(agentWorld, field, BREEZE) == 0) &&(testFlag(agentWorld, field, STENCH) == 0))
			{
				setFlag(agentWorld, neighbors[z], SAFE);
				delFlag(agentWorld, neighbors[z], CAVESUS);
				delFlag(agentWorld, neighbors[z], WUMPUSSUS);
				delFlag(agentWorld, neighbors[z], CAVE);
				delFlag(agentWorld, neighbors[z], WUMPUS);
			}

		}
	}
}
void moveOneField(int agentWorld[NOCOLS][NOROWS], int direction, struct coord srcField ){
	struct coord temp;
	agentWorld[srcField.x][srcField.y] = agentWorld[srcField.x][srcField.y] & ~CURRENT;
	switch(direction){
		case 0:
			temp.x = srcField.x;
			temp.y = srcField.y+1;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO UP!\n");
			performance=performance-2;
			break;
		case 1:
			temp.x = srcField.x+1;
			temp.y = srcField.y;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO RIGHT!\n");
			performance=performance-2;
			break;
		case 2:
			temp.x = srcField.x;
			temp.y = srcField.y-1;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO DOWN!\n");
			performance=performance-2;
			break;
		case 3:
			temp.x = srcField.x-1;
			temp.y = srcField.y;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO LEFT!\n");
			performance=performance-2;
			break;
	}
}
double calcDistance(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField){
	double distance;
	distance = sqrt((double)pow((double)destField.x-srcField.x,2)+(double)pow((double)destField.y-srcField.y,2));
	return distance;
}
int calcDirrection(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField){
	int z, i, shortestGoodNeighbor;
	double shortestDist;
	struct coord neighbors[4];
	int goodNeighbors_i;				
	int goodNeighbors_size;
	int *goodNeighbors = NULL;			
	int *goodNeighbors_temp = NULL;
	double *distances = NULL;			
	double *distances_temp = NULL;
	neighborFieldsCoords(srcField, neighbors);
	goodNeighbors_i = 0;
	goodNeighbors_size = 0;
	for(z=0; z<4; z++){
		if	(	(
				((neighbors[z].y < NOROWS) && (z == 0))
				||
				((neighbors[z].x < NOCOLS) && (z == 1))
				||
				((neighbors[z].y >= 0) && (z == 2))
				||
				((neighbors[z].x >= 0) && (z == 3))
				)
				&&
				(testFlag(agentWorld, neighbors[z], VISITED))
			){
			if(goodNeighbors_i == 0){
				goodNeighbors_size++;
				if ((goodNeighbors = (int*)calloc(goodNeighbors_size, sizeof(int))) == NULL) {
					fprintf(stderr, "Error allocating memory.\n");
					exit(EXIT_FAILURE);
				}
				if ((distances = (double*)calloc(goodNeighbors_size, sizeof(double))) == NULL) {
					fprintf(stderr, "Error allocating memory.\n");
					exit(EXIT_FAILURE);
				}
			}else if(goodNeighbors_i == goodNeighbors_size){
				goodNeighbors_size++;
				if ((goodNeighbors_temp = (int*)realloc(goodNeighbors, goodNeighbors_size * sizeof(int))) == NULL) {
					fprintf(stderr, "Ran out of memory while expanding except array.\n");
					free(goodNeighbors);
					exit(EXIT_FAILURE);
				}
				if ((distances_temp = (double*)realloc(distances, goodNeighbors_size * sizeof(double))) == NULL) {
					fprintf(stderr, "Ran out of memory while expanding except array.\n");
					free(distances);
					exit(EXIT_FAILURE);
				}
				goodNeighbors = goodNeighbors_temp;
				distances = distances_temp;
			}
			goodNeighbors[goodNeighbors_i] = z;
			distances[goodNeighbors_i] = calcDistance(agentWorld,neighbors[z], destField);
			goodNeighbors_i++;
		}
	}
	if(goodNeighbors != 0){
		shortestGoodNeighbor = goodNeighbors[0];
		shortestDist = distances[0];
		for(i=1;i<goodNeighbors_i;i++){
			if(distances[i] < shortestDist){
				shortestGoodNeighbor = goodNeighbors[i];
				shortestDist = distances[i];
			}
		}
	}
	free(goodNeighbors);
	free(distances);
	return shortestGoodNeighbor;
}
int takeGlowAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int z;
	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	for(z=0; z<4; z++){
		if	(	(
					((neighbors[z].y < NOROWS) && (z == 0))
					||
					((neighbors[z].x < NOCOLS) && (z == 1))
					||
					((neighbors[z].y >= 0) && (z == 2))
					||
					((neighbors[z].x >= 0) && (z == 3))
				)
				&&
				(
					testFlag(agentWorld, neighbors[z], GLOW)
				)
			){
				printf("GLOW could be nearby, lets grab it!\n");
				moveOneField(agentWorld, z, field);
				return(1);
		}
	}
	return(0);
}
int takeSafeAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int random,z;
	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	int except_i;			
	int except_size;
	int *exceptSF = NULL;			
	int *exceptSF_temp = NULL;	
	except_i = 0;
	except_size = 0;
	for(z=0; z<4; z++){
		if	(	(
					((neighbors[z].y > NOROWS-1) && (z == 0))
					||
					((neighbors[z].x > NOCOLS-1) && (z == 1))
					||
					((neighbors[z].y < 0) && (z == 2))
					||
					((neighbors[z].x < 0) && (z == 3))
				)
				||
				(testFlag(agentWorld, neighbors[z], VISITED))
				||
				(testFlag(agentWorld, neighbors[z], SAFE) == 0)
				){
			if(except_i == 0){
				except_size++;
				if ((exceptSF = (int*)calloc(except_size, sizeof(int))) == NULL) {
					fprintf(stderr, "Error allocating memory.\n");
					exit(EXIT_FAILURE);
				}
			}else if(except_i == except_size){
				except_size++;
				if ((exceptSF_temp = (int*)realloc(exceptSF, except_size * sizeof(int))) == NULL) {
					fprintf(stderr, "Ran out of memory while expanding except array.\n");
					free(exceptSF);
					exit(EXIT_FAILURE);
				}
				exceptSF = exceptSF_temp;
			}
			exceptSF[except_i] = z;
			except_i++;
		}
	}
	if (except_i<4){
		random = randInt(0,3,exceptSF,except_i);
		moveOneField(agentWorld, random, field);
		free(exceptSF);
		return(1);
	}else{
		free(exceptSF);
		int x,y,direction;
		struct coord nearestSafeFieldCoord = { -1, -1 };
		for(x=0; x<NOROWS; x++){
			for(y=0; y<NOCOLS; y++){
				struct coord testSafeFieldCoord = { x, y };
				if(testFlag(agentWorld, testSafeFieldCoord, SAFE)){
					if((nearestSafeFieldCoord.x == -1) && (nearestSafeFieldCoord.y == -1)){
						nearestSafeFieldCoord = testSafeFieldCoord;
					}else{
						if(calcDistance(agentWorld,field,testSafeFieldCoord) < calcDistance(agentWorld, field,nearestSafeFieldCoord)){
							nearestSafeFieldCoord = testSafeFieldCoord;
						}
					}
				}
			}
		}
		if((nearestSafeFieldCoord.x == -1) && (nearestSafeFieldCoord.y == -1)){
			return(0);
		} else{
			direction = calcDirrection(agentWorld,field,nearestSafeFieldCoord);
			moveOneField(agentWorld,direction,field);
			return(1);
		}

	}
}
int killWumpus(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int z,k;
   	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	for(z=0; z<4; z++)
	{
		if	(	(
					((neighbors[z].y < NOROWS) && (z == 0))
					||
					((neighbors[z].x < NOCOLS) && (z == 1))
					||
					((neighbors[z].y >= 0) && (z == 2))
					||
					((neighbors[z].x >= 0) && (z == 3))
				)
				&&
				(testFlag(agentWorld, neighbors[z], WUMPUSSUS))
				){
					struct coord send=neighbors[z];
				        if(testFlag(realWorld,send,WUMPUS)&& (arrow==1))
				        {
				        	arrow=0;
					        performance=performance-10;
					        printf("\nArrow thrown at Wumpus. SCREAM percieved!!! WUMPUS KILLED\n");
					        delFlag(agentWorld,send,WUMPUS);
					        delFlag(agentWorld,send,WUMPUSSUS);
					        delFlag(agentWorld,send,STENCH);
					        copyFlags(agentWorld,realWorld,send);
					        if((testFlag(agentWorld, field, BREEZE) == 0) ){
							setFlag(agentWorld, neighbors[z], SAFE);
							delFlag(agentWorld, neighbors[z], CAVESUS);
							delFlag(agentWorld, neighbors[z], WUMPUSSUS);
							delFlag(agentWorld, neighbors[z], CAVE);
							delFlag(agentWorld, neighbors[z], WUMPUS);
						}
						return 1;
					}
				        else if(!testFlag(realWorld,send,WUMPUS)&& (arrow==1))
				        {
						 arrow=0;
						 performance=performance-10;
						 delFlag(agentWorld,send,WUMPUSSUS);
						 delFlag(agentWorld,send,WUMPUS);
						 printf("\nArrow wasted while trying to kill wumpus!\n");
						 if((testFlag(agentWorld, field, BREEZE) == 0) ){
							setFlag(agentWorld, neighbors[z], SAFE);
							delFlag(agentWorld, neighbors[z], CAVESUS);
							delFlag(agentWorld, neighbors[z], WUMPUSSUS);
							delFlag(agentWorld, neighbors[z], CAVE);
							delFlag(agentWorld, neighbors[z], WUMPUS);
						}
				      		return 1;
				       }
				       else 
				       {
					       printf("\nNo arrows left. Beware of Wumpus!\n");
					       return 0;
				       }
			       }
	}
}
int takeRollTheDiceAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int random,z;
	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	int except_i;
	int except_size;
	int *exceptRND = NULL;
	int *exceptRND_temp = NULL;
	except_i = 0;
	except_size = 0;
	for(z=0; z<4; z++){
		if	(	(
					((neighbors[z].y > NOROWS-1) && (z == 0))
					||
					((neighbors[z].x > NOCOLS-1) && (z == 1))
					||
					((neighbors[z].y < 0) && (z == 2))
					||
					((neighbors[z].x < 0) && (z == 3))
				)
				||
				(testFlag(agentWorld, neighbors[z], VISITED))
				||
				(testFlag(agentWorld, neighbors[z], CAVE))
				||
				(testFlag(agentWorld, neighbors[z], WUMPUS))
				){
			if(except_i == 0){
				except_size++;
				if ((exceptRND = (int*)calloc(except_size, sizeof(int))) == NULL) {
					fprintf(stderr, "Error allocating memory.\n");
					exit(EXIT_FAILURE);
				}
			}else if(except_i == except_size){
				except_size++;
				if ((exceptRND_temp = (int*)realloc(exceptRND, except_size * sizeof(int))) == NULL) {
					fprintf(stderr, "Ran out of memory while expanding except array.\n");
					free(exceptRND);
					exit(EXIT_FAILURE);
				}
				exceptRND = exceptRND_temp;
			}
			exceptRND[except_i] = z;
			except_i++;
		}
	}
	if (except_i<4){
		random = randInt(0,3,exceptRND,except_i);

		printf("Roll the dice!\n");
		moveOneField(agentWorld, random, field);
		free(exceptRND);
		return(1);
	} else{
		free(exceptRND);
		return(0);
	}
}
int takeSuicideAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int random,z;
	struct coord neighbors[4];
	neighborFieldsCoords(field, neighbors);
	int except_i;
	int except_size;
	int *exceptRND = NULL;
	int *exceptRND_temp = NULL;
	except_i = 0;
	except_size = 0;
	for(z=0; z<4; z++){
		if	(	(
					((neighbors[z].y > NOROWS-1) && (z == 0))
					||
					((neighbors[z].x > NOCOLS-1) && (z == 1))
					||
					((neighbors[z].y < 0) && (z == 2))
					||
					((neighbors[z].x < 0) && (z == 3))
				)
				||
				(testFlag(agentWorld, neighbors[z], VISITED))
				){
			if(except_i == 0){
				except_size++;
				if ((exceptRND = (int*)calloc(except_size, sizeof(int))) == NULL) {
					fprintf(stderr, "Error allocating memory.\n");
					exit(EXIT_FAILURE);
				}
			}else if(except_i == except_size){
				except_size++;
				if ((exceptRND_temp = (int*)realloc(exceptRND, except_size * sizeof(int))) == NULL) {
					fprintf(stderr, "Ran out of memory while expanding except array.\n");
					free(exceptRND);
					exit(EXIT_FAILURE);
				}
				exceptRND = exceptRND_temp;
			}
			exceptRND[except_i] = z;
			except_i++;
		}
	}
	if (except_i<4){
		random = randInt(0,3,exceptRND,except_i);
		printf("Commit suicide!\n");
		moveOneField(agentWorld, random, field);
		free(exceptRND);
		return(1);
	} else{
		free(exceptRND);
		return(0);
	}
}
void myPause (void){
  printf ( "Press any key to continue . . ." );
  fflush ( stdout );
  getchar();
}	

