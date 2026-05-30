#include "m1_encoder.h"
#include "../include/errors.h"
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compareMorseEntry(const void *a, const void *b) {
    const MorseEntry *ea = a;
    const MorseEntry *eb = b;
    return (int)ea->character - (int)eb->character;
}

int loadMorseTable(AlphabetSet *outAlphabet, size_t maxEntries, const char *dataPath) {
    if (!outAlphabet || !dataPath || maxEntries == 0) return ERR_ARG;
    FILE *f = fopen(dataPath, "r");
    if (!f) return ERR_IO;

    outAlphabet->alphabetID = 0;
    safe_str_copy(outAlphabet->name, sizeof(outAlphabet->name), "Latin");
    outAlphabet->entryCount = 0;
    outAlphabet->enabled = 1;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\0' || line[0] == '#') continue;
        char ch;
        char morse[12];
        if (sscanf(line, " %c | %11[.-] ", &ch, morse) != 2) {
            if (sscanf(line, " %c|%11[.-] ", &ch, morse) != 2) continue;
        }
        if (outAlphabet->entryCount >= (int)maxEntries) break;
        outAlphabet->entries[outAlphabet->entryCount].character = (char)toupper((unsigned char)ch);
        safe_str_copy(outAlphabet->entries[outAlphabet->entryCount].morseCode, sizeof(outAlphabet->entries[outAlphabet->entryCount].morseCode), morse);
        outAlphabet->entries[outAlphabet->entryCount].alphabetType = 0;
        outAlphabet->entries[outAlphabet->entryCount].usageCount = 0;
        outAlphabet->entryCount++;
    }
    fclose(f);
    qsort(outAlphabet->entries, outAlphabet->entryCount, sizeof(MorseEntry), compareMorseEntry);
    return OK;
}

static int appendToBuffer(char *dst, size_t dstSize, const char *src) {
    size_t len = strlen(dst);
    size_t add = strlen(src);
    if (len + add + 1 > dstSize) return ERR_NOMEM;
    memcpy(dst + len, src, add + 1);
    return OK;
}

int encodeText(const char *input, const AlphabetSet *alphabet, char *outBuf, size_t outBufSize) {
    if (!input || !alphabet || !outBuf) return ERR_ARG;
    outBuf[0] = '\0';
    for (const char *p = input; *p; ++p) {
        char ch = *p;
        if (ch == ' ') {
            if (appendToBuffer(outBuf, outBufSize, "/ ") != OK) return ERR_NOMEM;
            continue;
        }
        if (ch == '\n' || ch == '\r' || ch == '\t') continue;
        const char *morse = getMorseForChar(alphabet, ch);
        if (!morse) morse = getMorseForChar(alphabet, (char)toupper((unsigned char)ch));
        if (!morse) return ERR_NOTFOUND;
        if (appendToBuffer(outBuf, outBufSize, morse) != OK) return ERR_NOMEM;
        if (appendToBuffer(outBuf, outBufSize, " ") != OK) return ERR_NOMEM;
    }
    size_t outLen = strlen(outBuf);
    if (outLen > 0 && outBuf[outLen - 1] == ' ') outBuf[outLen - 1] = '\0';
    return OK;
}

int decodeMorse(const char *morseInput, const AlphabetSet *alphabet, char *outBuf, size_t outBufSize) {
    if (!morseInput || !alphabet || !outBuf) return ERR_ARG;
    if (!validateMorseInput(morseInput)) return ERR_ARG;
    outBuf[0] = '\0';
    char token[16];
    const char *start = morseInput;
    while (*start) {
        while (*start == ' ') start++;
        if (*start == '\0') break;
        if (*start == '/') {
            if (appendToBuffer(outBuf, outBufSize, " ") != OK) return ERR_NOMEM;
            start++;
            continue;
        }
        char *dst = token;
        while (*start && *start != ' ' && *start != '/') {
            *dst++ = *start++;
            if ((size_t)(dst - token) >= sizeof(token) - 1) break;
        }
        *dst = '\0';
        if (token[0] == '\0') continue;
        char decoded = 0;
        if (getCharFromMorse(alphabet, token, &decoded) != OK) return ERR_NOTFOUND;
        char str[2] = {decoded, '\0'};
        if (appendToBuffer(outBuf, outBufSize, str) != OK) return ERR_NOMEM;
    }
    return OK;
}

int validateMorseInput(const char *morseInput) {
    if (!morseInput) return 0;
    for (const char *p = morseInput; *p; ++p) {
        if (*p != '.' && *p != '-' && *p != '/' && *p != ' ' && *p != '\n' && *p != '\r' && *p != '\t') return 0;
    }
    return 1;
}

int lookupCharacter(const MorseEntry *table, int tableLen, char ch) {
    if (!table || tableLen <= 0) return -1;
    ch = (char)toupper((unsigned char)ch);
    int low = 0;
    int high = tableLen - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        char mc = table[mid].character;
        if (mc == ch) return mid;
        if (mc < ch) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

const char *getMorseForChar(const AlphabetSet *alphabet, char ch) {
    if (!alphabet) return NULL;
    int idx = lookupCharacter(alphabet->entries, alphabet->entryCount, ch);
    return (idx >= 0) ? alphabet->entries[idx].morseCode : NULL;
}

int getCharFromMorse(const AlphabetSet *alphabet, const char *morse, char *outChar) {
    if (!alphabet || !morse || !outChar) return ERR_ARG;
    for (int i = 0; i < alphabet->entryCount; ++i) {
        if (strcmp(alphabet->entries[i].morseCode, morse) == 0) {
            *outChar = alphabet->entries[i].character;
            return OK;
        }
    }
    return ERR_NOTFOUND;
}
