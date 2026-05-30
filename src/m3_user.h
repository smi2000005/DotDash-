#ifndef M3_USER_H
#define M3_USER_H

#include "../include/types.h"
#include <stddef.h>
#include <stdio.h>

int registerUser(UserProfile *users, size_t *userCount, size_t maxUsers, const char *username, const char *pin, UserProfile *outUser);
int loginUser(const UserProfile *users, size_t userCount, const char *username, const char *pin, UserProfile *outUser);
void displayUserStats(const UserProfile *user, FILE *outStream);
int updateUserStats(UserProfile *user, char mode);
int searchUserByName(const UserProfile *users, size_t userCount, const char *username, int *outIndex);
int sortUsersByActivity(UserProfile *users, size_t userCount);
int deleteUser(UserProfile *users, size_t *userCount, int userID);

#endif // M3_USER_H
