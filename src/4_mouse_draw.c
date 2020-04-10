#include "declaration.h"


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