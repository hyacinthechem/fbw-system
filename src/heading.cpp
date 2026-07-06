#include "heading.h"
#include <iostream>

void heading::update_bearing(int bearing){
 if(bearing >= 0 && bearing<=359){
    std::cout << "Incorrect Bearing input";
 }   
 this->bearing = bearing;
}

int heading::get_bearing() const { return bearing; }
