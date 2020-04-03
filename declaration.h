#ifndef DECLRATION_H
#define DECLRATION_H


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
volatile int* pixel_ctrl_ptr;
bool left_clicked;

#endif

