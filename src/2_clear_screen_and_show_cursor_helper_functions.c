#include "declaration.h"

void draw_cursor(int x_cursor, int y_cursor, int colour, int size, bool draw_mode){

      draw_block(x_cursor, y_cursor, colour, size);
      if(draw_mode) {
        canvas[x_cursor][y_cursor] = 1;
        printf("Writing into canvas\n");
      }
}



void plot_pixel(int x, int y, short int line_color){   
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}


void draw_block(int x_start, int y_start, int colour, int size){
    for(int x = x_start; x < x_start + size; x++){
        for(int y = y_start; y < y_start + size; y++){
            plot_pixel(x, y, colour);//show cursor
        }
      }
}

void load_screen() {
    //Redraw whole 320x240 VGA screen to WHITE
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, WHITE);//Plotting black pixel all over the VGA display
        }
    }

    //Reset cursor location and previous cursor location
    cursor_location[0] = 150;
    cursor_location[1] = 150;
    prev_buffer_cursor_location[0] = 150;
    prev_buffer_cursor_location[1] = 150;
    

    draw_colour_choice_and_brush_size();
    draw_line(30, 40, 30, 230, BLACK);  // vertical line from (30, 40) to (30, 230)
    draw_line(30, 40, 300, 40, BLACK); // hoizontal line from (30, 40) to (300, 40)
    draw_line(300, 40, 300, 230, BLACK); // vertical line from (300, 40) to (300, 230)
    draw_line(30, 230, 300, 230, BLACK); // horizontal line from (30, 230) to (300, 230)
}


void redraw_icon_and_box() {
    draw_colour_choice_and_brush_size();
    draw_line(30, 40, 30, 230, BLACK);  // vertical line from (30, 40) to (30, 230)
    draw_line(30, 40, 300, 40, BLACK); // hoizontal line from (30, 40) to (300, 40)
    draw_line(300, 40, 300, 230, BLACK); // vertical line from (300, 40) to (300, 230)
    draw_line(30, 230, 300, 230, BLACK); // horizontal line from (30, 230) to (300, 230)
}

void reset_canvass_to_zero() {
    for(int i = 0; i < 240 ; i++) {
        for(int j = 0; j < 320; j++) {
            canvas[i][j] = 0;
        }
    }
}


void clear_screen(bool clear_text_box) {
  if(!clear_text_box){ //clear the whole screen
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, WHITE);//Plotting black pixel all over the VGA display
        }
    }
  }else{
    for (int x = 31; x < 300; x++) { 
        for (int y = 41; y < 230; y++) {
            plot_pixel(x, y, WHITE);//Plotting black pixel all over the VGA display
        }
    }
  }
}

void draw_colour_choice_and_brush_size() {
  draw_black_brush();
  draw_blue_brush();
  draw_green_brush();
  draw_pink_brush();
  draw_red_brush();
  draw_block(4, 16, cursor_colour, 4);
  draw_block(16, 16, cursor_colour, 8);
  draw_block(32, 16, cursor_colour, 12);
  draw_block(52, 16, cursor_colour, 16);
  draw_block(76, 16, cursor_colour, 20);
  //Ink to shape button
  draw_block(100, 16, GREEN, 10);

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

