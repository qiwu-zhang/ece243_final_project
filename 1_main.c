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
  load_screen(); // pixel_buffer_start points to the pixel buffer

  
  *(pixel_ctrl_ptr + 1) = 0xC0000000;//set back pixel buffer to start of SDRAM memory
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

  //Initializing 320x240 pixels in the back buffer(SDRAM)
  load_screen();

  /*************************Initializing front/back pixel buffer End****************************/


  //Global signal indicating if the left button on the mouse is pressed
  left_clicked = 0;
  int cursor_colour = WHITE; //initialize to white brush
  int cursor_size = 4;

  //An array that hold cursor's x/y location in the VGA display
  int cursor_location[2] = {150, 150};

    
  while(true){
    //@ Need a array to track the previous location(x,y) of the cursor and erase by drawing black on previous
    clear_screen(1); 

    mouse_movement movement = get_mouse_movement();

    // Update cursor_location and draw
    cursor_location[0] = cursor_location[0] + movement.dx;
    cursor_location[1] = cursor_location[1] + movement.dy;

    if(cursor_location[0] >= 20 && cursor_location[0] <= 28 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){ //if clicked on the pink
      cursor_colour = PINK;
    }else if(cursor_location[0] >= 36 && cursor_location[0] <= 44 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){
      cursor_colour = BLUE;
    }else if(cursor_location[0] >= 52 && cursor_location[0] <= 60 && cursor_location[1] >= 20 && cursor_location[1] <= 28 && left_clicked){
      cursor_colour = WHITE;
    }else if(cursor_location[0] >= 20 && cursor_location[0] <= 24 && cursor_location[1] >= 36 && cursor_location[1] <= 40 && left_clicked){
      cursor_size = 4;
    }else if(cursor_location[0] >= 32 && cursor_location[0] <= 40 && cursor_location[1] >= 36 && cursor_location[1] <= 44 && left_clicked){
      cursor_size = 8;
    }else if(cursor_location[0] >= 48 && cursor_location[0] <= 60 && cursor_location[1] >= 36 && cursor_location[1] <= 48 && left_clicked){
      cursor_size = 12;
    }

    draw_cursor(cursor_location[0], cursor_location[1], cursor_colour, cursor_size, left_clicked);
  
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
    
  }
}
