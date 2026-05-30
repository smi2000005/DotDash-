#ifndef M1_ENCODER_H
#define M1_ENCODER_H

#include "../include/types.h"
#include <stddef.h>

int loadMorseTable(AlphabetSet *outAlphabet, size_t maxEntries, const char *dataPath);
int encodeText(const char *input, const AlphabetSet *alphabet, char *outBuf, size_t outBufSize);
int decodeMorse(const char *morseInput, const AlphabetSet *alphabet, char *outBuf, size_t outBufSize);
int validateMorseInput(const char *morseInput);
int lookupCharacter(const MorseEntry *table, int tableLen, char ch);
const char *getMorseForChar(const AlphabetSet *alphabet, char ch);
int getCharFromMorse(const AlphabetSet *alphabet, const char *morse, char *outChar);

#endif // M1_ENCODER_H
