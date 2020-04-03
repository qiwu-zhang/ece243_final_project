/**
 * # This code does the following things:
 * 1) Infinite loop  until a DE1-SoC KEY (e.g. KEY[0]) is pressed
 * 2) While looping, read mouse movement erase/ draw on back buffer
 * 3) Swap back to front to refresh VGA screen 
 * 
 * * Note: Should have some mechanism to check if cursor have reached one of the boudaries (top, bottom, left, right)
 * 
 * Addition: 4) Create interrupt when left mouse button is clicked 
 * 
 */

#include "declaration.h"


mouse_movement get_mouse_movement() {
	//Declare return struct variable
	mouse_movement movement;

	// Up counter resets upon counts to 3 to retrieve one 3-byte movement packet
	int counter = 1;
	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;

	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address

    int PS2_data, RVALID;

    while(1) {
        PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field (15th bit)
		if (RVALID && counter == 1) {
			// &0xFF: Only take the first 8 bits - data
			byte1 = PS2_data & 0xFF; 
		} else if (RVALID && counter == 2) {
			byte2 = PS2_data & 0xFF;
		} else if (RVALID && counter == 3) {
			byte3 = PS2_data & 0xFF;
		}

		printf("x_movement byte (byte 2) is %u \n", (unsigned int) byte2);
		printf("y_movement byte (byte 3) is %u \n", (unsigned int) byte3);

		if (counter ==3) { // One 3-byte movement packet has been stored in byte1/2/3, returning...
			//Get dx/dy movement
			int x_sign_bit = byte1 & 0x10; //0b00010000
			int y_sign_bit = byte1 & 0x20; //0b00100000
			int left_pressed_bit = byte1 & 0x01;
			
			if(x_sign_bit == 0) { //positive dx
				movement.dx = (int) byte2;
			} else { //negative dx
				movement.dx = -(int) byte2;
			}

			if(y_sign_bit == 0) { //positive dx
				movement.dy = (int) byte3;
			} else { //negative dx
				movement.dy = -(int) byte3;
			}

			movement.left_pressed_bit = left_pressed_bit;
			printf("the left-pressed bit is %i \n", left_pressed_bit);

			return movement;

		} else {
			counter ++;
		}
		
    }

	printf("Returning empty mouse movement - no movement \n");
	return movement;

}