#include <stdlib.h>
#include <stdbool.h>
#include "declaration.h"


int main(void){
  
  

  /*************************Initializing front/back pixel buffer ****************************/

  pixel_ctrl_ptr = (int *)0xFF203020; //pointing at the front buffer
  *(pixel_ctrl_ptr + 1) = 0xC8000000; //first store the address in the back buffer

  wait_for_vsync();
  //Initialize the base address to the first pixel in the front buffer(On-chip)
  pixel_buffer_start = *pixel_ctrl_ptr;
  
  //Initializing 320x240 pixels in the front buffer
  clear_screen(0); // pixel_buffer_start points to the pixel buffer
  
  *(pixel_ctrl_ptr + 1) = 0xC0000000;//set back pixel buffer to start of SDRAM memory
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

  //Initializing 320x240 pixels in the back buffer(SDRAM)
  clear_screen(0);

  /*************************Initializing front/back pixel buffer End****************************/


  //Global signal indicating if the left button on the mouse is pressed
  left_clicked = 0;
  int cursor_colour = WHITE;
    
  while(true){
  //@ Need a array to track the previous location(x,y) of the cursor and erase by drawing black on previous 
  clear_screen(1); 

    for(int x = 50; x<249;x++){
      for(int y = 100; y<199; y++){
        draw_cursor(x, y, cursor_colour, left_clicked);
      }
    }
  
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
    
  }
}
