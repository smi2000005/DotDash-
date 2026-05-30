#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/types.h"
#include "../include/errors.h"
#include "../src/utils.h"
#include "../src/m1_encoder.h"
#include "../src/m2_logger.h"
#include "../src/m3_user.h"
#include "../src/m4_alphabet.h"
#include "../src/m5_analytics.h"

static void assertTrue(int condition, const char *msg) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", msg);
        exit(1);
    }
}

static void assertStringEquals(const char *a, const char *b, const char *msg) {
    assertTrue(strcmp(a, b) == 0, msg);
}

int main(void) {
    AlphabetSet alphabets[8]; size_t alphCount = 0;
    assertTrue(initAlphabets(alphabets, &alphCount, 8, "data/morse_table.txt") == OK, "initAlphabets");
    assertTrue(alphCount >= 1, "alphabet count");

    char encoded[1024];
    assertTrue(encodeText("AB", &alphabets[0], encoded, sizeof(encoded)) == OK, "encode AB");
    assertTrue(strlen(encoded) > 0, "encoded output nonempty");

    char decoded[256];
    assertTrue(decodeMorse(encoded, &alphabets[0], decoded, sizeof(decoded)) == OK, "decode AB");
    assertStringEquals(decoded, "AB", "decoded matches AB");

    assertTrue(validateMorseInput(encoded) == 1, "validate morse input");
    assertTrue(validateMorseInput("... --- ...") == 1, "validate SOS");
    assertTrue(validateMorseInput("...x---") == 0, "reject invalid morse");

    assertTrue(addCustomEntry(&alphabets[0], '@', ".--.-.") == OK, "add custom entry");
    assertTrue(encodeText("@", &alphabets[0], encoded, sizeof(encoded)) == OK, "encode custom char");
    assertTrue(decodeMorse(encoded, &alphabets[0], decoded, sizeof(decoded)) == OK, "decode custom char");
    assertStringEquals(decoded, "@", "custom entry decode");

    char missing[256];
    assertTrue(validateAlphabetCoverage(&alphabets[0], "ABC#", missing, sizeof(missing)) == OK, "validate coverage");
    assertTrue(strlen(missing) > 0, "missing characters detected");
    assertTrue(strchr(missing, '#') != NULL, "missing includes #");

    UserProfile users[10]; size_t userCount = 0;
    assertTrue(registerUser(users, &userCount, 10, "bob", "5678", NULL) == OK, "register bob");
    assertTrue(userCount == 1, "user count");
    int bobIndex = -1;
    assertTrue(searchUserByName(users, userCount, "bob", &bobIndex) == OK, "find registered user");
    assertTrue(bobIndex == 0, "search returns correct index");
    assertTrue(loginUser(users, userCount, "bob", "5678", NULL) == OK, "login bob");
    assertTrue(updateUserStats(&users[0], 'E') == OK, "update user encode stats");
    assertTrue(users[0].totalEncodes == 1, "user encode count");

    SessionLog logs[10]; size_t logCount = 0;
    SessionLog session;
    createSession(&session, 1, "E");
    session.sessionID = 1;
    safe_str_copy(session.inputText, sizeof(session.inputText), "HELLO");
    safe_str_copy(session.morseOutput, sizeof(session.morseOutput), ".... . .-.. .-.. ---");
    logTranslation(logs, &logCount, 10, &session);
    assertTrue(logCount == 1, "log count after add");

    const char *savePath = "tests/test_logs.txt";
    assertTrue(saveLogsToFile(logs, logCount, savePath) == OK, "save logs to file");

    SessionLog loaded[10]; size_t loadedCount = 0;
    assertTrue(loadLogsFromFile(loaded, &loadedCount, 10, savePath) == OK, "load logs from file");
    assertTrue(loadedCount == 1, "loaded log count");
    assertStringEquals(loaded[0].inputText, "HELLO", "loaded log input text");
    remove(savePath);

    AnalyticsReport report;
    assertTrue(generateReport(logs, logCount, &report) == OK, "generateReport");
    assertTrue(report.totalSessions == 1, "report session count");
    assertTrue(report.totalErrors == 0, "report error count");

    SessionLog found[10]; size_t foundCount = 0;
    assertTrue(searchLogByKeyword(logs, logCount, "HELLO", found, &foundCount, 10) == OK, "search log by keyword");
    assertTrue(foundCount == 1, "keyword search count");

    printf("All tests passed.\n");
    return 0;
}
