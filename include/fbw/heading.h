#ifndef HEADING_H
#define HEADING_H

#endif //AILERON_H

#include "cardinal_direction.cpp"
#include <iostream>

class heading{
  public:
    /* Default constructor for test initialisation */ 
    heading(){
      bearing = 0; // True North
      cd_dir = cardinal_direction::NORTH;
    }

    heading(int bearing){
      if(bearing < 0 || bearing > 359){
         std::cout << "Invalid Bearing";    
      }

      this.bearing = bearing;
      if(bearing == 0 && bearing < 20){
        cd_dir = cardinal_direction::NORTH;
      }else if(bearing > 20 && bearing <=90){
        cd_dir = cardinal_direction::NORTH_EAST;
      }else if(bearing > 90 && bearing <=180){
        cd_dir = cardinal_direction::SOUTH_EAST
      }else if(bearing == 180 && bearing < 210){
        cd_dir = cardinal_direction::SOUTH;
      }else if(bearing > 210 && bearing<=270){
         cd_dir = cardinal_direction::SOUTH_WEST
      }else if(bearing > 270 && bearing <= 359){
         cd_dir = cardinal_direction::NORTH_WEST
      }
    }

    void update_bearing(int bearing);
    int get_bearing()  const;
  private:
   int bearing;
   cardinal_direction cd_dir;
};
