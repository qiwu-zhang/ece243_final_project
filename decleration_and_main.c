#include <stdlib.h>
#include <stdbool.h>
#define WHITE 0xFFFF
#define BLACK 0x0000
#define PINK  0xF81F
#define BLUE  0x001F
#define RED   0xF800

void clear_screen(bool clear_text_box);
void plot_pixel(int x, int y, short int line_color);
void draw_cursor(int x_cursor, int y_cursor, int colour, bool left_clicked);
void wait_for_timer_interrupt();
void wait_for_vsync();
void counting_down();
void draw_block(int x_start, int y_start, int colour);
void draw_colour_choice_and_brush_size();

volatile int pixel_buffer_start; // global variable
volatile int* PRIVATE_TIMER_PTR = (int*)0xFFFEC600; //set up a pointer to A9 private timer
volatile int * pixel_ctrl_ptr;
bool left_clicked;


int main(void){
  
   pixel_ctrl_ptr = (int *)0xFF203020; //pointing at the front buffer
  *(pixel_ctrl_ptr + 1) = 0xC8000000; //first store the address in the back buffer

  wait_for_vsync();
  pixel_buffer_start = *pixel_ctrl_ptr;
  
  clear_screen(0); // pixel_buffer_start points to the pixel buffer
  
  *(pixel_ctrl_ptr + 1) = 0xC0000000;//set back pixel buffer to start of SDRAM memory
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

  bool left_clicked = 0;
  int cursor_colour = WHITE;
    
  while(true){
  clear_screen(1); 
    for(int x = 50; x<249;x++){
      for(int y = 100; y<199; y++){
        draw_cursor(x, y, cursor_colour, left_clicked);
      }
    }
  
    wait_for_vsync(); // swap front and back buffers on VGA vertical sync
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    
  }
}
