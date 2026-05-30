#ifndef M5_ANALYTICS_H
#define M5_ANALYTICS_H

#include "../include/types.h"
#include <stddef.h>
#include <stdio.h>

int generateReport(const SessionLog *logs, size_t logCount, AnalyticsReport *outReport);
void printReportSummary(const AnalyticsReport *report, FILE *outStream);
int exportReportToFile(const AnalyticsReport *report, const char *path);
int getMostUsedCharacter(const SessionLog *logs, size_t logCount, char *outChar);
int sortLogsByDate(SessionLog *logs, size_t logCount);
int searchLogByKeyword(const SessionLog *logs, size_t logCount, const char *keyword, SessionLog *outBuf, size_t *outCount, size_t maxOut);
float calculateErrorRate(const SessionLog *logs, size_t logCount);

#endif // M5_ANALYTICS_H
