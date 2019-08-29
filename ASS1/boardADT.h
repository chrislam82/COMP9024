// COMP9024 2019T2
// Name: Christopher Shu Chun Lam
// zID: z3460499
// Date: 29/06/2019
// Assignment 1 - boardADT.h (ADT Interface)

# define TRUE 1
# define FALSE 0
# define FOUND_ERROR -2 // Default error found message
# define NO_ERROR -3 // Default no error found message

// PuzzleData contains all data to represent puzzle game
typedef struct puzzle_data *PuzzleData;

// Populates PuzzleData with input from stdin
	// Returns PuzzleData
	// If there is invalid input, prints error message, frees all data and returns NULL
PuzzleData generatePuzzle (void);

// Tests values and configurations of puzzle
	// If values and configurations are valid, returns TRUE
	// Else prints error message and returns FALSE
int checkConfiguration (PuzzleData puzzle);

// Tests if goal configuration is achievable.
// Runs checkConfiguration
	// If invalid configuration, returns ERROR_FOUND
	// Else, displays input and solvability
int determineSolvability (PuzzleData puzzle);

// frees all allocated memory and returns NULL
PuzzleData freeData (PuzzleData puzzle);
