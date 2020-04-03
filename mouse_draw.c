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

/* Connect mouse with the cursor */ 
void mouse_draw(int initialX, int initialY) {
    unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;

    volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address

    int PS2_data, RVALID;

    while(1) {
        PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0)
		{
			/* always save the last three bytes received */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
		}
		
    }

}