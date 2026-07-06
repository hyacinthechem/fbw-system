//
// Created by Hyacinthe Chemasle on 06/01/2026.
//

#ifndef DISPLAY_H
#define DISPLAY_H

#endif //DISPLAY_H
#include "left_aileron.h"
#include "right_aileron.h"
#include "stabiliser.h"
#include "vertical.h"
#include "horizontal.h"
#include "heading.h"
#include "cardinal_direction.cpp"


class FBWSystem {
  public:
    explicit FBWSystem(); //constructor for cockpit display

    /* Use of Aileron, both left and right aileron need to be queried to assess
      both down and up position to verify banking of right and left turns
    */
    void bank_right(int bank_angle, left_aileron lft_alr, right_aileron rght_alr); //control to turn plane right
    void bank_left(int bank_angle, left_aileron lft_alr, right_aileron rght_alr);  //control to turn plane left

    /* Use of Stabiliser */
    /* Feedback for how big of a push down force and pull up force*/ 
    void pull_up(int feedback);
    void push_down(int feedback);

    /* Use of flaps */
    void set_flaps(int pos);

    /* update heading after movement event */
    void set_heading(int bearing){
      if(bearing == 0 && bearing < 20){
        cd_dir = cardinal_direction::NORTH;
      }else if(bearing > 20 && bearing <=90){
        cd_dir = cardinal_direction::NORTH_EAST;
      }else if(bearing > 90 && bearing <=180){
        cd_dir = cardinal_direction::SOUTH_EAST;
      }else if(bearing == 180 && bearing < 210){
        cd_dir = cardinal_direction::SOUTH;
      }else if(bearing > 210 && bearing<=270){
         cd_dir = cardinal_direction::SOUTH_WEST;
      }else if(bearing > 270 && bearing <= 359){
         cd_dir = cardinal_direction::NORTH_WEST;
      }
       hdg.update_bearing(bearing);
    }
    /*Get the heading */
    heading get_heading() const { return hdg;} 
    /* Update current direction after movement event */
    void move_dir(std::string direction);

    /* Get cardinal direction */
    cardinal_direction get_crd_dir() const { return cd_dir; }

    ~FBWSystem(); //destructor for cockpit display

  private:
    /*Get the current heading */ 
    heading hdg;
    /* Convert to normal N, NE, S, SE, SW, NW String direction */
    cardinal_direction cd_dir;
};