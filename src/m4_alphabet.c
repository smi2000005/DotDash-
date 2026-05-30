#include "m4_alphabet.h"
#include "../include/errors.h"
#include "../src/m1_encoder.h"
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int containsEntry(const AlphabetSet *alphabet, char ch) {
    for (int i = 0; i < alphabet->entryCount; ++i) {
        if (alphabet->entries[i].character == ch) return 1;
    }
    return 0;
}

int initAlphabets(AlphabetSet *alphabets, size_t *outCount, size_t maxAlphabets, const char *dataPath) {
    if (!alphabets || !outCount || maxAlphabets == 0 || !dataPath) return ERR_ARG;
    *outCount = 0;
    AlphabetSet latin = {0};
    latin.alphabetID = 0;
    safe_str_copy(latin.name, sizeof(latin.name), "Latin");
    latin.enabled = 1;
    if (loadMorseTable(&latin, 128, dataPath) != OK) return ERR_IO;
    alphabets[(*outCount)++] = latin;

    if (*outCount < maxAlphabets) {
        AlphabetSet digits = {0};
        digits.alphabetID = 1;
        safe_str_copy(digits.name, sizeof(digits.name), "Digits");
        digits.enabled = 0;
        const char *digitMap[] = {"0|-----", "1|.----", "2|..---", "3|...--", "4|....-", "5|.....", "6|-....", "7|--...", "8|---..", "9|----."};
        for (size_t i = 0; i < 10 && digits.entryCount < 128; ++i) {
            char ch = digitMap[i][0];
            const char *code = digitMap[i] + 2;
            digits.entries[digits.entryCount].character = ch;
            safe_str_copy(digits.entries[digits.entryCount].morseCode, sizeof(digits.entries[digits.entryCount].morseCode), code);
            digits.entries[digits.entryCount].alphabetType = 1;
            digits.entryCount++;
        }
        alphabets[(*outCount)++] = digits;
    }

    if (*outCount < maxAlphabets) {
        AlphabetSet prosigns = {0};
        prosigns.alphabetID = 2;
        safe_str_copy(prosigns.name, sizeof(prosigns.name), "Prosign");
        prosigns.enabled = 0;
        const char *terms[] = {"AR|.-.-.", "BT|-...-", "SK|...-.-", "SOS|...---..."};
        for (size_t i = 0; i < 4 && prosigns.entryCount < 128; ++i) {
            char ch = terms[i][0];
            const char *code = terms[i] + 3;
            prosigns.entries[prosigns.entryCount].character = ch;
            safe_str_copy(prosigns.entries[prosigns.entryCount].morseCode, sizeof(prosigns.entries[prosigns.entryCount].morseCode), code);
            prosigns.entries[prosigns.entryCount].alphabetType = 3;
            prosigns.entryCount++;
        }
        alphabets[(*outCount)++] = prosigns;
    }
    return OK;
}

int switchAlphabet(const AlphabetSet *alphabets, size_t alphCount, int alphabetID, AlphabetSet *outActive) {
    if (!alphabets || !outActive) return ERR_ARG;
    for (size_t i = 0; i < alphCount; ++i) {
        if (alphabets[i].alphabetID == alphabetID) {
            *outActive = alphabets[i];
            return OK;
        }
    }
    return ERR_NOTFOUND;
}

void displayAlphabetTable(const AlphabetSet *alphabet, FILE *outStream) {
    if (!alphabet) return;
    if (!outStream) outStream = stdout;
    fprintf(outStream, "Alphabet %s (ID %d) entries=%d\n", alphabet->name, alphabet->alphabetID, alphabet->entryCount);
    for (int i = 0; i < alphabet->entryCount; ++i) {
        fprintf(outStream, " %c -> %s\n", alphabet->entries[i].character, alphabet->entries[i].morseCode);
    }
}

static void sortAlphabetEntries(AlphabetSet *alphabet) {
    for (int i = 1; i < alphabet->entryCount; ++i) {
        MorseEntry key = alphabet->entries[i];
        int j = i - 1;
        while (j >= 0 && alphabet->entries[j].character > key.character) {
            alphabet->entries[j + 1] = alphabet->entries[j];
            j--;
        }
        alphabet->entries[j + 1] = key;
    }
}

int addCustomEntry(AlphabetSet *alphabet, char ch, const char *morse) {
    if (!alphabet || !morse) return ERR_ARG;
    if (alphabet->entryCount >= 128) return ERR_NOMEM;
    ch = (char)toupper((unsigned char)ch);
    if (containsEntry(alphabet, ch)) return ERR_NOTFOUND;
    alphabet->entries[alphabet->entryCount].character = ch;
    safe_str_copy(alphabet->entries[alphabet->entryCount].morseCode, sizeof(alphabet->entries[alphabet->entryCount].morseCode), morse);
    alphabet->entries[alphabet->entryCount].alphabetType = alphabet->alphabetID;
    alphabet->entryCount++;
    sortAlphabetEntries(alphabet);
    return OK;
}

int removeCustomEntry(AlphabetSet *alphabet, char ch) {
    if (!alphabet) return ERR_ARG;
    ch = (char)toupper((unsigned char)ch);
    for (int i = 0; i < alphabet->entryCount; ++i) {
        if (alphabet->entries[i].character == ch) {
            for (int j = i; j + 1 < alphabet->entryCount; ++j) {
                alphabet->entries[j] = alphabet->entries[j + 1];
            }
            alphabet->entryCount--;
            return OK;
        }
    }
    return ERR_NOTFOUND;
}

int mergeAlphabets(const AlphabetSet *a, const AlphabetSet *b, AlphabetSet *outMerged, size_t maxEntries) {
    if (!a || !b || !outMerged || maxEntries == 0) return ERR_ARG;
    outMerged->entryCount = 0;
    outMerged->alphabetID = -1;
    safe_str_copy(outMerged->name, sizeof(outMerged->name), "Merged");
    outMerged->enabled = 1;
    for (int i = 0; i < a->entryCount && outMerged->entryCount < (int)maxEntries; ++i) {
        outMerged->entries[outMerged->entryCount++] = a->entries[i];
    }
    for (int i = 0; i < b->entryCount && outMerged->entryCount < (int)maxEntries; ++i) {
        if (!containsEntry(outMerged, b->entries[i].character)) {
            outMerged->entries[outMerged->entryCount++] = b->entries[i];
        }
    }
    return OK;
}

int validateAlphabetCoverage(const AlphabetSet *alphabet, const char *inputChars, char *missingBuf, size_t missingBufLen) {
    if (!alphabet || !inputChars || !missingBuf) return ERR_ARG;
    size_t pos = 0;
    for (const char *p = inputChars; *p; ++p) {
        char ch = *p;
        if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') continue;
        ch = (char)toupper((unsigned char)ch);
        if (!containsEntry(alphabet, ch)) {
            int already = 0;
            for (size_t j = 0; j < pos; ++j) if (missingBuf[j] == ch) already = 1;
            if (!already && pos + 2 < missingBufLen) missingBuf[pos++] = ch;
        }
    }
    missingBuf[pos] = '\0';
    return OK;
}
