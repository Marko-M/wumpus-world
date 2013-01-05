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

#include <wumpus-world.h>

int main(int argc, char *argv[]) {
	int realWorld[NOCOLS][NOROWS];	// Array of ints holding real world map
	int agentWorld[NOCOLS][NOROWS];	// Array of ints holding agents world map
	int x, y;						// Counters
	struct coord currentCoord;		// Current position coordinates
	struct coord start = {0, 0};	// Starting position

	// Initialize random number generator seed
	srand((unsigned int)time(0));

	// Initialize both worlds to zero.
	for(y=0; y<NOROWS; y++){
		for(x=0; x<NOCOLS; x++){
			realWorld[x][y] = 0;
			agentWorld[x][y] = 0;
		}
	}

	// Generate realWorld.
	generateRealWorld(realWorld);

	// Generate agentWorld
	setFlag(agentWorld, start, CURRENT);	// Initialy player is at (0,0)

	// Set current coordinates
	currentCoord = start;

	// Display real world
	puts("R E A L  W O R L D");
	displayWorldAll(realWorld);

	int count = 0;
	while(1){
		printf("\n============================================================\n");
		count++;

		// Update current coordinates
		currentCoord = getCurrentCoord(agentWorld);

		// If current field isn't visited, process sensor data
		if(testFlag(agentWorld, currentCoord, VISITED) == 0){

			// Read sensors and update agents map of the world
			copyFlags(realWorld,agentWorld,currentCoord);

			// Check is cave or wumpus here, if it is then game over, else process sensors
			if(testFlag(agentWorld, currentCoord, CAVE)){
				// Display agent world
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);

				puts("\nWent down the cave. GAME OVER!");
				break;
			} else if(testFlag(agentWorld,currentCoord,WUMPUS)){
				// Display agent world
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);

				puts("\nWumpus had a good meal. GAME OVER!");
				break;
			} else if(testFlag(agentWorld,currentCoord,GLOW)){
				// Display agent world
				printf("\nA G E N T  W O R L D (Step %d)\n",count);
				printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
				displayWorldAll(agentWorld);

				puts("\nCongratulate! You have found gold! Nice!");
				break;
			} else{
				// Mark current coordinates as visited and remove safe
				setFlag(agentWorld, currentCoord, VISITED);

				// Evaluate neighbors and set appropriate flags
				evaluateNeighbors(agentWorld,currentCoord);
			}
		}

		// Display agent world
		printf("\nA G E N T  W O R L D (Step %d)\n",count);
		printf("Current position (%d,%d):\n", currentCoord.x, currentCoord.y);
		displayWorldAll(agentWorld);

		if(takeGlowAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeGlowAction method!\n");
		} else if(takeSafeAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeSafeAction method!\n");
		} else if(takeRollTheDiceAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeRollTheDiceAction method!\n");
		} else if(takeSuicideAction(agentWorld, currentCoord)){
			printf("This move is recommended by takeSuicideAction method, situation was hopeless!\n");
		}

		// sleep(2);
		myPause();
	}

	myPause();
	return EXIT_SUCCESS;
}


// Int to to binary string with padding
char* pBinFill(long int x,char *so, char fillChar){
	// fill in array from right to left
	char s[NOFBITS+1];
	int  i=NOFBITS;
	s[i--]=0x00;   // terminate string
	do
	{ // fill in array from right to left
	s[i--]=(x & 1) ? '1':'0';
	x>>=1;  // shift right 1 bit
	} while( x > 0);
	while(i>=0) s[i--]=fillChar;    // fill with fillChar
	sprintf(so,"%s",s);
	return so;
}

// Int to to binary string without padding
char* pBin(long int x,char *so){
	char s[NOFBITS+1];
	int  i=NOFBITS;
	s[i--]=0x00;   // terminate string
	do
	{ // fill in array from right to left
	s[i--]=(x & 1) ? '1':'0';  // determine bit
	x>>=1;  // shift right 1 bit
	} while( x > 0);
	i++;   // point to last valid character
	sprintf(so,"%s",s+i); // stick it in the temp string string
	return so;
}



// return current player position
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

// Display world as matrix of flag vectors in decimal form
void displayWorldDecimal(int world[NOROWS][NOCOLS]){
	int x, y;

	for(y=NOROWS-1; y>=0; y--){
		for(x=0; x<NOCOLS; x++){
		  printf("%d",world[x][y]);
		  printf("\t");

		}
		printf("\n");
	}
}

// Display world as matrix of flag vectors in binary form
void displayWorldBinary(int world[NOROWS][NOCOLS]){
	int x, y;
	char so[NOFBITS+1];

	for(y=NOROWS-1; y>=0; y--){
		for(x=0; x<NOCOLS; x++){
			printf("%s", pBinFill(world[x][y],so,'0'));
			printf("\t");
		}
		printf("\n");
	}
}

void displayWorldAll(int world[NOROWS][NOCOLS]){
	displayWorldDecimal(world);
	printf("\n------------------------------------------------------------\n");
	displayWorldBinary(world);
	printf("\n------------------------------------------------------------\n");
	displayWorldSymbols(world);
	printf("\nLEGEND\nWumpus (W), Cave (C), Glow (G), WumpusSus (w), CaveSus (s), GlowSus (g), Breeze (B), Stench (T), Glowing (L), Visited (V), Safe (S), Current (A)\n\n");
	fflush(stdout);
}

// Display world as matrix of flag vectors using symbols
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

// Get coordinates for given field
void neighborFieldsCoords(struct coord field, struct coord results[4]){
	// One up
	results[0].x = field.x;
	results[0].y = field.y + 1;

	// One right
	results[1].x = field.x + 1;
	results[1].y = field.y;

	// One down
	results[2].x = field.x;
	results[2].y = field.y - 1;

	// One left
	results[3].x = field.x - 1;
	results[3].y = field.y;
}

// Delete flag from field
void delFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	// TODO: REMOVE, testing only
	if(field.x <0 || field.y<0){
		printf("Del flag: Negative numbers!\n");
		exit(EXIT_FAILURE);
	}
	world[field.x][field.y] = world[field.x][field.y] & ~action;
}

// Copy flags for one field from one world to another
void copyFlags(int fromWorld[NOCOLS][NOROWS], int toWorld[NOCOLS][NOROWS], struct coord field){
	// TODO: REMOVE, testing only
	if(field.x <0 || field.y<0){
		printf("Copy flag: Negative numbers!\n");
		exit(EXIT_FAILURE);
	}
	toWorld[field.x][field.y] |= fromWorld[field.x][field.y];
}

// Test field for flags
int testFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	if(((world[field.x][field.y] & action) == action)){
		return 1;
	} else {
		return 0;
	}
}

// Set flag for field
void setFlag(int world[NOCOLS][NOROWS], struct coord field, int action){
	// TODO: REMOVE, testing only
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

/* Generates a random integer between min and max, except
 * integers inside the except vector */
int randInt(int min, int max, int *except, int noExcept){
	int result, i;
	//result = min + rand() / ((RAND_MAX / (max - min + 1)) + 1);
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

/* Generates a random integer pair between min(x,y) and max(x,y) except
 * integer pairs inside the except vector. Result is stored into struct struct coord result */
struct coord randPair(struct coord min, struct coord max, struct coord *except, int noExcept){
	int i;
	struct coord result;
	// x coordinate
	result.x = randInt(min.x,max.x, NULL,0);

	//y coordinate
	result.y = randInt(min.y,max.y, NULL,0);

	if(except != NULL){
		for(i=0;i<noExcept;i++){
			if(result.x == except[i].x && result.y == except[i].y){
				result = randPair(min,max,except,noExcept);
				break;
			}
		}
	}
	return result;
}

// Generate real wumpus world
void generateRealWorld(int realWorld[NOCOLS][NOROWS]){
	int k, z, w;			// Counters.
	struct coord xy;
	struct coord cavePos[NOCAVES];	// Coords of cave positions.
	struct coord wumpusPos[NOWUMPUS]; // Coords of wumpus positions.
	struct coord glowPos[NOGLOW];		// Coords of treasure positions.
	struct coord neighbors[4];		// Stores coords of the up, down, left, right fields

	struct coord min = {1,1};					// Min for random pairs generator
	struct coord max = {NOROWS-1, NOCOLS-1};	// Max for random pairs generator
	struct coord default_except = {0,0};		/* We don't want cave, wumpus or glow on the (0,0) */

	int except_i;		/* Indexing into except array. */
	int except_size;	/* No. of exceptions inside except array. */

	struct coord *except = NULL;		/* Array with exceptions */
	struct coord *except_temp = NULL;	/* Intermediate for validating realloc(). */

	/* We calloc space for the first (0,0) default_except. */
	except_i = 0;
	except_size = 1;
	if ((except = (struct coord*)calloc(except_size, sizeof(struct coord))) == NULL) {
		fprintf(stderr, "Error allocating memory.\n");
		exit(EXIT_FAILURE);
	}

	/* We don't want cave, glow or wumpus on (0,0) cause that's where the player
	 * starts at the beginning. */
	except[except_i] = default_except;

	// Generate caves coordinates.
	k = 0;
	while(k<NOCAVES){
		except_i++;		// This is new except
		if(except_i == except_size){
			except_size += 1;	// New except size including this new except
			if ((except_temp = (struct coord*)realloc(except, except_size * sizeof(struct coord))) == NULL) {
				fprintf(stderr, "Ran out of memory while expanding except array.\n");
				free(except);
				exit(EXIT_FAILURE);
			}
			except = except_temp;	// realloc went OK, inform except of its new size
		}
		/* Lets generate field and store it into cavePos[k]. We omit field already taken
		 * by passing except array to rand pair generator.
		 */
		cavePos[k] = randPair(min,max,except,except_i);

		// Add generated randPair to the list of excepts to mark this field as taken
		except[except_i] = cavePos[k];

		k++;
	}

	// Generate wumpus coordinates.
	k = 0;
	while(k<NOWUMPUS){
		except_i++;
		if(except_i == except_size){
			except_size += 1;
			if ((except_temp = (struct coord*)realloc(except, except_size * sizeof(struct coord))) == NULL) {
				fprintf(stderr, "Ran out of memory while expanding except array.\n");
				free(except);
				exit(EXIT_FAILURE);
			}
			except = except_temp;
		}

		wumpusPos[k] = randPair(min,max,except,except_i);

		except[except_i] = wumpusPos[k];

		k++;
	}

	// Generate glow coordinates
	k = 0;
	while(k<NOGLOW){
		except_i++;
		if(except_i == except_size){
			except_size += 1;
			if ((except_temp = (struct coord*)realloc(except, except_size * sizeof(struct coord))) == NULL) {
				fprintf(stderr, "Ran out of memory while expanding except array.\n");
				free(except);
				exit(EXIT_FAILURE);
			}
			except = except_temp;
		}

		glowPos[k] = randPair(min,max,except,except_i);

		except[except_i] = glowPos[k];

		k++;
	}

	free(except);

	// Initialize realWorld to what is generated
	for(xy.x=0; xy.x<NOROWS; xy.x++){
		for(xy.y=0; xy.y<NOCOLS; xy.y++){

			// We check if cave should be positioned here
			for(k=0; k< NOCAVES; k++){
				if(xy.x == cavePos[k].x && xy.y == cavePos[k].y){
					setFlag(realWorld,cavePos[k], CAVE);

					// Every cave has breeze one field around it
					neighborFieldsCoords(xy, neighbors);

					for(z=0; z<4; z++){
						/* We dont want to put breeze on field outside of the board
						 * limits */
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

			// We check if wumpus should be positioned here
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

			// We check if glow should be positioned here
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

	// Status messages with cave, wumpus and glow positions
	for(w=0; w<NOCAVES; w++){
			printf("%d. cave coords: (%d, %d)\n",w,cavePos[w].x,cavePos[w].y);
	}

	for(w=0; w<NOWUMPUS; w++){
			printf("%d. wumpus coords: (%d, %d)\n",w,wumpusPos[w].x,wumpusPos[w].y);
	}
	for(w=0; w<NOGLOW; w++){
				printf("%d. glow coords: (%d, %d)\n",w,glowPos[w].x,glowPos[w].y);
	}
	printf("\n");
}

// Read sensors and update KB
void evaluateNeighbors(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int z;
	struct coord neighbors[4];		// Neighbors coordinates
	// Get neighbors coordinates
	neighborFieldsCoords(field, neighbors);

	// Iterate all four neighbors to set suspect flags (ignore if neighbor SAFE and VISITED)
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



			/* If current field says breeze, CAVESUS marks all around. If one
			 * of the neighbors already has CAVESUS flag then put cave mark there.*/
			if(testFlag(agentWorld, field, BREEZE)){
				if(testFlag(agentWorld, neighbors[z], CAVESUS)){
					/* Cave must be there cause this this field is already
					 * marked with CAVESUS*/
					setFlag(agentWorld, neighbors[z], CAVE);
				}else{
					setFlag(agentWorld, neighbors[z], CAVESUS);
				}
			}

			if(testFlag(agentWorld, field, STENCH)){
				if(testFlag(agentWorld, neighbors[z], WUMPUSSUS)){
					setFlag(agentWorld, neighbors[z], WUMPUS);
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

			//If this field gives no sensor data, mark surrounding fields as SAFE
			if((testFlag(agentWorld, field, BREEZE) == 0) &&
				(testFlag(agentWorld, field, STENCH) == 0)){
				setFlag(agentWorld, neighbors[z], SAFE);

				// Delete all unnecessary flags
				delFlag(agentWorld, neighbors[z], CAVESUS);
				delFlag(agentWorld, neighbors[z], WUMPUSSUS);
				delFlag(agentWorld, neighbors[z], CAVE);
				delFlag(agentWorld, neighbors[z], WUMPUS);
			}

		}
	}
}

// Move agent one field in required direction
void moveOneField(int agentWorld[NOCOLS][NOROWS], int direction, struct coord srcField ){
	struct coord temp;
	agentWorld[srcField.x][srcField.y] = agentWorld[srcField.x][srcField.y] & ~CURRENT;
	switch(direction){
		// Up
		case 0:
			temp.x = srcField.x;
			temp.y = srcField.y+1;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO UP!\n");
			break;
		// Right
		case 1:
			temp.x = srcField.x+1;
			temp.y = srcField.y;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO RIGHT!\n");
			break;
		// Down
		case 2:
			temp.x = srcField.x;
			temp.y = srcField.y-1;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO DOWN!\n");
			break;
		// Left
		case 3:
			temp.x = srcField.x-1;
			temp.y = srcField.y;
			setFlag(agentWorld, temp, CURRENT);
			printf("Move is: GO LEFT!\n");
			break;
	}
}

// Calculate distance between two fields
double calcDistance(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField){
	double distance;
	distance = sqrt((double)pow((double)destField.x-srcField.x,2)+(double)pow((double)destField.y-srcField.y,2));
	return distance;
}

// Calculate direction to go from srcField to destField
int calcDirrection(int agentWorld[NOCOLS][NOROWS], struct coord srcField, struct coord destField){
	int z, i, shortestGoodNeighbor;
	double shortestDist;
	struct coord neighbors[4];
	int goodNeighbors_i;				/* Indexing into except array. */
	int goodNeighbors_size;			/* No. of exceptions inside except array. */

	int *goodNeighbors = NULL;			/* Array with exceptions */
	int *goodNeighbors_temp = NULL;	/* Intermediate for validating realloc(). */

	double *distances = NULL;			/* Array with exceptions */
	double *distances_temp = NULL;	/* Intermediate for validating realloc(). */

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

// Search for gold and take if it has been found
int takeGlowAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int z;
	struct coord neighbors[4];		// Neighbors coordinates
	// Get neighbors coordinates
	neighborFieldsCoords(field, neighbors);

	// If one of the neighbor fields is GLOW or GLOWSUS, grab it
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

// Go to the nearest safe field if any safe field exists in agent world
int takeSafeAction(int agentWorld[NOCOLS][NOROWS], struct coord field){
	int random,z;
	struct coord neighbors[4];		// Neighbors coordinates

	// Get neighbors coordinates
	neighborFieldsCoords(field, neighbors);

	int except_i;				/* Indexing into except array. */
	int except_size;			/* No. of exceptions inside except array. */

	int *exceptSF = NULL;			/* Array with exceptions */
	int *exceptSF_temp = NULL;	/* Intermediate for validating realloc(). */

	except_i = 0;
	except_size = 0;

	// First we will see are there any neighbor fields that are SAFE
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
		// If there are no neighbor fields SAFE, then we hunt for nearest safe field in the world
		free(exceptSF);
		int x,y,direction;
		struct coord nearestSafeFieldCoord = { -1, -1 };

		/* We go trough agentWorld in a search of a SAFE field nearest to our current position */
		for(x=0; x<NOROWS; x++){
			for(y=0; y<NOCOLS; y++){
				struct coord testSafeFieldCoord = { x, y };
				// Test is SAFE?
				if(testFlag(agentWorld, testSafeFieldCoord, SAFE)){
					// If this is first SAFE field, it is nearest
					if((nearestSafeFieldCoord.x == -1) && (nearestSafeFieldCoord.y == -1)){
						nearestSafeFieldCoord = testSafeFieldCoord;
					}else{
						// It this is not first SAFE field, we check is it nearest one
						if(calcDistance(agentWorld,field,testSafeFieldCoord) < calcDistance(agentWorld, field,nearestSafeFieldCoord)){
							nearestSafeFieldCoord = testSafeFieldCoord;
						}
					}
				}
			}
		}

		// If there are no SAFE fields left, return 0
		if((nearestSafeFieldCoord.x == -1) && (nearestSafeFieldCoord.y == -1)){
			return(0);
		} else{
			// Move in direction of the nearestSafeFieldCoord
			direction = calcDirrection(agentWorld,field,nearestSafeFieldCoord);
			moveOneField(agentWorld,direction,field);
			return(1);
		}

	}
}

// Gamble when there are no safe fields
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

// This is beyond gambling, to avoid being trapped we go anywhere except visited fields even if this suicide
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

// Pause to wait for enter key press
void myPause (void){
  printf ( "Press [Enter] to continue . . ." );
  fflush ( stdout );
  getchar();
}
