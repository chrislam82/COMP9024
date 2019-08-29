/* COMP9024 2019T2
 * Name: Christopher Shu Chun Lam
 * zID: z3460499
 * Last Modified: 11/08/2019
 * Assignment 2 - owl.c
 */ 

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "Graph.h"
# include "Quack.h"
# define UNVISITED -1
# define TRUE 1
# define FALSE 0
# define MAX_DICT 1000 // Maximum num nodes is 1000
# define MAX_WORD 20 // Max word length is 20
# define MAX_LADDERS 99 // Limit on max number of ladders to print

/* findOWLs
 *    Uses DFS from leaf to root nodes to represent longest paths
 *    Stores paths in parentsMatrix
 *    Iterate through parentsMatrix to print all longestPaths
 */
void findOWLs (Graph graph, char ** dictArray, int numWords);

/* ladderRecursion
 *    Recursion through parentsMatrix
 *    Only call child nodes that are part of longest OWL
 */
void ladderRecursion (int ** matrix, char ** dictArray, int * ladderArray, int * hopsLeft, int numWords, int node, int * ladderNum);

/* printLadder
 *    Print longest OWLs as long as < MAX_LADDERS has been printed
 */
void printLadder (int * ladderArray, char ** dictArray, int numWords, int * ladderNum);

/* createArray
 *    Malloc an arrayof ints
 *    Initialise all values to initValue
 */
int * createArray (int numWords, int initValue);

/* createParentsMatrix
 *    Creates a parentsMatrix (array(size numWords) of int arrays(size numWords))
 *    All elements initialised to -1
 *    Returns this parentsMatrix
 */
int ** createParentsMatrix (int numWords);

/* destroyParentsMatrix
 *   free all memory occupied by parentsMatrix
 *   returns NULL
 */ 
int ** destroyParentsMatrix (int ** parentsMatrix, int numWords);

/* destroyDict()
 *   Free all memory stored in array of Words in Dictionary
 *   returns NULL
 */
char ** destroyDict(char ** dictArray, int numWords);

/* differByOne()
 *   Given 2 strings, determine if they differ by one
 *     One letter change OR one letter difference
 *     Return 1 if True, 0 if False
 *     NOTE: Assumes all input is valid so "\0" is valid
 */
int differByOne(char * word1, char * word2);

int main (void) {
    // 1. Read words from stdin until EOF
    char ** dictArray = malloc(sizeof(char *) * MAX_DICT); //Array of words
    if (dictArray == NULL) {
        fprintf (stderr, "Unable to malloc memory\n");
        return EXIT_FAILURE;
    }
    int numWords = 0;
    while (feof(stdin) != 1) {
        char * word = malloc(sizeof(char) * (MAX_WORD + 1)); // +1 for '\0'
        if (word == NULL) {
            fprintf (stderr, "Unable to malloc memory\n");
            return EXIT_FAILURE;
        }
        if (scanf("%s", word) != EOF) {
            // Ignore word if it is same as previous
            if (numWords == 0 || strcmp(dictArray[numWords - 1], word) != 0) {
                dictArray[numWords] = word;
                numWords++;
            }
        }
    }
    if (numWords <= 0) {
        printf ("No words in dictionary\n");
        return EXIT_FAILURE;
    }

    // 2. Show Dictionary
    printf ("Dictionary\n");
    for (int num = 0; num < numWords; num++) {
        printf ("%d: %s\n", num, dictArray[num]);
    }

    // 2. Iterate through dictionary and insert edges into graph
    Graph graph = newGraph(numWords);
    for (int i = 0; i < numWords; i++) {
        for (int j = i + 1; j < numWords; j++) {
            if (differByOne(dictArray[i], dictArray[j]) == TRUE) {
                insertEdge(newEdge(i, j), graph);
            }
        }
    }
    printf ("Ordered Word Ladder Graph\n");
    showGraph(graph);

    // 3. Find all longest OWLs if dictionary is not empty, else do nothing
    findOWLs(graph, dictArray, numWords);

    // 4. Free data
    freeGraph(graph);
    dictArray = destroyDict (dictArray, numWords);

    return EXIT_SUCCESS;
}

/* findOWLs
 *    Uses DFS from leaf to root nodes to represent longest paths
 *    Stores paths in parentsMatrix
 *    Iterate through parentsMatrix to print all longestPaths
 */
void findOWLs (Graph graph, char ** dictArray, int numWords) {
    // 1. Traverse graph from leaf to roots until all paths traversed

    // hopsArray represents the hops from a node to the root on it's longest path(s)
        // I.e. hopsArray[leafNode] = 0 since it is 0 hops from itself
    int * hopsArray = createArray(numWords, UNVISITED);
    // array of int arrays in the form parentsMatrix[parent][child] = hops from parent to leaf through child
    int ** parentsMatrix = createParentsMatrix(numWords);

    int allVisited = 0;
    int mostHops = 0;
    int initLeaf = numWords - 1; // Initialise 1st leaf as largest node

    Quack stack = createQuack(); // stack to store nodes
    Quack stackChildren = createQuack(); // stack to store corresponding children of stack
    push (initLeaf, stack);
    push (initLeaf, stackChildren); // child of leaf is initialised as itself

    while (!allVisited) {
        while (!isEmptyQuack(stack)) {
            int node = pop(stack);
            int nodeChild = pop(stackChildren);
            int hopsToLeaf = hopsArray[nodeChild] + 1;

            // Update variables if hops from node thorugh child are > previously stored values
                // I.e. Found a longer path from node to a leaf through the child nodeChild
            if (hopsToLeaf > parentsMatrix[node][nodeChild]) {
                parentsMatrix[node][nodeChild] = hopsToLeaf;
            }
            
            if (hopsToLeaf > hopsArray[node]) {
                hopsArray[node] = hopsToLeaf;
                if (hopsToLeaf > mostHops) {
                    mostHops = hopsToLeaf;
                }
                // Placed inside if statement so that it skips for below loop if vertex is
                // visited by a same length or longer path from another leaf previously
                // Only check vertexes < current node since only those can be potential parents
                for (int vertex = node - 1; vertex >= 0; vertex--) {
                    if (isEdge(newEdge(node, vertex), graph)) { // Found a parent of node
                        push (vertex,stack);
                        push (node ,stackChildren);
                    }
                }                
            }

        }

        // Check for any disconnected subgraphs in dictionary. If there is, repeat loop
        allVisited = 1;
        int vertex = numWords - 1;
        while (vertex >= 0 && allVisited == 1) {
            if (hopsArray[vertex] == UNVISITED) {
                allVisited = 0;
                push (vertex, stack);
                push (vertex, stackChildren);
            }
            vertex--;
        }
    }

    // 2. Find longest OWLs using recursion through parentsMatrix
    int * ladderArray = createArray(numWords, FALSE); // binary array to determine words to print
    int ladderNum = 0;
    printf ("Longest ladder length: %d\n", (mostHops + 1));
    printf ("Longest ladders:\n");

    // Find root nodes of longest OWLs and recurse through parentsMatrix to leaf(s)
    for (int root = 0; root < numWords; root++) {
        if (hopsArray[root] == mostHops) {
            int hopsLeft = mostHops;
            ladderArray[root] = TRUE; // Turn bit on
            if (ladderNum < MAX_LADDERS) { // Skip if 99 ladders printed
                ladderRecursion(parentsMatrix, dictArray, ladderArray, &hopsLeft, numWords, root, &ladderNum);                
            }
            ladderArray[root] = FALSE; // Turn bit off when done
        }
    }

    // 3. Free all malloced variables
    parentsMatrix = destroyParentsMatrix(parentsMatrix, numWords);
    free (hopsArray);
    hopsArray = NULL;
    free (ladderArray);
    ladderArray = NULL;
    hopsArray = NULL;
    makeEmptyQuack (stack);
    stack = NULL;
    makeEmptyQuack (stackChildren);
    stackChildren = NULL;
}

/* ladderRecursion
 *    Recursion through parentsMatrix
 *    Only call child nodes that are part of longest OWL
 */
void ladderRecursion (int ** matrix, char ** dictArray, int * ladderArray, int * hopsLeft, int numWords, int node, int * ladderNum) {
    for (int nextNode = 0; nextNode < numWords; nextNode++) {
        if (matrix[node][nextNode] == *hopsLeft) { // Only move through nodes that result in longest OWLs
            ladderArray[nextNode] = TRUE;
            if (*hopsLeft == 0) { // We are at the leaf node of ladder
                printLadder(ladderArray, dictArray, numWords, ladderNum);
            } else {
                *hopsLeft = *hopsLeft - 1;
                if (*ladderNum < MAX_LADDERS) { // Skip if 99 ladders printed
                    ladderRecursion(matrix, dictArray, ladderArray, hopsLeft, numWords, nextNode, ladderNum);
                }
                *hopsLeft = *hopsLeft + 1; // increment since we are no longer travelling through child (nextNode)
            }
            ladderArray[nextNode] = FALSE;
        }
    }
}

/* printLadder
 *   Print longest OWLs as long as < MAX_LADDERS has been printed
 */
void printLadder (int * ladderArray, char ** dictArray, int numWords, int  * ladderNum) {
    if (*ladderNum < MAX_LADDERS) {
        int start = TRUE;
        *ladderNum = *ladderNum + 1;
        printf ("%2d: ", *ladderNum);
        for (int i = 0; i < numWords; i++) {
            if (ladderArray[i] == TRUE) {
                if (start != TRUE) {
                    printf (" -> %s", dictArray[i]);
                } else {
                    printf ("%s", dictArray[i]);
                    start = FALSE;
                }
            }
        }
        printf ("\n");
    }
}

/* createArray
 *   Malloc an array of ints
 *   Initialise all values to initValue
 */
int * createArray (int numWords, int initValue) {
    int * array = malloc(sizeof(int) * numWords);
    if (array == NULL) {
        fprintf (stderr, "Failure to malloc memory\n");
        exit (EXIT_FAILURE);
    }
    for (int i = 0; i < numWords; i++) {
        array[i] = initValue;
    }
    return array;
}

/* createParentsMatrix
 *     Creates a parentsMatrix (array(size numWords) of int arrays(size numWords))
 *     All elements initialised to -1
 *     Returns this parentsMatrix
 */
int ** createParentsMatrix (int numWords) {
    int ** parentsMatrix = malloc(sizeof(int *) * numWords);
    if (parentsMatrix == NULL) {
        fprintf (stderr, "Failure to malloc memory\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numWords; i++) {
        int * intArray = malloc(sizeof(int *) * numWords);
        if (intArray == NULL) {
            fprintf (stderr, "Failure to malloc memory\n");
            for (int j = i - 1; j >= 0; j--) { // Free all previously malloced memory
                free(parentsMatrix[j]);
            }
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < numWords; j++) {
            intArray[j] = -1;
        }
        parentsMatrix[i] = intArray;
    }
    return parentsMatrix;
}

/* destroyParentsMatrix
 *    free all memory occupied by parentsMatrix
 *     returns NULL
 */ 
int ** destroyParentsMatrix (int ** parentsMatrix, int numWords) {
    for (int i = 0; i < numWords; i++) {
        free(parentsMatrix[i]);
    }
    free(parentsMatrix);
    return NULL;
}

/* destroyDict()
 *   Free all memory stored in array of Words in Dictionary
 *   returns NULL
 */
char ** destroyDict(char ** dictArray, int numWords) {
    for (int num = 0; num < numWords; num++) {
        free(dictArray[num]);
    }
    free(dictArray);
    return NULL;
}

/* differByOne()
 *   Given 2 strings, determine if they differ by one
 *      One letter change OR one letter difference
 *      Return 1 if True, 0 if False
 *      NOTE: Assumes all input is valid so "\0" is valid
 */
int differByOne(char * word1, char * word2) {
    int retval = TRUE;
    int lenDiff = strlen(word1) - strlen(word2);

    if (abs(lenDiff) > 1) { // string len difference > 1
        retval = FALSE;
    } else {
        if (lenDiff == -1) { // word2 longer than word1, switch them
            char * wordStore = word1;
            word1 = word2;
            word2 = wordStore;
        }
        int diffCount = 0; // Number of different letters
        int pos1 = 0;
        int pos2 = 0;
        while (word1[pos1] != '\0' && word2[pos2] != '\0' && retval == TRUE) {
            if (word1[pos1] != word2[pos2]) {
                diffCount += 1;
                if (diffCount > 1) { // More than 1 letter difference
                    retval = FALSE;
                } else if (abs(lenDiff) == 1) { // word1 longer by 1 char
                    pos1 += 1; // Only increment by 1 char in word1
                } else {
                    pos1 += 1;
                    pos2 += 1;
                }
            } else {
                pos1 += 1;
                pos2 += 1;
            }
        }
    }
    return retval;
}