#include <stdlib.h>
#include <stdbool.h>
#define WHITE 0xFFFF
#define BLACK 0x0000

void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void draw_cursor(int x_cursor, int y_cursor, int colour);
void wait_for_timer_interrupt();

volatile int pixel_buffer_start; // global variable
volatile int* PRIVATE_TIMER_PTR = (int*)0xFFFEC600; //set up a pointer to A9 private timer


int main(void){
  volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();
  int cursor_colour = WHITE;
  draw_cursor(158, 118, cursor_colour);
}
