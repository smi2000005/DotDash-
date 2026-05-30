#include "m3_user.h"
#include "../include/errors.h"
#include "utils.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

static int isNumericPin(const char *pin) {
    if (!pin) return 0;
    size_t len = strlen(pin);
    if (len < 4 || len > 6) return 0;
    for (size_t i = 0; i < len; ++i) if (!isdigit((unsigned char)pin[i])) return 0;
    return 1;
}

int registerUser(UserProfile *users, size_t *userCount, size_t maxUsers, const char *username, const char *pin, UserProfile *outUser) {
    if (!users || !userCount || !username || !pin) return ERR_ARG;
    if (*userCount >= maxUsers) return ERR_NOMEM;
    if (!isNumericPin(pin)) return ERR_ARG;
    int existing = -1;
    searchUserByName(users, *userCount, username, &existing);
    if (existing >= 0) return ERR_NOTFOUND;
    UserProfile u = {0};
    u.userID = (int)(*userCount + 1);
    safe_str_copy(u.username, sizeof(u.username), username);
    safe_str_copy(u.pin, sizeof(u.pin), pin);
    u.totalEncodes = 0;
    u.totalDecodes = 0;
    u.preferredAlphabet = 0;
    safe_str_copy(u.regDate, sizeof(u.regDate), "2026-05-30");
    users[*userCount] = u;
    if (outUser) *outUser = u;
    (*userCount)++;
    return OK;
}

int loginUser(const UserProfile *users, size_t userCount, const char *username, const char *pin, UserProfile *outUser) {
    if (!users || !username || !pin) return ERR_ARG;
    for (size_t i = 0; i < userCount; ++i) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].pin, pin) == 0) {
            if (outUser) *outUser = users[i];
            return OK;
        }
    }
    return ERR_NOTFOUND;
}

void displayUserStats(const UserProfile *user, FILE *outStream) {
    if (!user) return;
    if (!outStream) outStream = stdout;
    fprintf(outStream, "User %s (ID %d) registered=%s encodes=%d decodes=%d prefAlphabet=%d\n",
            user->username, user->userID, user->regDate, user->totalEncodes, user->totalDecodes, user->preferredAlphabet);
}

int updateUserStats(UserProfile *user, char mode) {
    if (!user) return ERR_ARG;
    if (mode == 'E') user->totalEncodes++;
    else if (mode == 'D') user->totalDecodes++;
    else return ERR_ARG;
    return OK;
}

int searchUserByName(const UserProfile *users, size_t userCount, const char *username, int *outIndex) {
    if (!users || !username || !outIndex) return ERR_ARG;
    *outIndex = -1;
    for (size_t i = 0; i < userCount; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            *outIndex = (int)i;
            return OK;
        }
    }
    return ERR_NOTFOUND;
}

int sortUsersByActivity(UserProfile *users, size_t userCount) {
    if (!users) return ERR_ARG;
    for (size_t i = 0; i + 1 < userCount; ++i) {
        for (size_t j = 0; j + 1 < userCount - i; ++j) {
            int a = users[j].totalEncodes + users[j].totalDecodes;
            int b = users[j + 1].totalEncodes + users[j + 1].totalDecodes;
            if (a < b) {
                UserProfile tmp = users[j];
                users[j] = users[j + 1];
                users[j + 1] = tmp;
            }
        }
    }
    return OK;
}

int deleteUser(UserProfile *users, size_t *userCount, int userID) {
    if (!users || !userCount) return ERR_ARG;
    size_t idx = (size_t)-1;
    for (size_t i = 0; i < *userCount; ++i) {
        if (users[i].userID == userID) {
            idx = i;
            break;
        }
    }
    if (idx == (size_t)-1) return ERR_NOTFOUND;
    for (size_t i = idx; i + 1 < *userCount; ++i) {
        users[i] = users[i + 1];
    }
    (*userCount)--;
    return OK;
}
