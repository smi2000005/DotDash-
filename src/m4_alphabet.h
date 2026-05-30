#ifndef M4_ALPHABET_H
#define M4_ALPHABET_H

#include "../include/types.h"
#include <stddef.h>
#include <stdio.h>

int initAlphabets(AlphabetSet *alphabets, size_t *outCount, size_t maxAlphabets, const char *dataPath);
int switchAlphabet(const AlphabetSet *alphabets, size_t alphCount, int alphabetID, AlphabetSet *outActive);
void displayAlphabetTable(const AlphabetSet *alphabet, FILE *outStream);
int addCustomEntry(AlphabetSet *alphabet, char ch, const char *morse);
int removeCustomEntry(AlphabetSet *alphabet, char ch);
int mergeAlphabets(const AlphabetSet *a, const AlphabetSet *b, AlphabetSet *outMerged, size_t maxEntries);
int validateAlphabetCoverage(const AlphabetSet *alphabet, const char *inputChars, char *missingBuf, size_t missingBufLen);

#endif // M4_ALPHABET_H
