#include "m2_logger.h"
#include "../include/errors.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int parseTimestamp(const char *timestamp, struct tm *outTm) {
    if (!timestamp || !outTm) return ERR_ARG;
    memset(outTm, 0, sizeof(*outTm));
    if (sscanf(timestamp, "%4d-%2d-%2d %2d:%2d:%2d",
        &outTm->tm_year, &outTm->tm_mon, &outTm->tm_mday,
        &outTm->tm_hour, &outTm->tm_min, &outTm->tm_sec) != 6) {
        return ERR_ARG;
    }
    outTm->tm_year -= 1900;
    outTm->tm_mon -= 1;
    return OK;
}

int createSession(SessionLog *outSession, int userID, const char *mode) {
    if (!outSession) return ERR_ARG;
    outSession->sessionID = 0;
    outSession->userID = userID;
    now_timestamp(outSession->timestamp, sizeof(outSession->timestamp));
    outSession->inputText[0] = '\0';
    outSession->morseOutput[0] = '\0';
    outSession->mode = (mode && mode[0]) ? mode[0] : 'E';
    outSession->durationMs = 0;
    outSession->errorFlag = 0;
    return OK;
}

int logTranslation(SessionLog *logs, size_t *logCount, size_t maxLogs, const SessionLog *entry) {
    if (!logs || !logCount || !entry) return ERR_ARG;
    if (*logCount >= maxLogs) return ERR_NOMEM;
    logs[*logCount] = *entry;
    (*logCount)++;
    return OK;
}

int saveLogsToFile(const SessionLog *logs, size_t logCount, const char *path) {
    if (!logs || !path) return ERR_ARG;
    FILE *f = fopen(path, "w");
    if (!f) return ERR_IO;
    for (size_t i = 0; i < logCount; ++i) {
        fprintf(f, "%d|%d|%s|%c|%d|%d|%s|%s\n",
            logs[i].sessionID,
            logs[i].userID,
            logs[i].timestamp,
            logs[i].mode,
            logs[i].durationMs,
            logs[i].errorFlag,
            logs[i].inputText,
            logs[i].morseOutput);
    }
    fclose(f);
    return OK;
}

int loadLogsFromFile(SessionLog *outLogs, size_t *outLogCount, size_t maxLogs, const char *path) {
    if (!outLogs || !outLogCount || !path) return ERR_ARG;
    FILE *f = fopen(path, "r");
    if (!f) return ERR_IO;
    char line[2048];
    *outLogCount = 0;
    while (fgets(line, sizeof(line), f)) {
        if (*outLogCount >= maxLogs) break;
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#') continue;
        line[strcspn(line, "\r\n")] = '\0';
        SessionLog entry = {0};
        char inputText[256];
        char morseOutput[1024];
        if (sscanf(line, "%d|%d|%19[^|]|%c|%d|%d|%255[^|]|%1023[^\n]",
                 &entry.sessionID,
                 &entry.userID,
                 entry.timestamp,
                 &entry.mode,
                 &entry.durationMs,
                 &entry.errorFlag,
                 inputText,
                 morseOutput) < 8) {
            continue;
        }
        safe_str_copy(entry.inputText, sizeof(entry.inputText), inputText);
        safe_str_copy(entry.morseOutput, sizeof(entry.morseOutput), morseOutput);
        outLogs[*outLogCount] = entry;
        (*outLogCount)++;
    }
    fclose(f);
    return OK;
}

void displaySessionHistory(const SessionLog *logs, size_t logCount, FILE *outStream) {
    if (!outStream) outStream = stdout;
    fprintf(outStream, "SessionID | UserID | Timestamp | Mode | Duration | Error | Input -> Morse\n");
    for (size_t i = 0; i < logCount; ++i) {
        fprintf(outStream, "%d | %d | %s | %c | %d | %d | %s -> %s\n",
            logs[i].sessionID,
            logs[i].userID,
            logs[i].timestamp,
            logs[i].mode,
            logs[i].durationMs,
            logs[i].errorFlag,
            logs[i].inputText,
            logs[i].morseOutput);
    }
}

int filterLogsByUser(const SessionLog *logs, size_t logCount, int userID, SessionLog *outBuf, size_t *outCount, size_t maxOut) {
    if (!logs || !outBuf || !outCount) return ERR_ARG;
    *outCount = 0;
    for (size_t i = 0; i < logCount && *outCount < maxOut; ++i) {
        if (logs[i].userID == userID) {
            outBuf[*outCount] = logs[i];
            (*outCount)++;
        }
    }
    return OK;
}

int clearOldLogs(SessionLog *logs, size_t *logCount, int daysThreshold) {
    if (!logs || !logCount) return ERR_ARG;
    time_t now = time(NULL);
    size_t writeIndex = 0;
    for (size_t i = 0; i < *logCount; ++i) {
        struct tm tm;
        if (parseTimestamp(logs[i].timestamp, &tm) != OK) continue;
        time_t entryTime = mktime(&tm);
        if (entryTime == (time_t)-1) continue;
        double diffDays = difftime(now, entryTime) / (60.0 * 60.0 * 24.0);
        if (diffDays <= daysThreshold) {
            logs[writeIndex++] = logs[i];
        }
    }
    *logCount = writeIndex;
    return OK;
}
