#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

typedef struct {
    char character;
    char morseCode[12];
    int alphabetType;
    int usageCount;
} MorseEntry;

typedef struct {
    int sessionID;
    int userID;
    char timestamp[20];
    char inputText[256];
    char morseOutput[1024];
    char mode;
    int durationMs;
    int errorFlag;
} SessionLog;

typedef struct {
    int userID;
    char username[50];
    char pin[7];
    int totalEncodes;
    int totalDecodes;
    int preferredAlphabet;
    char regDate[12];
} UserProfile;

typedef struct {
    int alphabetID;
    char name[30];
    MorseEntry entries[128];
    int entryCount;
    int enabled;
} AlphabetSet;

typedef struct {
    int reportID;
    int generatedByUserID;
    char date[20];
    int totalSessions;
    char mostActiveUser[50];
    char mostUsedChar;
    float avgMessageLength;
    int totalErrors;
} AnalyticsReport;

#endif // TYPES_H
