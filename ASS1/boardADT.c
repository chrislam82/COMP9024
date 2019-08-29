// COMP9024 2019T2
// Date: 29/06/2019
// Assignment 1 - boardADT.c (ADT Implementation)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "boardADT.h"

# define TRUE 1
# define FALSE 0
# define FOUND_ERROR -2 // Default error found message
# define NO_ERROR -3 // Default no error found message
# define BLANK_SPACE -1
# define DEFAULT_PUZZLE_TILES 100
# define DEFAULT_CHAR '\0' // For Initialising a default char

struct puzzle_data {
	int *start;
	int *goal;
	int startLength;
	int goalLength;
};

// Calculates disorder based on disorder method for determining solvability of puzzles
int disorder (int* start, int numTiles);

// Initialise and populate PuzzleData with input from stdin
	// Returns user errors for invalid input (not an integer or 'b')
PuzzleData generatePuzzle (void) {
	// Initialise Puzzle
	PuzzleData puzzle;
	puzzle = malloc(sizeof(struct puzzle_data));
	if (!puzzle) {
		fprintf (stderr, "Error: Unable to obtain memory for data. Exiting program\n");
		freeData(puzzle);
		exit(EXIT_FAILURE);
	}
	puzzle->startLength = 0;
	puzzle->goalLength = 0;
	puzzle->start = malloc(sizeof(int) * DEFAULT_PUZZLE_TILES);
	puzzle->goal = malloc(sizeof(int) * DEFAULT_PUZZLE_TILES);
	if (!puzzle->start || !puzzle->goal) {
		fprintf (stderr, "Error: Unable to obtain memory for data. Exiting program\n");
		freeData(puzzle);
		exit(EXIT_FAILURE);
	}

	// Read data into puzzle from stdin
	int tileNum = 0;
	int tiles_allocated = DEFAULT_PUZZLE_TILES; // default memory malloced for a configuration
	int fillingGoal = FALSE; // Check if we are filling start puzzle or goal puzzle
	int *currentAddress = puzzle->start; // points to where to insert value next
	char prevChar = DEFAULT_CHAR; // Var to keep track of previous char in stdin

	char c = DEFAULT_CHAR;
	while (c != EOF) {
		c = getchar();
		switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'b':
				// Check that we do not have values like 6b1 (invalid)
				if ((c >= '0' && c <= '9' && prevChar == 'b') || (c == 'b' && prevChar >= '0' && prevChar <= '9')) {
					fprintf (stdout, "Error: Blank tile must be seperated by a space ' ' or tab '\\t' from values \n");
					puzzle = freeData(puzzle);
					return NULL;
				}
				// Check if more memory is required
				if (tileNum >= tiles_allocated) {
					tiles_allocated += DEFAULT_PUZZLE_TILES;
					if (!fillingGoal) { // resize start
						puzzle->start = realloc(puzzle->start, sizeof(int) * tiles_allocated);
						currentAddress = puzzle->start + tileNum;
					} else { // resize goal
						puzzle->goal = realloc(puzzle->goal, sizeof(int) * tiles_allocated);
						currentAddress = puzzle->goal + tileNum;
					}
					if (!currentAddress) { // Failed to realloc more memory
						fprintf (stderr, "Error: Unable to obtain memory for data. Exiting program\n");
						freeData(puzzle);
						exit(EXIT_FAILURE);						
					}
				}
				// Insert data into puzzle
				if (c == 'b') {
					*currentAddress = BLANK_SPACE;
				} else {
					if (prevChar >= '0' && prevChar <= '9') { // Check if digit preceded by another digit
						currentAddress--; // if so, revert back to previous tile and increase that value
						tileNum--;
						*currentAddress = ((*currentAddress) * 10) + (c - '0');
					} else {
						*currentAddress = c - '0';
					}
				}
				currentAddress++;
				tileNum++;
				break;
			case ' ':
			case '\t':
			case EOF:
				break;
			case '\n':
				if (!fillingGoal) { // Reset and start filling goal configuration
					fillingGoal = TRUE;
					puzzle->startLength = tileNum;
					tileNum = 0;
					currentAddress = puzzle->goal;
					tiles_allocated = DEFAULT_PUZZLE_TILES;
				}
				break;
			default:
				fprintf (stdout, "Error: Invalid input. Input must be positive integer (>0) or [b|' '|\\t|\\n]\n");
				puzzle = freeData (puzzle);
				return NULL;
		}
		prevChar = c;
	}
	if (fillingGoal) { // Insert terminating tile count into last filled configuration
		puzzle->goalLength = tileNum;
	} else {
		puzzle->startLength = tileNum;
	}
	puzzle->start = realloc(puzzle->start, sizeof(int) * puzzle->startLength);
	puzzle->goal = realloc(puzzle->goal, sizeof(int) * puzzle->goalLength);

	return puzzle;
}

int checkConfiguration (PuzzleData puzzle) {
	// 1. Check that both configurations are at least 2x2
	if (puzzle->startLength < 4 || puzzle->goalLength < 4) {
		fprintf (stdout, "Error: Minimum size of Puzzle is 2x2 tiles \n");
		return FALSE;
	}
	// 2. Check that both configurations are a perfect square
	int startSqrtLength = sqrt(puzzle->startLength); // To force floor of sqrt
	int goalSqrtLength = sqrt(puzzle->goalLength);
	if (startSqrtLength * startSqrtLength != puzzle->startLength || goalSqrtLength * goalSqrtLength != puzzle->goalLength) {
		fprintf (stdout, "Error: Input does not fill a complete NxN grid where N is the Width\n");
		return FALSE;
	}
	// 3. Check that both configurations are the same size
	if (puzzle->startLength != puzzle->goalLength) {
		fprintf (stdout, "Error: Start and Goal configurations are not the same size \n");
		return FALSE;
	}
	// 4. Check that all tiles exist, else return missing tile value
	for (int i = 0; i < puzzle->startLength; i++) {
		int currentStart = *(puzzle->start + i);
		int currentGoal = *(puzzle->goal + i);
		if (currentStart == 0 || currentGoal == 0) {
			fprintf (stdout, "Error: Tile values cannot be 0. Missing at least one tile value\n");
			return FALSE;			
		}
		if (currentStart >= puzzle->startLength || currentGoal >= puzzle->goalLength) {
			fprintf (stdout, "Error: Tile values must not exceed (puzzle width ^ 2) - 1\n");
			return FALSE;
		}
		for (int j = i + 1; j < puzzle->startLength; j++) {
			if (currentStart == *(puzzle->start + j) || currentGoal == *(puzzle->goal + j)) {
				fprintf (stdout, "Error: At least one tile value is repeated \n");
				return FALSE;
			}
		}
	}
	return TRUE;
}

// Calculates disorder based on disorder method for determining solvability of puzzles
int disorder (int* start, int numTiles) {
	int numRows = sqrt(numTiles);
	int disorder = 0;

	// Iterate through data and check subsequent tiles for repeating values
	for (int i = 0; i < numTiles; i++) {
		int current = *(start + i);
		if (current == BLANK_SPACE && numRows % 2 == 0) {
			disorder += (i / numRows) + 1;
		} else {
			for (int j = i + 1; j < numTiles; j++) {
				if (current > *(start + j) && *(start + j) != BLANK_SPACE) {
					disorder++;
				}
			}
		}
	}
	return disorder;
}

// Compares disorder of configurations to determine solvability
int determineSolvability (PuzzleData puzzle) {
	if (checkConfiguration(puzzle) == FALSE) { // Calls checkConfiguration in case user does not
		return FOUND_ERROR;
	}

	int startDisorder = disorder(puzzle->start, puzzle->startLength);
	int goalDisorder = disorder(puzzle->goal, puzzle->goalLength);

	// Display Configurations and solvable or unsolvable
	fprintf (stdout, "start:");
	for (int i = 0; i < puzzle->startLength; i++) {
		if (*(puzzle->start + i) == BLANK_SPACE) {
			fprintf (stdout, " b");
		} else {
			fprintf (stdout, " %d", *(puzzle->start + i));
		}
	}
	fprintf (stdout, "\n");
	fprintf (stdout, "goal:");
	for (int i = 0; i < puzzle->goalLength; i++) {
		if (*(puzzle->goal + i) == BLANK_SPACE) {
			fprintf (stdout, " b");
		} else {
			fprintf (stdout, " %d", *(puzzle->goal + i));
		}
	}
	fprintf (stdout, "\n");
	if (startDisorder % 2 == goalDisorder % 2) {
		fprintf (stdout, "solvable\n");
	} else {
		fprintf (stdout, "unsolvable\n");
	}
	return NO_ERROR;
}

// Frees all malloced memory
	// Checks that pointers are not NULL pointers first
	// NOTE: ADT implementation only frees if data cannot be entered. Else, it is up to user.
PuzzleData freeData (PuzzleData puzzle) {
	free(puzzle->start);
	puzzle->start = NULL;
	free(puzzle->goal);
	puzzle->goal = NULL;
	free(puzzle);
	return NULL;
}
