#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/types.h"
#include "../include/errors.h"
#include "m1_encoder.h"
#include "m2_logger.h"
#include "m3_user.h"
#include "m4_alphabet.h"
#include "m5_analytics.h"
#include "utils.h"

static void promptString(const char *prompt, char *buf, size_t len) {
    printf("%s", prompt);
    if (!fgets(buf, (int)len, stdin)) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

static int promptInt(const char *prompt) {
    char buf[32];
    promptString(prompt, buf, sizeof(buf));
    return atoi(buf);
}

static void showMainMenu(const char *username, const char *alphabetName) {
    printf("\n=== MORSE CODE TRANSLATOR & LOGGER ===\n");
    printf("Logged in as: %s   Alphabet: %s\n", username, alphabetName);
    printf("1. Encode Text -> Morse\n");
    printf("2. Decode Morse -> Text\n");
    printf("3. View Session History\n");
    printf("4. Manage Users\n");
    printf("5. Alphabet Settings\n");
    printf("6. Analytics & Reports\n");
    printf("0. Exit\n");
}

static void showUserMenu(void) {
    printf("\nUser Menu:\n");
    printf("1. Register user\n");
    printf("2. Login user\n");
    printf("3. Display current user stats\n");
    printf("0. Back\n");
}

static void showAlphabetMenu(void) {
    printf("\nAlphabet Menu:\n");
    printf("1. Switch alphabet\n");
    printf("2. Display active alphabet table\n");
    printf("3. Add custom entry\n");
    printf("4. Validate input coverage\n");
    printf("0. Back\n");
}

int main(void) {
    AlphabetSet alphabets[8];
    size_t alphCount = 0;
    AlphabetSet activeAlphabet;
    SessionLog logs[1000];
    size_t logCount = 0;
    UserProfile users[100];
    size_t userCount = 0;
    UserProfile currentUser = {0};
    char currentUsername[50] = "Guest";
    int currentUserID = 0;
    int nextSessionID = 1;

    if (initAlphabets(alphabets, &alphCount, 8, "data/morse_table.txt") != OK) {
        fprintf(stderr, "Failed to initialize alphabets\n");
        return 1;
    }

    if (loadLogsFromFile(logs, &logCount, 1000, "data/sessions.txt") != OK) {
        printf("No previous logs found, starting fresh.\n");
    }

    if (switchAlphabet(alphabets, alphCount, 0, &activeAlphabet) != OK) {
        fprintf(stderr, "Failed to set active alphabet\n");
        return 1;
    }

    while (1) {
        showMainMenu(currentUsername, activeAlphabet.name);
        int choice = promptInt("Enter choice: ");

        if (choice == 0) {
            saveLogsToFile(logs, logCount, "data/sessions.txt");
            printf("Goodbye. Logs saved.\n");
            break;
        }

        if (choice == 1) {
            char text[256];
            promptString("Enter text to encode: ", text, sizeof(text));
            char morse[1024];
            if (encodeText(text, &activeAlphabet, morse, sizeof(morse)) == OK) {
                printf("Morse: %s\n", morse);
                SessionLog session;
                createSession(&session, currentUserID, "E");
                session.sessionID = nextSessionID++;
                safe_str_copy(session.inputText, sizeof(session.inputText), text);
                safe_str_copy(session.morseOutput, sizeof(session.morseOutput), morse);
                session.durationMs = 0;
                session.errorFlag = 0;
                logTranslation(logs, &logCount, 1000, &session);
            } else {
                printf("Encoding failed.\n");
            }
            continue;
        }

        if (choice == 2) {
            char morse[1024];
            promptString("Enter Morse code to decode: ", morse, sizeof(morse));
            char text[256];
            if (decodeMorse(morse, &activeAlphabet, text, sizeof(text)) == OK) {
                printf("Text: %s\n", text);
                SessionLog session;
                createSession(&session, currentUserID, "D");
                session.sessionID = nextSessionID++;
                safe_str_copy(session.inputText, sizeof(session.inputText), morse);
                safe_str_copy(session.morseOutput, sizeof(session.morseOutput), text);
                session.durationMs = 0;
                session.errorFlag = 0;
                logTranslation(logs, &logCount, 1000, &session);
            } else {
                printf("Decoding failed.\n");
            }
            continue;
        }

        if (choice == 3) {
            displaySessionHistory(logs, logCount, stdout);
            continue;
        }

        if (choice == 4) {
            while (1) {
                showUserMenu();
                int opt = promptInt("Choose user action: ");
                if (opt == 0) break;
                if (opt == 1) {
                    char username[50];
                    char pin[10];
                    promptString("New username: ", username, sizeof(username));
                    promptString("New PIN: ", pin, sizeof(pin));
                    if (registerUser(users, &userCount, 100, username, pin, NULL) == OK) {
                        printf("User registered: %s\n", username);
                    } else {
                        printf("Registration failed.\n");
                    }
                    continue;
                }
                if (opt == 2) {
                    char username[50];
                    char pin[10];
                    promptString("Username: ", username, sizeof(username));
                    promptString("PIN: ", pin, sizeof(pin));
                    if (loginUser(users, userCount, username, pin, &currentUser) == OK) {
                        currentUserID = currentUser.userID;
                        safe_str_copy(currentUsername, sizeof(currentUsername), username);
                        printf("Logged in as %s.\n", username);
                    } else {
                        printf("Login failed.\n");
                    }
                    continue;
                }
                if (opt == 3) {
                    if (currentUserID == 0) {
                        printf("No user is logged in.\n");
                    } else {
                        displayUserStats(&currentUser, stdout);
                    }
                    continue;
                }
                printf("Unknown selection.\n");
            }
            continue;
        }

        if (choice == 5) {
            while (1) {
                showAlphabetMenu();
                int opt = promptInt("Choose alphabet action: ");
                if (opt == 0) break;
                if (opt == 1) {
                    printf("Available alphabets:\n");
                    for (size_t i = 0; i < alphCount; ++i) {
                        printf(" %d: %s\n", alphabets[i].alphabetID, alphabets[i].name);
                    }
                    int id = promptInt("Enter alphabet ID: ");
                    if (switchAlphabet(alphabets, alphCount, id, &activeAlphabet) == OK) {
                        printf("Switched to %s.\n", activeAlphabet.name);
                    } else {
                        printf("Alphabet not found.\n");
                    }
                    continue;
                }
                if (opt == 2) {
                    displayAlphabetTable(&activeAlphabet, stdout);
                    continue;
                }
                if (opt == 3) {
                    char chBuf[4];
                    char morse[32];
                    promptString("Character to add: ", chBuf, sizeof(chBuf));
                    promptString("Morse code: ", morse, sizeof(morse));
                    if (chBuf[0] && addCustomEntry(&activeAlphabet, chBuf[0], morse) == OK) {
                        printf("Custom entry added.\n");
                    } else {
                        printf("Failed to add entry.\n");
                    }
                    continue;
                }
                if (opt == 4) {
                    char sample[256];
                    char missing[256];
                    promptString("Enter text to validate: ", sample, sizeof(sample));
                    validateAlphabetCoverage(&activeAlphabet, sample, missing, sizeof(missing));
                    if (missing[0]) {
                        printf("Missing characters: %s\n", missing);
                    } else {
                        printf("All characters are covered.\n");
                    }
                    continue;
                }
                printf("Unknown selection.\n");
            }
            continue;
        }

        if (choice == 6) {
            AnalyticsReport report;
            if (generateReport(logs, logCount, &report) == OK) {
                printReportSummary(&report, stdout);
                exportReportToFile(&report, "reports/latest_report.txt");
            } else {
                printf("Report generation failed.\n");
            }
            continue;
        }

        printf("Invalid selection.\n");
    }

    return 0;
}
