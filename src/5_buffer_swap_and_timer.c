#include "declaration.h"

void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int *) 0xFF203020;  //pixel controller of front buffer address
    register int status; //the status register
    
    *pixel_ctrl_ptr = 1;    //start the synchronization procress by writing 
            // the value 1 into the front Buffer register in the pixel buffer controller
    
    status = *(pixel_ctrl_ptr + 3); // Wait for the S bit to become 0, that means VGA sweep is done
    //3*(4 bits) = 12 bits = 0xFF20302C

    //Polling to get status bit
    while ((status & 0x01) != 0) { //wait until bit S of the Status register becomes equal to 0
        status = *(pixel_ctrl_ptr + 3);
    }
}

void counting_down(){
    int status;
    *PRIVATE_TIMER_PTR = 100000000;//load the 200MHz timer to have 0.5s interval
    *(PRIVATE_TIMER_PTR + 2) = 0b011; //Enable is set to 1, load to 1 at the spot of (timer + 8 bits)
    
    while( *(PRIVATE_TIMER_PTR + 3) == 0){ //polling......
        status = *(PRIVATE_TIMER_PTR + 3); //doing nothing
    } // signal registered again
    
    *(PRIVATE_TIMER_PTR + 3) = 0; //reset interrupt signal
}