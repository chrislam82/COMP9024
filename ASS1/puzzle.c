// COMP9024 2019T2
// Date: 29/06/2019
// Assignment 1 - puzzle.c
// Reads input for a sliding puzzle and determines solvability if input is valid

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "boardADT.h"

int main (void) {
	PuzzleData puzzle = generatePuzzle();
	if (!puzzle) { // invalid input
		return EXIT_FAILURE;
	}

	if (determineSolvability (puzzle) == FOUND_ERROR) { // missing value(s) / invalid configuration(s)
		puzzle = freeData(puzzle);
		return EXIT_FAILURE;
	}

	puzzle = freeData(puzzle);

	return EXIT_SUCCESS;
}
