/**************************************************************************************************
 * ECE243 Final Project: VGA Drawing Board with "Ink to Shape" Feature
 * TA: Ciaran B. 
 * Team Member: Yixin T., Qiwu Z.
 * 
 * ************************************************************************************************
 * Description: 
 * 
 * The application is wrote in C and is designed to be run on ARM DE1-SoC computer. The application allows user to
 * draw on VGA real-time with a PS/2 Mouse by polling 3-byte PS/2 movement packet(2's complement mouse movement, left/right click 
 * bit) from the PS/2 port on DE1-SoC Board. The extra feature being added now is "ink to circle" which can later be extended to 
 * more shapes.
 * 
 * ***********************************************************************************************
 * Interface:
 * 
 * 1) Left click - toggle draw_mode on/off (this sometimes need accurate click rate as the read speed from PS/2 port is 4 times/s)
 * 2) Right click - select brush colour/size 
 * 3) Brush Colour - located at the top left of the VGA display 
 * 4) Brush Size - locatied below the brush colour
 * 5) "Ink to Circle" - the green button on the right of the brush size
 * 6) KEY[3-0] - Reset cursor and clear the canvas
 * 
 * ************************************************************************************************
 * An Example Showing How to Use: 
 * 
 * 1) Open and compile the code on CPULATOR: https://cpulator.01xz.net/?sys=arm-de1soc
 * 2) Click on the small drop-down menu on the top left of the PS/2 Keyboard or Mouse panel and select "Mouse" in "Mode" 
 * 3) Run the compiled code and locatae one of the two PS/2 Keyboard or Mouse panels on the left 
 * 4) If want to move mouse click "start" and move slowly (otherwise may cause 64-byte FIFO overflow), click "Esc" to pause
 * 5) click left button once to toggle into "draw mode" and click one more time to switch back 
 * Note: if it's not working properly, exit capture mode and use "send packet 3-byte" to send left/right click signal
 * 
 * 6) Draw something using different brushes! (Use right click for selecting)
 * 
 * 7) After done drawing click "ink-to-circle" button (green) - this only supports when there is only a single enclosed contour
 * on the VGA display
 * 
 * 8) Press any KEY to reset  
 * 
 * ************************************************************************************************
 * Troubleshooting: 
 * 
 * 1) if get "CPU tried writing to address ..." error afte right click on "ink-to-circle" button, that means it's reading right  
 * click multiple times  - use "send packet - 3-byte" in "PS/2 Keyboard or Mouse" panel as your right click
 * 
 * *************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define PINK  0xF81F
#define BLUE  0x001F
#define RED   0xF800
#define GREEN 0x07E0 


//PS/2 mouse send movement/button information to the host using the 3-byte movementpacket
//This struct is used to store the key information we need from the 3-byte movement packet
typedef struct {
    int dx;
    int dy;
    int left_pressed_bit;
    int right_pressed_bit;
}mouse_movement;

typedef struct {
  int x0;
  int y0;
  int radius;
} centriod;


void load_screen();
void redraw_icon_and_box();
void reset_canvass_to_zero();
void clear_screen(bool clear_text_box);
void plot_pixel(int x, int y, short int line_color);
void check_cursor_update_colour_size(int cursor_location[], int right_clicked);
void check_left_click_update_mode(bool left_clicked);
void boundary_check(int cursor_location[]);
void draw_cursor(int x_cursor, int y_cursor, int colour,int cursor_size, bool left_clicked);
void wait_for_timer_interrupt();
void wait_for_vsync();
void counting_down();
void draw_block(int x_start, int y_start, int colour, int size);
void draw_colour_choice_and_brush_size();
mouse_movement get_mouse_movement();


//Functions for ink to shape algorithm 
void ink_to_circle();
centriod find_centriod();
void midpoint_algorithm_draw_circle(int x0, int y0, int radius);
// void midpoint_algorithm_drawsquare(int x0, int y0, int side_Length);


/***********************************Global******************************************/
volatile int pixel_buffer_start; // global variable
volatile int* PRIVATE_TIMER_PTR = (int*)0xFFFEC600; //set up a pointer to A9 private timer
volatile int* RESET_BUTTON_PTR = (int*)0xFF20005C; //set up a pointer to buttons
volatile int* pixel_ctrl_ptr;
bool left_clicked;

int cursor_colour;
// Draw mode activated by left click once 
bool draw_mode = false;
int reset;


//2-D Arrays keep track of pixel been drawed (0/1)
/* 2D array declaration using initializing list*/
int canvas[240][320] = {0};



//Variable holds brush size and it's colour, update in the while loop accordingly left click position 
int cursor_colour = BLACK; //initialize to white brush
int cursor_size = 4;

//Arrays that hold cursor's x/y location in the VGA display
int cursor_location[2] = {150, 150};
int prev_buffer_cursor_location[2] = {150, 150};
/***********************************Global End******************************************/

 
int black_brush[192] = {
  /*Pixel format: Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x79, 0xce, 0x00, 0x00, 0x00, 0x00, 0xcb, 0x5a, 0xae, 0x73, 0x00, 0x00, 0x00, 0x00, 0xec, 0x62, 
  0xfc, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x63, 
  0xf7, 0xbd, 0x9e, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x92, 0x94, 
  0x7d, 0xef, 0x72, 0x94, 0xbe, 0xf7, 0x9e, 0xf7, 0xff, 0xff, 0x1c, 0xe7, 0xff, 0xff, 0x51, 0x8c, 
  0xff, 0xff, 0x0c, 0x63, 0xff, 0xff, 0x9e, 0xf7, 0xff, 0xff, 0x3d, 0xef, 0xfb, 0xde, 0x35, 0xad, 
  0xff, 0xff, 0xcb, 0x5a, 0xff, 0xff, 0x9e, 0xf7, 0xff, 0xff, 0xdf, 0xff, 0x8e, 0x73, 0xff, 0xff, 
  0xff, 0xff, 0xec, 0x62, 0xff, 0xff, 0x9e, 0xf7, 0xff, 0xff, 0xff, 0xff, 0x8a, 0x52, 0xff, 0xff, 
  0xff, 0xff, 0xbe, 0xf7, 0xcf, 0x7b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xab, 0x5a, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc7, 0x39, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7b, 0xbf, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x1c, 0xe7, 0x00, 0x00, 0x61, 0x08, 0x28, 0x42, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x39, 0xce, 0x41, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int blue_brush[] ={
  0x6e, 0x1a, 0x79, 0x03, 0x78, 0x03, 0x3b, 0x65, 0xfb, 0x5c, 0x78, 0x03, 0x79, 0x03, 0x6e, 0x22, 
  0x2c, 0x22, 0x33, 0x02, 0x32, 0x02, 0xb4, 0x3b, 0x74, 0x33, 0x32, 0x02, 0x33, 0x02, 0x4d, 0x2a, 
  0xef, 0x83, 0xda, 0xe6, 0x79, 0xde, 0xfb, 0xee, 0xfb, 0xe6, 0x79, 0xde, 0xda, 0xe6, 0xef, 0x83, 
  0x31, 0x8c, 0xbf, 0xff, 0x5d, 0xef, 0xff, 0xff, 0xdf, 0xff, 0x5d, 0xef, 0xbf, 0xff, 0x51, 0x8c, 
  0x71, 0x8c, 0x9e, 0xf7, 0x3c, 0xe7, 0xdf, 0xff, 0xbf, 0xff, 0x3c, 0xe7, 0x7e, 0xf7, 0x72, 0x94, 
  0xdb, 0xde, 0xf4, 0xa4, 0x7d, 0xef, 0xdf, 0xff, 0xbf, 0xff, 0x7d, 0xef, 0xd3, 0x9c, 0x1c, 0xe7, 
  0xff, 0xff, 0x4d, 0x6b, 0x9e, 0xf7, 0xdf, 0xff, 0xbf, 0xff, 0x9e, 0xf7, 0x4d, 0x6b, 0xff, 0xff, 
  0xff, 0xff, 0xcf, 0x7b, 0x7e, 0xf7, 0xdf, 0xff, 0xdf, 0xff, 0x5d, 0xef, 0xef, 0x7b, 0xff, 0xff, 
  0xff, 0xff, 0xbf, 0xff, 0xcf, 0x7b, 0xff, 0xff, 0xff, 0xff, 0xae, 0x7b, 0xdf, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x51, 0x94, 0x0b, 0x01, 0x0b, 0x01, 0x71, 0x94, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x4c, 0x01, 0x4c, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x96, 0xb5, 0xb7, 0xb5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int green_brush[] = {
  0x49, 0x23, 0x8b, 0x05, 0x6b, 0x05, 0x73, 0x66, 0x31, 0x4e, 0x4a, 0x05, 0x6a, 0x05, 0xee, 0x5b, 
  0xc9, 0x42, 0x6a, 0x33, 0x6a, 0x33, 0xed, 0x5b, 0xac, 0x4b, 0x49, 0x33, 0x49, 0x2b, 0xcf, 0x73, 
  0x72, 0x94, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0x72, 0x94, 
  0x51, 0x8c, 0x9e, 0xf7, 0x3d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x3d, 0xef, 0x3d, 0xef, 0xd3, 0x9c, 
  0xd3, 0x9c, 0x3d, 0xef, 0x3d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x5d, 0xef, 0x55, 0xad, 0xbb, 0xde, 
  0x3c, 0xe7, 0xb2, 0x94, 0x7d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x9e, 0xf7, 0x8e, 0x73, 0xff, 0xff, 
  0xff, 0xff, 0x8e, 0x73, 0x9e, 0xf7, 0xdf, 0xff, 0xbe, 0xf7, 0xbe, 0xf7, 0x8e, 0x73, 0xff, 0xff, 
  0xff, 0xff, 0xf0, 0x83, 0x7e, 0xf7, 0xff, 0xff, 0xdf, 0xff, 0xf8, 0xc5, 0x96, 0xb5, 0xff, 0xff, 
  0xff, 0xff, 0x5d, 0xef, 0x51, 0x8c, 0xff, 0xff, 0xff, 0xff, 0x2c, 0x63, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x8e, 0x73, 0x83, 0x02, 0x80, 0x01, 0xd7, 0xbd, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0x04, 0x03, 0xa5, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2f, 0x74, 0x96, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int pink_brush[] = {
  0xeb, 0x80, 0x10, 0xe0, 0x50, 0xd8, 0x36, 0xeb, 0x75, 0xea, 0x0f, 0xd8, 0x0f, 0xe0, 0xef, 0x8a, 
  0x69, 0x59, 0x08, 0x78, 0x08, 0x78, 0xac, 0x81, 0x4b, 0x81, 0x08, 0x78, 0x08, 0x78, 0x4e, 0x7b, 
  0xb2, 0x8c, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0x92, 0x94, 
  0x31, 0x8c, 0x9e, 0xf7, 0x3d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x3d, 0xef, 0x7d, 0xef, 0xb2, 0x94, 
  0x92, 0x94, 0x7e, 0xf7, 0x3d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x5d, 0xef, 0xd7, 0xbd, 0x59, 0xce, 
  0xba, 0xd6, 0x35, 0xad, 0x7d, 0xef, 0xdf, 0xff, 0xbe, 0xf7, 0x9e, 0xf7, 0xae, 0x73, 0xff, 0xff, 
  0xff, 0xff, 0x8e, 0x73, 0x9e, 0xf7, 0xdf, 0xff, 0xbe, 0xf7, 0x9e, 0xf7, 0x8e, 0x73, 0xff, 0xff, 
  0xff, 0xff, 0xaf, 0x7b, 0x9e, 0xf7, 0xff, 0xff, 0xdf, 0xff, 0x9a, 0xd6, 0xf3, 0x9c, 0xff, 0xff, 
  0xff, 0xff, 0xbb, 0xde, 0x14, 0xa5, 0xff, 0xff, 0xff, 0xff, 0x6e, 0x73, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x0c, 0x63, 0xea, 0x80, 0xa8, 0x60, 0xf3, 0x9c, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x9d, 0xe7, 0x08, 0x98, 0x07, 0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xeb, 0x71, 0x52, 0x9c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int red_brush[] = {
  0x22, 0x90, 0x23, 0xf8, 0x06, 0xf1, 0x90, 0xfb, 0x0a, 0xfa, 0x03, 0xf0, 0x02, 0xf8, 0xb3, 0x94, 
  0xa7, 0x49, 0x0c, 0x8b, 0x0c, 0x83, 0xcf, 0x8b, 0x4d, 0x8b, 0xec, 0x82, 0xab, 0x82, 0x14, 0x9d, 
  0x71, 0x8c, 0xff, 0xff, 0xdf, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xf7, 0x3d, 0xe7, 0x55, 0xad, 
  0x30, 0x84, 0x9e, 0xf7, 0x5d, 0xef, 0xff, 0xff, 0x7e, 0xf7, 0x7d, 0xef, 0x35, 0xad, 0xdb, 0xde, 
  0x51, 0x8c, 0xbe, 0xf7, 0x5d, 0xef, 0xff, 0xff, 0x7e, 0xf7, 0x9e, 0xf7, 0xaf, 0x7b, 0xff, 0xff, 
  0x75, 0xad, 0x7a, 0xd6, 0x5d, 0xef, 0xff, 0xff, 0x7e, 0xf7, 0x9e, 0xf7, 0xaf, 0x7b, 0xff, 0xff, 
  0xff, 0xff, 0xaf, 0x7b, 0x9e, 0xf7, 0xff, 0xff, 0x9e, 0xf7, 0x1c, 0xe7, 0x51, 0x8c, 0xff, 0xff, 
  0xff, 0xff, 0x8e, 0x73, 0xbf, 0xff, 0xff, 0xff, 0xdf, 0xff, 0x10, 0x84, 0xbf, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xb6, 0xb5, 0x18, 0xc6, 0xff, 0xff, 0xff, 0xff, 0x2d, 0x6b, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x86, 0x41, 0x00, 0x90, 0x00, 0x30, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x79, 0xc6, 0x00, 0xc8, 0x29, 0x7a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x09, 0x6a, 0x1c, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  

 void draw_black_brush(){
  int i = 0, j = 0;
  for(int k = 0; k<8*12*2 - 1; k+=2){
    int red = ((black_brush[k+1]) & 0xF8 >> 3) <<11;
    int green = (((black_brush[k] & 0xE0) >>5)) | ((black_brush[k+1] & 0x7) << 3);
    int blue = (black_brush[k] & 0x1F);
    short int p = red | ((green <<  5) | blue);
    plot_pixel(4+i, j, p);
    i+=1;
    if(i == 8){
      i = 0;
      j+=1;
    }
  }
}

void draw_blue_brush(){
  int i = 0, j = 0;
  for(int k = 0; k<8*12*2 - 1; k+=2){
    int red = ((blue_brush[k+1]) & 0xF8 >> 3) <<11;
    int green = (((blue_brush[k] & 0xE0) >>5)) | ((blue_brush[k+1] & 0x7) << 3);
    int blue = (blue_brush[k] & 0x1F);
    short int p = red | ((green <<  5) | blue);
    plot_pixel(20+i, j, p);
    i+=1;
    if(i == 8){
      i = 0;
      j+=1;
    }
  }
}


void draw_green_brush(){
  int i = 0, j = 0;
  for(int k = 0; k<8*12*2 - 1; k+=2){
    int red = ((green_brush[k+1]) & 0xF8 >> 3) <<11;
    int green = (((green_brush[k] & 0xE0) >>5)) | ((green_brush[k+1] & 0x7) << 3);
    int blue = (green_brush[k] & 0x1F);
    short int p = red | ((green <<  5) | blue);
    plot_pixel(36+i, j, p);
    i+=1;
    if(i == 8){
      i = 0;
      j+=1;
    }
  }
}

void draw_pink_brush(){
  int i = 0, j = 0;
  for(int k = 0; k<8*12*2 - 1; k+=2){
    int red = ((pink_brush[k+1]) & 0xF8 >> 3) <<11;
    int green = (((pink_brush[k] & 0xE0) >>5)) | ((pink_brush[k+1] & 0x7) << 3);
    int blue = (pink_brush[k] & 0x1F);
    short int p = red | ((green <<  5) | blue);
    plot_pixel(52+i, j, p);
    i+=1;
    if(i == 8){
      i = 0;
      j+=1;
    }
  }
}

void draw_red_brush(){
  int i = 0, j = 0;
  for(int k = 0; k<8*12*2 - 1; k+=2){
    int red = ((red_brush[k+1]) & 0xF8 >> 3) <<11;
    int green = (((red_brush[k] & 0xE0) >>5)) | ((red_brush[k+1] & 0x7) << 3);
    int blue = (red_brush[k] & 0x1F);
    short int p = red | ((green <<  5) | blue);
    plot_pixel(68+i, j, p);
    i+=1;
    if(i == 8){
      i = 0;
      j+=1;
    }
  }
}

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


void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int *) 0xFF203020;  //pixel controller of front buffer address
    register int status; //the status register
    
    *pixel_ctrl_ptr = 1;    //start the synchronization procress by writing 
            // the value 1 into the front Buffer register in the pixel buffer controller
    
    status = *(pixel_ctrl_ptr + 3); // Wait for the S bit to become 0, that means VGA sweep is done
    //3*(4 bits) = 12 bits = 0xFF20302C

    //Polling to get status bit
    while ((status & 0x01) != 0) { //wait until bit S of the Status register becomes equal to 0
        status = *(pixel_ctrl_ptr + 3);
    }
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



void counting_down(){
    int status;
    *PRIVATE_TIMER_PTR = 100000000;//load the 200MHz timer to have 0.5s interval
    *(PRIVATE_TIMER_PTR + 2) = 0b011; //Enable is set to 1, load to 1 at the spot of (timer + 8 bits)
    
    while( *(PRIVATE_TIMER_PTR + 3) == 0){ //polling......
        status = *(PRIVATE_TIMER_PTR + 3); //doing nothing
    } // signal registered again
    
    *(PRIVATE_TIMER_PTR + 3) = 0; //reset interrupt signal
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


  
mouse_movement get_mouse_movement() {

  counting_down();
  counting_down();
  //Declare return struct variable
  mouse_movement movement;

  // Up counter resets upon counts to 3 to retrieve one 3-byte movement packet
  int counter = 1;
  // Unsigned char: 1 byte
  int byte1 = 0;
  int byte2 = 0;
  int byte3 = 0;

  volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address

    int PS2_data, RVALID;

    while(1) {

    PS2_data = *(PS2_ptr);  // read the Data register in the PS/2 port

  
    RVALID = (PS2_data & 0x8000); // extract the RVALID field (15th bit)

    if (RVALID && counter == 1) {
      // &0xFF: Only take the first 8 bits - data
      byte1 = PS2_data & 0xFF; 
    } else if (RVALID && counter == 2) {
      byte2 = PS2_data & 0xFF;
    } else if (RVALID && counter == 3) {
      byte3 = PS2_data & 0xFF;
    }



   if (counter ==3) { // One 3-byte movement packet has been stored in byte1/2/3, returning...
      //Get dx/dy movement
      int x_sign_bit = byte1 & 0x10; //0b00010000
      int y_sign_bit = byte1 & 0x20; //0b00100000
      int left_pressed_bit = byte1 & 0x01;// read 1st bit of the 1st byte of 3-byte movement packet
      int right_pressed_bit = byte1 & 0x02; // read 2nd bit of the 1st byte of 3-byte movement packet

      

      //Constructing movement struct element
      movement.left_pressed_bit = left_pressed_bit;
      movement.right_pressed_bit = right_pressed_bit;

      

      if(x_sign_bit == 0) { //positive dx

        movement.dx = byte2;

      } else { //negative dx with sign extension

        int sign_extended_twos =   0xFFFFFF00 + byte2 ; // Equivalent to 0xFFFFFE00 + byte 2

        movement.dx = sign_extended_twos;

      }

      if(x_sign_bit == 0) { //positive dx
        movement.dx = byte2;
      } else { //negative dx with sign extension
        int sign_extended_twos =   0xFFFFFF00 + byte2 ; // Equivalent to 0xFFFFFE00 + byte 2
        movement.dx = sign_extended_twos;
      }

      if(y_sign_bit == 0) { //positive dx
        movement.dy = byte3;
      } else { //negative dx
        int sign_extended_twos = 0xFFFFFF00 + byte3;
        movement.dy = sign_extended_twos;
      }

      return movement;

    } else { // Continue reading unfinished 3-byte packet
      counter ++;
    }

    if ( (byte2 == 0xAA) && (byte3 == 0x00) ) {
      // mouse inserted; initialize sending of data
      *(PS2_ptr) = 0xF4;
    }
    
    
    }

}


void ink_to_circle() {
  centriod centriod_find = find_centriod();
  printf ("Found centriod x/y: %i %i\n", centriod_find.x0, centriod_find.y0);
  int x0 = centriod_find.x0;
  int y0 = centriod_find.y0;
  int radius = centriod_find.radius;

  //Reset canvas to white in back buffer
  clear_screen(1);
  
  //Reconstruct a perfect circle in current back buffer
  midpoint_algorithm_draw_circle(x0, y0, radius);
  plot_pixel(x0, y0, BLACK);

  wait_for_vsync(); // swap front and back buffers on VGA vertical sync i.e. display drawed back buffer
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // Set to draw on new back buffer
  clear_screen(1);

  midpoint_algorithm_draw_circle(x0, y0, radius);
  plot_pixel(x0, y0, BLACK);

  
}


//Find the x/y centriod coordinates of all differnetial rectangles (1-pixel wide) and take the average
centriod find_centriod() {
  //INITIALIZING return value (x0, y0, and radius)
  centriod  centriod_find;
  
  //Default centriod position at 150,150
  centriod_find.x0 = 150;
  centriod_find.y0 = 150;
  //default Radius
  centriod_find.radius = 20;


  int x_segs_num = 0;
  int y_segs_num = 0;

  for(int i = 41; i < 230; i ++) {
    for(int j = 30; j <300; j ++) {
       if(canvas[i][j] == 1){
         //If come across a drawed pixel, start to traverse downwards (i++) and increment  y_seg_num (1 pixel wide)
         int tempi = i;
         int tempj = j;
         
         tempi ++;
         while(tempi <= 230) {
           if(canvas[tempi][j] == 1) {
            x_segs_num += 1;
            // Sum - Compute the avg x centriod of this differential rectangle (1-pixel wide)
            centriod_find.x0 += (tempi + i)/2;
            //?Set that bottom pixel to 0 since it's a enclosed shape 
            canvas[tempi][j] = 0;
            
           }
           tempi ++;
         }

          tempj ++;
          // Start to traverse to right 
          while(tempj <= 300) {
            if(canvas[i][tempj] == 1) {
              y_segs_num += 1;
              centriod_find.y0 += (tempj +j)/2;
              canvas[i][tempj] = 0;//?
              
            }

            tempj ++;

          }


       }
    }
  }

  //Last, sum all the x-centriod and y centriod of differential components and divided by number of components to get the avg 
  centriod_find.x0 = centriod_find.x0/x_segs_num;
  centriod_find.y0 = centriod_find.y0/y_segs_num;


  printf(" Sum of differential x_centriod : % i; Sum of differential y_centriod  is %i \n", centriod_find.x0, centriod_find.y0);
  printf(" Differential x_segd_num is %i; Differential y_segs_num is %i \n", x_segs_num, y_segs_num);


  /******************************Find all distance from drawed pixel to  found centriod and take the average as the radius******************************/

  // counter_differential_distance_from_pixel_to_centriod
  int counter_differential_distance = 0;

  //Will be casted into int in final stage 
  double sum_of_differential_distance = 0;

  for(int i = 41; i < 230; i ++) {
    for(int j = 30; j <300; j ++) {
       if(canvas[i][j] == 1){
         //If come across a drawed pixel, Compute the geometrical distance to the centriod found above
        double dx = abs( i - centriod_find.x0);
        double dy = abs( j - centriod_find.y0);
        sum_of_differential_distance += sqrt(pow((float)dx, 2) + pow((float)dy, 2));
        counter_differential_distance ++;
       }
  
    }
  }

  if(counter_differential_distance != 0)
    centriod_find.radius = (int)sum_of_differential_distance/ (int)counter_differential_distance;

  printf("Finding radius... The radius is: %i\n", centriod_find.radius);

  return centriod_find; 
  

}


// Midpoint Circle Algorithm
void midpoint_algorithm_draw_circle(int x0, int y0, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;
 
    while (x >= y) {

      
      draw_block(x0 + x, y0 + y,BLACK, 1);
      draw_block(x0 + y, y0 + x,BLACK, 1);
      draw_block(x0 - y, y0 + x,BLACK, 1);
      draw_block(x0 - x, y0 + y,BLACK, 1);
      draw_block(x0 - x, y0 - y,BLACK, 1);
      draw_block(x0 - y, y0 - x,BLACK, 1);
      draw_block(x0 + y, y0 - x,BLACK, 1);
      draw_block(x0 + x, y0 - y,BLACK, 1);
    
      if (err <= 0)
      {
          y += 1;
          err += 2*y + 1;
      }
    
      if (err > 0)
      {
          x -= 1;
          err -= 2*x + 1;
      }
    }
}


// void midpoint_algorithm_drawsquare(int x0, int y0, int side_Length){
//     int x = 0.5 * side_Length;
//     int y = 0;
//     int error = 0;

// }
