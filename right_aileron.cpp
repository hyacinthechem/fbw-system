#include "right_aileron.h"
#include "aileron_position.h"
#include <string>


void right_aileron::move_aileron(int bank_angle, aileron_position alr_position){
    this->bank_angle = bank_angle;
    this->alr_position = alr_position;
}
  
int right_aileron::get_bank_angle() const { return bank_angle;}

aileron_position right_aileron::get_aileron_position() const { return alr_position; }


