#include <stdlib.h>
#include <stdbool.h>
#include "declaration.h"


int main(void)
{

  /*************************Initializing front/back pixel buffer ****************************/

  pixel_ctrl_ptr = (int *)0xFF203020; //pointing at the front buffer
  *(pixel_ctrl_ptr + 1) = 0xC8000000; //first store the address in the back buffer
  wait_for_vsync();

  //Initialize the base address to the first pixel in the front buffer(On-chip)
  pixel_buffer_start = *pixel_ctrl_ptr;

  //Initializing 320x240 pixels in the front buffer
  load_screen(); // pixel_buffer_start points to the pixel buffer

  *(pixel_ctrl_ptr + 1) = 0xC0000000; //set back pixel buffer to start of SDRAM memory
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

  //Initializing 320x240 pixels in the back buffer(SDRAM)
  load_screen();

  /*************************Initializing front/back pixel buffer End****************************/

  while (true) {
    // Read from KEY port to see if ANY KEY is pressed
    reset = *RESET_BUTTON_PTR;

    if (reset != 0)
    { //if a KEY is pressed
      printf("a button is pressed, resetting...\n");
      //Reset screen of current back buffer
      load_screen();
      //Swap front to back
      wait_for_vsync();
      pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
      //Reset screen of swaped front buffer (both buffer is reset now)
      load_screen();
      //Reset canvass 2d array to all ZERO
      reset_canvass_to_zero();

      //reset the edge capture
      *RESET_BUTTON_PTR = 0b1111;
      continue;
    }

    // Getting mouse_movement struct and data
    mouse_movement movement = get_mouse_movement();
    bool left_clicked = movement.left_pressed_bit;
    bool right_clicked = movement.right_pressed_bit;

    //@ Erasing - by drawing WHITE on the last last previous location (); set left click to 0 to avoid write 1 to canavas
    if (!draw_mode)
    { // If last cylcle not drawing
      draw_cursor(prev_buffer_cursor_location[0], prev_buffer_cursor_location[1], WHITE, cursor_size, 0);
    } else { // Last cycle drawing
      draw_cursor(prev_buffer_cursor_location[0], prev_buffer_cursor_location[1], cursor_colour, cursor_size, 1);
    }

    //@ Redraw to refresh after erasing white avoid drawing white on brush icons and brush size blocks
    redraw_icon_and_box();

    //@ Updating cursor colour/size and boundary check for cursor movement

    // Store cursor location (the location previously used to draw in the back buffer - now it's front) to previous location before add mouse movement
    // The prev_location will be used in next cycle to erase back buffer - now it's front
    prev_buffer_cursor_location[0] = cursor_location[0];
    prev_buffer_cursor_location[1] = cursor_location[1];

    // Update cursor_location (location used to draw on the current back buffer)
    cursor_location[0] = cursor_location[0] + movement.dx;
    cursor_location[1] = cursor_location[1] + movement.dy;

    //Cursor status checking (left|right click) - Update cursor size and colour based on cursor location and its clicking
    check_cursor_update_colour_size(cursor_location, right_clicked);
    check_left_click_update_mode(left_clicked);

    //Boundary checking - If left clicking drawing - can only move within the textbox (correct cursor location if exceeds the bounddary)

    boundary_check(cursor_location);

    // Add a delay of 0.5 second

    counting_down();

    //@ Drawing cursor on updated location (on the back buffer)

    draw_cursor(cursor_location[0], cursor_location[1], cursor_colour, cursor_size, draw_mode);


    wait_for_vsync(); // swap front and back buffers on VGA vertical sync i.e. display drawed back buffer
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
  }

  return 0;
}
