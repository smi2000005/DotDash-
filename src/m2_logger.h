#ifndef M2_LOGGER_H
#define M2_LOGGER_H

#include "../include/types.h"
#include <stddef.h>
#include <stdio.h>

int createSession(SessionLog *outSession, int userID, const char *mode);
int logTranslation(SessionLog *logs, size_t *logCount, size_t maxLogs, const SessionLog *entry);
int saveLogsToFile(const SessionLog *logs, size_t logCount, const char *path);
int loadLogsFromFile(SessionLog *outLogs, size_t *outLogCount, size_t maxLogs, const char *path);
void displaySessionHistory(const SessionLog *logs, size_t logCount, FILE *outStream);
int filterLogsByUser(const SessionLog *logs, size_t logCount, int userID, SessionLog *outBuf, size_t *outCount, size_t maxOut);
int clearOldLogs(SessionLog *logs, size_t *logCount, int daysThreshold);

#endif // M2_LOGGER_H
