#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    int complement = 0xFFFFFE00 + 0b100000110; //-250 for 9-bit 2's complement IMPORTANT: sign extension
    printf("the 0b1111111100000110 as a short with sign extension is: %i\n", complement);
    
    int pos_result = ~complement +1;
    printf("the positive 2's complement is: %i", pos_result);
}


convert_to_