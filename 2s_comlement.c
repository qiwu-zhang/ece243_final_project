#include <stdio.h>
#include <stdlib.h>

int main() {
    char 2complement = 0b100000110;
    if((2complemnt & 0x100)) { //i.e. negtive num
        2complement = ~2complement + 1; // flip it
    }

    printf("the positive 2's complement is: %i", int(2complement));
}
