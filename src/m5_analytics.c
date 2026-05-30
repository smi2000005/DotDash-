#include "m5_analytics.h"
#include "../include/errors.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int compareTimestamp(const char *a, const char *b) {
    return strcmp(a, b);
}

static void lowercaseString(char *out, const char *in, size_t maxLen) {
    size_t idx = 0;
    while (*in && idx + 1 < maxLen) {
        out[idx++] = (char)tolower((unsigned char)*in++);
    }
    out[idx] = '\0';
}

int getMostUsedCharacter(const SessionLog *logs, size_t logCount, char *outChar) {
    if (!logs || !outChar) return ERR_ARG;
    int counts[256] = {0};
    for (size_t i = 0; i < logCount; ++i) {
        const char *p = logs[i].inputText;
        while (*p) {
            if (*p != ' ') counts[(unsigned char)*p]++;
            p++;
        }
    }
    int best = 0;
    char bestChar = '\0';
    for (int i = 0; i < 256; ++i) {
        if (counts[i] > best) {
            best = counts[i];
            bestChar = (char)i;
        }
    }
    *outChar = bestChar;
    return OK;
}

int generateReport(const SessionLog *logs, size_t logCount, AnalyticsReport *outReport) {
    if (!logs || !outReport) return ERR_ARG;
    outReport->reportID = 1;
    outReport->generatedByUserID = logCount > 0 ? logs[0].userID : 0;
    now_timestamp(outReport->date, sizeof(outReport->date));
    outReport->totalSessions = (int)logCount;
    outReport->totalErrors = 0;
    int userCounts[101] = {0};
    size_t totalLength = 0;
    for (size_t i = 0; i < logCount; ++i) {
        if (logs[i].errorFlag) outReport->totalErrors++;
        if (logs[i].userID >= 0 && logs[i].userID < 101) userCounts[logs[i].userID]++;
        totalLength += strlen(logs[i].inputText);
    }
    int mostActiveUserID = 0;
    int bestCount = 0;
    for (int i = 0; i < 101; ++i) {
        if (userCounts[i] > bestCount) { bestCount = userCounts[i]; mostActiveUserID = i; }
    }
    if (bestCount > 0) {
        snprintf(outReport->mostActiveUser, sizeof(outReport->mostActiveUser), "User%d", mostActiveUserID);
    } else {
        safe_str_copy(outReport->mostActiveUser, sizeof(outReport->mostActiveUser), "N/A");
    }
    outReport->avgMessageLength = logCount ? (float)totalLength / (float)logCount : 0.0f;
    getMostUsedCharacter(logs, logCount, &outReport->mostUsedChar);
    return OK;
}

void printReportSummary(const AnalyticsReport *report, FILE *outStream) {
    if (!report) return;
    if (!outStream) outStream = stdout;
    fprintf(outStream, "Report ID: %d\nUser ID: %d\nDate: %s\nSessions: %d\nErrors: %d\nMost active user: %s\nMost used char: %c\nAverage message length: %.2f\n",
            report->reportID,
            report->generatedByUserID,
            report->date,
            report->totalSessions,
            report->totalErrors,
            report->mostActiveUser,
            report->mostUsedChar ? report->mostUsedChar : '-',
            report->avgMessageLength);
}

int exportReportToFile(const AnalyticsReport *report, const char *path) {
    if (!report || !path) return ERR_ARG;
    FILE *f = fopen(path, "w");
    if (!f) return ERR_IO;
    fprintf(f, "Report ID: %d\nUser: %d\nDate: %s\nSessions: %d\nErrors: %d\nMost active user: %s\nMost used char: %c\nAverage length: %.2f\n",
            report->reportID,
            report->generatedByUserID,
            report->date,
            report->totalSessions,
            report->totalErrors,
            report->mostActiveUser,
            report->mostUsedChar ? report->mostUsedChar : '-',
            report->avgMessageLength);
    fclose(f);
    return OK;
}

int sortLogsByDate(SessionLog *logs, size_t logCount) {
    if (!logs) return ERR_ARG;
    for (size_t i = 0; i + 1 < logCount; ++i) {
        for (size_t j = 0; j + 1 < logCount - i; ++j) {
            if (compareTimestamp(logs[j].timestamp, logs[j + 1].timestamp) > 0) {
                SessionLog tmp = logs[j];
                logs[j] = logs[j + 1];
                logs[j + 1] = tmp;
            }
        }
    }
    return OK;
}

int searchLogByKeyword(const SessionLog *logs, size_t logCount, const char *keyword, SessionLog *outBuf, size_t *outCount, size_t maxOut) {
    if (!logs || !keyword || !outBuf || !outCount) return ERR_ARG;
    *outCount = 0;
    char lowerKeyword[256];
    lowercaseString(lowerKeyword, keyword, sizeof(lowerKeyword));
    for (size_t i = 0; i < logCount && *outCount < maxOut; ++i) {
        char lowerInput[256];
        lowercaseString(lowerInput, logs[i].inputText, sizeof(lowerInput));
        char lowerMorse[1024];
        lowercaseString(lowerMorse, logs[i].morseOutput, sizeof(lowerMorse));
        if (strstr(lowerInput, lowerKeyword) || strstr(lowerMorse, lowerKeyword)) {
            outBuf[*outCount] = logs[i];
            (*outCount)++;
        }
    }
    return OK;
}

float calculateErrorRate(const SessionLog *logs, size_t logCount) {
    if (!logs || logCount == 0) return 0.0f;
    int errors = 0;
    for (size_t i = 0; i < logCount; ++i) if (logs[i].errorFlag) errors++;
    return (float)errors / (float)logCount;
}
