#include "declaration.h"

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