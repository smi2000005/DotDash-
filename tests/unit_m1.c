#include <stdio.h>
#include "../src/m1_encoder.h"

int main(void) {
    int ok = validateMorseInput("... --- ...");
    printf("validateMorseInput returned %d\n", ok);
    return 0;
}
