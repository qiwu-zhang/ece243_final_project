#include "declaration.h"

void check_cursor_update_colour_size(int cursor_location[], int right_clicked) {
    if(cursor_location[0] >= 4 && cursor_location[0] <= 8 && cursor_location[1] >= 0 && cursor_location[1] <= 12 && right_clicked){ //if clicked on the pink
      cursor_colour = BLACK;
      printf("Changed colour");
    }else if(cursor_location[0] >= 20 && cursor_location[0] <= 24 && cursor_location[1] >= 0 && cursor_location[1] <= 12 && right_clicked){
      cursor_colour = BLUE;
      printf("Changed colour");
    }else if(cursor_location[0] >= 36 && cursor_location[0] <= 44 && cursor_location[1] >= 0 && cursor_location[1] <= 12 && right_clicked){
      cursor_colour = GREEN;
      printf("Changed colour");
    }else if(cursor_location[0] >= 52 && cursor_location[0] <= 60 && cursor_location[1] >= 0 && cursor_location[1] <= 12 && right_clicked){
      cursor_colour = PINK;
      printf("Changed colour");
    }else if(cursor_location[0] >= 68 && cursor_location[0] <= 76 && cursor_location[1] >= 0 && cursor_location[1] <= 12 && right_clicked){
      cursor_colour = RED;
      printf("Changed colour");
    }else if(cursor_location[0] >= 4 && cursor_location[0] <= 8 && cursor_location[1] >= 16 && cursor_location[1] <= 20 && right_clicked){
      cursor_size = 4;
    }else if(cursor_location[0] >= 16 && cursor_location[0] <= 24 && cursor_location[1] >= 16 && cursor_location[1] <= 24 && right_clicked){
      cursor_size = 8;
    }else if(cursor_location[0] >= 32 && cursor_location[0] <= 44 && cursor_location[1] >= 16 && cursor_location[1] <= 28 && right_clicked){
      cursor_size = 12;
    }else if(cursor_location[0] >= 52 && cursor_location[0] <=68 && cursor_location[1] >= 16 && cursor_location[1] <= 32 && right_clicked){
      cursor_size = 16;
    }else if(cursor_location[0] >= 76 && cursor_location[0] <= 96 && cursor_location[1] >= 16 && cursor_location[1] <= 36 && right_clicked){
      cursor_size = 20;
    }else if(cursor_location[0] >= 100 && cursor_location[0] <= 110 && cursor_location[1] >= 16 && cursor_location[1] <= 26 && right_clicked){
      ink_to_circle();
    } 
}

void check_left_click_update_mode(bool left_clicked) {

  if(left_clicked){
    if(draw_mode) {
      draw_mode = false;
    } else {
      draw_mode = true;
    }
    printf("Changed draw_mode to %i\n", draw_mode);
  }
}

void boundary_check(int cursor_location[]) {
    if(draw_mode) {
      if(cursor_location[0] <=30) {
        cursor_location[0] = 30;
      } else if ( cursor_location[0]  >= 300) {
        cursor_location[0] = 300;
      }

    if(cursor_location[1] <= 40) {
       cursor_location[1] = 40;
      } else if (cursor_location[1] >= 230) {
        cursor_location[1] = 230;
      }
    } else { //cursor can move whole VGA display
      if(cursor_location[0] <=0) {
        cursor_location[0] = 0;
      } else if ( cursor_location[0]  >= 310) {
        cursor_location[0] = 310;
      }

      if(cursor_location[1] <= 0) {
        cursor_location[1] = 0;
      } else if (cursor_location[1] >= 230) {
        cursor_location[1] = 230;
      }
    }
    
}
