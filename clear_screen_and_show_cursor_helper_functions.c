void clear_screen() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {
            plot_pixel(x, y, BLACK);//Plotting black pixel all over the VGA display
        }
    }
}


void plot_pixel(int x, int y, short int line_color){   
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void draw_cursor(int x_cursor, int y_cursor, int colour){
  int status;
    *PRIVATE_TIMER_PTR = 10000000000;//load the 200MHz timer to have 0.5s interval
    *(PRIVATE_TIMER_PTR + 2) = 0b011; //Enable is set to 1, load to 1 at the spot of (timer + 8 bits)
  
  while(true){
    if( *(PRIVATE_TIMER_PTR + 3) == 1){ //if a signal is registered
      for(int x = x_cursor; x < x_cursor + 4; x++){
        for(int y = y_cursor; y < y_cursor + 4; y++){
          plot_pixel(x, y, WHITE);//show cursor
        }
       }
      *(PRIVATE_TIMER_PTR + 3) = 0; //reset interrupt signal
      while( *(PRIVATE_TIMER_PTR + 3) == 0){ //polling......
        status = *(PRIVATE_TIMER_PTR + 3); //doing nothing
      } // signal registered again
      clear_screen();
    }
  }
}