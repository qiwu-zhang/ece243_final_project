#include "declaration.h"

void draw_cursor(int x_cursor, int y_cursor, int colour, bool left_clicked){
   
      counting_down();
      draw_block(x_cursor, y_cursor, colour);
      wait_for_vsync(); // swap front and back buffers on VGA vertical sync
      pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

      if(!left_clicked){
        counting_down();
        draw_block(x_cursor, y_cursor, BLACK);
    clear_screen(1);
      }
}



void plot_pixel(int x, int y, short int line_color){   
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}



void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int *) 0xFF203020;  //pixel controller of front buffer address
    register int status; //the status register
    
    *pixel_ctrl_ptr = 1;    //start the synchronization procress by writing 
            // the value 1 into the front Buffer register in the pixel buffer controller
    
    status = *(pixel_ctrl_ptr + 3); // Wait for the S bit to become 0, that means VGA sweep is done
    //3*(4 bits) = 12 bits = 0xFF20302C
    while ((status & 0x01) != 0) { //wait until bit S of the Status register becomes equal to 0
        status = *(pixel_ctrl_ptr + 3);
    }
}


void draw_block(int x_start, int y_start, int colour){
    for(int x = x_start; x < x_start + 4; x++){
        for(int y = y_start; y < y_start + 4; y++){
            plot_pixel(x, y, colour);//show cursor
        }
      }
}

void clear_screen(bool clear_text_box) {
  if(!clear_text_box){ //clear the whole screen
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }
  }else{
    for (int x = 50; x < 250; x++) { 
        for (int y = 100; y < 200; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }
  }
  draw_colour_choice_and_brush_size();
  draw_line(50, 100, 50, 200, WHITE);  // vertical line from (100, 100) to (100, 200)
  draw_line(50, 100, 250, 100, WHITE); // horizontal line from (100, 100) to (200, 100)
  draw_line(250, 100, 250, 200, WHITE); // vertical line from (200, 100) to (200, 200)
  draw_line(50, 200, 250, 200, WHITE); // horizontal line from (200, 100) to (200, 200)
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

  
void draw_colour_choice_and_brush_size(){
    for(int x = 20; x < 28; x++){
        for(int y = 20; y < 28; y++){
            plot_pixel(x, y, PINK);//show cursor
        }
    }
  
  for(int x = 36; x < 44; x++){
        for(int y = 20; y < 28; y++){
            plot_pixel(x, y, BLUE);//show cursor
        }
    }
  
  for(int x = 52; x < 60; x++){
        for(int y = 20; y < 28; y++){
            plot_pixel(x, y, RED);//show cursor
        }
    }
  
   for(int x = 20; x < 24; x++){
        for(int y = 36; y < 40; y++){
            plot_pixel(x, y, WHITE);//show cursor
        }
    }
  
  for(int x = 32; x < 40; x++){
        for(int y = 36; y < 44; y++){
            plot_pixel(x, y, WHITE);//show cursor
        }
    }
  
  for(int x = 48; x < 60; x++){
        for(int y = 36; y < 48; y++){
            plot_pixel(x, y, WHITE);//show cursor
        }
    }
}


void draw_line(int x1, int y1, int x2, int y2, short int line_color) {
    
   bool is_steep = abs(y2 - y1) > abs(x2 - x1);
   if(is_steep) {
    swap(&x1, &y1);
    swap(&x2, &y2);

   }

   if(x1 > x2) {
    swap(&x1, &x2);
    swap(&y1, &y2);

   }

   int deltaX = x2 - x1;
   int deltaY = abs(y2 - y1);
    int error = -(deltaX/2);
    int y = y1;
    int y_step = 0;

    if(y1 <y2)
        y_step = 1;
    else
        y_step = -1;

    for(int x = x1; x < (x2+1) ; x++) {
        if(is_steep) 
            plot_pixel(y, x, line_color);
        else
            plot_pixel(x, y, line_color);

        error = error + deltaY;

        if(error >= 0) {
            y = y +y_step;
            error = error - deltaX;
        }
        
    }

}

void swap(int* x, int* y) {
  int temp = *x;
  *x = *y;
  *y = temp;
}


  
