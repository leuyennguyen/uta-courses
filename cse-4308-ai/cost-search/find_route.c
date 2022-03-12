/*
	Le Uyen Nguyen
	100 171 8086
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Each city is represented by a node. Each node contains:
// - state: city's name (assume that each name is 20 character long)
// - priority : actual cost from its parent
// - estTotalCost: estimate total cost from the start state
// - depth : the depth of the current node
// - parent : the address of its parent
// - next: the adddress of the next node/city
typedef struct node {
	char state[20];
	float priority;
	float estTotalCost;
	int depth;
	struct node *parent;
	struct node *next;
} Node;

// The array contains all visited node: CLOSED[]
typedef struct array {
	char state[20];
	struct array *next;
} Array;

// Pointer to 2D array contains distances between cities,
// or edges of the graph. The first two columns are the 
// two cities, and the last column is their distance apart.
char *Func[][3] = {};

// Double pointer to the heuristic's list. First column is
// the city's name, the second column is its straight-line
// distance to the goal state.
char **Heuristics = NULL;


/*	Function Prototype	*/

void AddToArray(Array **array, char state[20]);
int NotInClosed(Array *closed, char state[20]);
Node *newNode(char state[20], float priority, int depth);
int CountNumLine(FILE *FileHandle);
void ReadInputFile(FILE *FileHandle, int numLine);
void UninformedUCS(Node **Head, Node **Tail, int *nodeGenerated, int numFunc);
float ExtractHeuristic(int numHeu, char state[20]);
void InformedUCS(Node **Head, Node **Tail, int *nodeGenerated, int numFunc, int numHeu);
Node *GraphSearch(char start[20], char goal[20], int *nodePopped, int *nodeExpanded, int *nodeGenerated, int numLine, int numHeu, float estCost);
void printRoute(Node *Head);
FILE *OpenFile(char filename[20]);

// AddToArray() adds the city that is already visited to
// CLOSED[] according the the order of expanded node.
void AddToArray(Array **array, char state[20]) {
	Array *newArray = (Array*)malloc(sizeof(Array));
	strcpy(newArray->state, state);
	newArray->next = NULL;

	if (*array == NULL) {
		*array = newArray;
	}
	else {
		Array *temp = *array;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newArray;
	}
}

// NotInClosed() takes CLOSED[] and a city's name as parameters.
// The function will search for the city in CLOSED[]. If found,
// the city is already visited => return 0; else, 1.
int NotInClosed(Array *closed, char state[20]) {
	Array *temp = closed;
	while (temp != NULL) {
		if (!strcmp(temp->state, state)) {
			return 0;
		}
		temp = temp->next;
	}
	return 1;
}

// newNode() creates and returns the new node with state as city's
// name, priority as distance from its parents, and its node's depth.
Node *newNode(char state[20], float priority, int depth) {
	Node *newNode = (Node*)malloc(sizeof(Node));
	strcpy(newNode->state, state);
	newNode->priority = priority;
	newNode->estTotalCost = 0;
	newNode->depth = depth;
	newNode->parent = NULL;
	newNode->next = NULL;

	return newNode;
}

// CountNumLine() returns the number of lines in the given file.
int CountNumLine(FILE *FileHandle) {
	int numLine = 0;
	if (FileHandle != NULL) {
		int c;
		for (c = getc(FileHandle) ; c != EOF ; c = getc(FileHandle)) {
			// If the newline '\n' is read, increment the line number
			if (c == '\n') {
				numLine++;
			}
		}
	}
	// Move the file pointer to the beginning to read the file later.
	rewind(FileHandle);
	return numLine;
}

// ReadInputFile() only reads the input file, where each line has
// three and only three separate words, to Func[][3]. Each column
// in Func[][] corresponds to each word in the input file. The
// first two words are two cities' names. The last word is the
// floating-point number showing the distance between two cities.
void ReadInputFile(FILE *FileHandle, int numLine) {
	int i = 0;
	char Lines[100] = {};
	char *token = NULL;
	while (fgets(Lines, sizeof(Lines)-1 , FileHandle) && i < numLine) {
		token = strtok(Lines, " ");
		Func[i][0] = malloc(strlen(token)*sizeof(char)+1);
		strcpy(Func[i][0], token);

		token = strtok(NULL, " ");
		Func[i][1] = malloc(strlen(token)*sizeof(char)+1);
		strcpy(Func[i][1], token);

		token = strtok(NULL, "\n");
		Func[i][2] = malloc(strlen(token)*sizeof(char)+1);
		strcpy(Func[i][2], token);
		
		Lines[strlen(Lines)-1] = '\0';
		
		i++;
	}
}

// TESTING ONLY: print nodes in the FRINGE
void printFRINGE(Node *Head) {
    printf("=====================\n");
    Node *temp = Head;
    int i = 0;
    while (temp != NULL) {
        printf("Node %d. %s g(n) = %f | f(n) = %f at %p\n", 
				i++, temp->state, temp->priority, temp->estTotalCost, temp);
        temp = temp->next;
    }
	printf("=====================\n");
}

// UninformedUCS() performs the uninformed uniform cost search 
// to expand the first node in the PriorityQueue FRINGE.
// - Head and Tail represent the PriorityQueue FRINGE
// - nodeGenerated++ for every successor
// - numFunc: the # of rows of Func[][], or the # of graph's edges
void UninformedUCS(Node **Head, Node **Tail, int *nodeGenerated, int numFunc) {
    int i = 0;
	
	// string currState, priority, and depth will keep a copy
	// of the expanded node's information. In case the successor
	// becomes the first node, this guarantes the function only
	// generates successors of the old Head, not the new Head.
    char currState[20] = {};
    strcpy(currState, (*Head)->state);
    int priority = (*Head)->priority;
    int depth = (*Head)->depth;

	// The function reads through the list of graph's edges(Func[][]).
	// If the city's name is found in one column of Func[][], the other
	// column will have its successor as well as distance apart.
	while (i < numFunc) {
	    Node *successor = NULL;
		
		// If the city is not found in both columns of the ith row,
		// increment i and continue to read the next row.
	    if (strcmp(Func[i][0], currState) && strcmp(Func[i][1], currState)) {
	        i++;
	        continue;
	    }
	    else {
			// If the city is found, the other column has its successor
			// NewNode() creates and returns the node for each successor
	        if (!strcmp(Func[i][0], currState)) {
	            successor = newNode(Func[i][1], priority + atof(Func[i][2]), depth++);
	        }
	        else if (!strcmp(Func[i][1], currState)) {
	            successor = newNode(Func[i][0], priority + atof(Func[i][2]), depth++);
	        }
	        successor->parent = *Head;
			// for each generated successor, the # of node generated ++
	        (*nodeGenerated)++; 
			
			// If FRINGE is empty, the successor becomes FRINGE's head
			if ((*Head) == NULL) {
			    (*Head) = (*Tail) = successor;
			}
			else {
				// If successor has lower cost path (higher priority) than
				// the first node in FRINGE, it becomes the FRINGE's head.
			    if (successor->priority < (*Head)->priority) {
			        successor->next = (*Head);
			        (*Head) = successor;
			    }
			    else {
					// Else, the function traverses through the PriorityQueue
					// to find the appropriate slot for the successor
			        Node *prevTemp = (*Head);
			        Node *temp = (*Head)->next;
					
			        while (temp != NULL && temp->priority < successor->priority) {
			            prevTemp = temp;
			            temp = temp->next;
			        }
					// If successor has the highest cost path (lowest priority)
					// than any other node in the FRINGE, it'll the last node
					// in FRINGE, where FRINGE's Tail points to
			        if (temp == NULL) {
			            prevTemp->next = successor;
			            (*Tail) = successor;
			        }
			        else {
						// Else, it is inserted between two existing nodes in FRINGE
			            successor->next = temp;
			            prevTemp->next = successor;
			        }
			    }
			}
	    }
	    i++; // Read the next line
	}
}

// ExtractHeuristic() returns the heuristic value (straight-line
// distance) of the city's name provided to the function.
float ExtractHeuristic(int numHeu, char state[20]) {
    int i = 0;
    int heuristics = -9999;
    char *token = NULL;
    for (i = 0 ; i < numHeu ; i++) {
		// The function reads each row in **Heuristics, and put in		
		// *str in order not to modify info in **Heuristics.
        char *str = strdup(Heuristics[i]);
		
		// strtok() that row to find the city's name matching the 
		// provided info. If found, strtok() again to extract its 
		// heuristic value and return. Else, return -9999.
        token = strtok(str, " ");
        if (!strcmp(token, state)) {
            token = strtok(NULL, "\n");
            heuristics = atof(token);
        }
        free(str);
    }
    return heuristics;
}

// InformedUCS() performs the informed uniform cost search 
// to expand the first node in the PriorityQueue FRINGE.
// - Head and Tail represent the PriorityQueue FRINGE
// - nodeGenerated++ for every successor
// - numFunc: the # of rows of Func[][], or the # of graph's edges
// - numHeu: the # of rows in **Heuristics, or the # of cities
void InformedUCS(Node **Head, Node **Tail, int *nodeGenerated, int numFunc, int numHeu) {
    int i = 0;
	
	// string currState, priority, and depth will keep a copy
	// of the expanded node's information. In case the successor
	// becomes the first node, this guarantes the function only
	// generates successors of the old Head, not the new Head.
    char currState[20] = {};
    strcpy(currState, (*Head)->state);
    int priority = (*Head)->priority;
    int depth = (*Head)->depth;
    
	// The function reads through the list of graph's edges(Func[][]).
	// If the city's name is found in one column of Func[][], the other
	// column will have its successor as well as distance apart.
	while (i < numFunc) {
	    Node *successor = NULL;
		
		// If the city is not found in both columns of the ith row,
		// increment i and continue to read the next row.
	    if (strcmp(Func[i][0], currState) && strcmp(Func[i][1], currState)) {
	        i++;
	        continue;
	    }
	    else {
			// If the city is found, the other column has its successor
			// NewNode() creates and returns the node for each successor
	        if (!strcmp(Func[i][0], currState)) {
	            successor = newNode(Func[i][1], priority + atof(Func[i][2]), depth++);
	        }
	        else if (!strcmp(Func[i][1], currState)) {
	            successor = newNode(Func[i][0], priority + atof(Func[i][2]), depth++);
	        }
	        successor->parent = *Head;
			
			// estCost stores the return value of ExtractHeuristic(),
			// which is the heuristic value of that successor
	        float estCost = ExtractHeuristic(numHeu, successor->state);
			// The estimated total cost path = (the distance between 
			// the successor and its parent) + (its straight-line
			// distance to the goal)
	        successor->estTotalCost = successor->priority + estCost;

			// for each generated successor, the # of node generated ++
	        (*nodeGenerated)++;
			
			// If FRINGE is empty, the successor becomes FRINGE's head
			if ((*Head) == NULL) {
			    (*Head) = (*Tail) = successor;
			}
			else {
				// InformedUCS() enqueues the node based on its estTotalCost
				// If successor has lower total cost path (higher priority) 
				// than the first node in FRINGE, it becomes the FRINGE's head.
			    if (successor->estTotalCost < (*Head)->estTotalCost) {
			        successor->next = (*Head);
			        (*Head) = successor;
			    }
			    else {
					// Else, the function traverses through the PriorityQueue
					// to find the appropriate slot for the successor
			        Node *prevTemp = (*Head);
			        Node *temp = (*Head)->next;
			        while (temp != NULL && temp->estTotalCost < successor->estTotalCost) {
			            prevTemp = temp;
			            temp = temp->next;
			        }
					// If successor has the highest total cost path (lowest priority)
					// than any other node in the FRINGE, it'll the last node
					// in FRINGE, where FRINGE's Tail points to
			        if (temp == NULL) {
			            prevTemp->next = successor;
			            (*Tail) = successor;
			        }
			        else {
						// Else, it is inserted between two existing nodes in FRINGE
			            successor->next = temp;
			            prevTemp->next = successor;
			        }
			    }
			}
	    }
	    i++; // Read the next line
	}
}

// GraphSearch() performs graph search algorithm
Node *GraphSearch(char start[20], char goal[20], int *nodePopped, int *nodeExpanded, int *nodeGenerated, int numLine, int numHeu, float estCost) {
	// CLOSED[] contains the list of visited node
	Array *CLOSED = NULL;
	
	// FRINGE is the PriorityQueue containing all generated nodes.
	// Head points to the first node in FRINGE
	// Tail points to the last node in FRINGE
	Node *Head = NULL, *Tail = NULL;

	// The first node that is firstly generated is the start state
	Node *startNode = newNode(start, 0, 0);
	startNode->estTotalCost = estCost;
	Head = Tail = startNode;
	
	(*nodeGenerated)++; // = 1 = the start state
	
	// prevHead is a copy of the previous head,
	// just in case uniform cost search finds
	// the new node with the highest priority
	// making the FRINGE's head changes
	Node *prevHead = NULL;
	
	// While FRINGE is not empty
	while (Head != NULL) {
		// If FRINGE is empty and the goal is not found, there is
		// no path from start to goal state => return NULL
		if (Head == NULL) {
			return NULL;
		}
		
		(*nodePopped)++; // for every node popped out of the FRINGE
		
		// If current FRINGE's Head (the node that is popped) is the
		// goal state => the path found
		if (!strcmp(Head->state, goal)) {
			return Head;
		}
		
		prevHead = Head; // Keep the copy of the popped node
		
		// If the popped node is not visited yet, add it to CLOSED[]
		// and perform uniform cost search.
		// - If (estCost == 0), heuristic value is not used (no hFile 
		// provided), perform uninformed search.
		// - If (estCost != 0), there is heuristic value used (hFile
		// is provided), perform informed search.
		if (NotInClosed(CLOSED, Head->state)) {
			AddToArray(&CLOSED, Head->state);
			if (estCost == 0) {
			    UninformedUCS(&Head, &Tail, nodeGenerated, numLine);
			}
			else {
			    InformedUCS(&Head, &Tail, nodeGenerated, numLine, numHeu);
			}
			(*nodeExpanded)++; // the # of nodes popped out of FRINGE
		}
		
		/* The popped node is now removed from the PriorityQueue FRINGE,
		   but it should not be free() since it may contain useful info
		   for later use.
		   If the FRINGE's head changes after UCS (the current Head != 
		   the previous Head), remove prevHead by disconnected from
		   other nodes in FRINGE. Else, FRINGE's head pointer points
		   to the next node in line.
		*/
		Node *temp = Head, *prev = NULL; 
		if (prevHead != temp) {
		    while (temp->next != NULL && temp->next == prevHead) {
		        prev = temp;
		        temp = temp->next;
		    }
		    if (temp == prevHead) {
		        prev->next = temp->next;
		    }
		}
		else {
		    Node *newHead = Head->next;
		    Head = newHead;
		}
	}
	if (Head == NULL) {
			return NULL;
	}
}

// printRoute() will recursively get to the start state,
// print from the start state through its successor to
// get to the goal state.
void printRoute(Node *Head) {
    if (Head == NULL) {
        return;
    }
    printRoute(Head->parent);
    if (Head->parent->state != NULL) {
        printf("%s to %s, %0.1f\n", Head->parent->state, Head->state, Head->priority - Head->parent->priority);
    }
}

FILE *OpenFile(char filename[20]) {
	FILE *FileHandle = NULL;
	do {
		FileHandle = fopen(filename, "r");
		if (filename == NULL) {
			printf("%s not found.\n", filename);
			printf("Enter file name at prompt: ");
			scanf("%s", filename);
		}
	}
	while (FileHandle == NULL);
	return FileHandle;
}

int main(int argc, char *argv[]) {
	char inputFile[20] = {}; // contains the input file name from argv[1]
	char startState[20] = {}; // contains the start state argv[2]
	char goalState[20] = {}; // contains the goal state from argv[3]
	char hFile[20] = {}; // contains the heuristic file from argv[4], if provided
	
	int node_popped = 0;
	int node_expaned = 0;
	int node_generated = 0;
	int numFunc = 0; // the # of edges in the graph, or the # of lines in the input file
	int numHeu = 0; // the # of cities, or the # of lines in the heuristic file
	
	FILE *inputFileHandle = NULL;
	FILE *hFileHandle = NULL;
	
	Node *result = NULL; // the result node. If == NULL, route not found; else, found.
	float estCost = 0;
	
	// Copy the command line input to appropriate variables
	// If any of them is missing, the program cannot run.
	if (argc > 3) {
		strcpy(inputFile, argv[1]);
		strcpy(startState, argv[2]);
		strcpy(goalState, argv[3]);
	}
	else {
		printf("There is not enough information to search.\n");
		return 0;
	}

	inputFileHandle = OpenFile(inputFile);	
	numFunc = CountNumLine(inputFileHandle);
	ReadInputFile(inputFileHandle, numFunc);
	fclose(inputFileHandle);

	// If the heuristic file is provided, perform informed uniform cost search; else, uninformed UCS.
	if (argc == 5) {
		strcpy(hFile, argv[4]);
		hFileHandle = OpenFile(hFile);
		numHeu = CountNumLine(hFileHandle);
		
		// ReadInputFile() only reads the input file, where each line has
// three and only three separate words, to Func[][3]. Each column
// in Func[][] corresponds to each word in the input file. The
// first two words are two cities' names. The last word is the
// floating-point number showing the distance between two cities.

		// Read the heuristic file line-by-line to **Heuristics
		Heuristics = malloc(numHeu*sizeof(Heuristics));
	    char Lines[100] = {};
	    int i = 0;
	    for (i = 0 ; i < numHeu ; i++) {
	        fgets(Lines, sizeof(Lines)-1 , hFileHandle);
	        Heuristics[i] = strdup(Lines);
	    }
		fclose(hFileHandle);
		
		estCost = ExtractHeuristic(numHeu, startState);
	}
		
	result = GraphSearch(startState, goalState, &node_popped, &node_expaned, &node_generated, numFunc, numHeu, estCost);
	
	printf("Nodes Popped: %d\n", node_popped);
	printf("Nodes Expaned: %d\n", node_expaned);
	printf("Nodes Generated: %d\n", node_generated);

	if (result == NULL) {
		printf("Distance: Infinity\n");
		printf("Route: \nNone\n");
	}
	else {
		printf("Distance: %0.1f\n", result->priority);
		printf("Route: \n");
		printRoute(result);
	}
	
	return 0;
}
