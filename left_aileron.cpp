//
// Created by Hyacinthe Chemasle on 06/01/2026.
// Worked on during CX198 from Auckland To Hong Kong on Airbus A350-900
//
#include "left_aileron.h"
#include "aileron_position.h"
#include <string>

/*
 This method is called when bankRight or bankLeft is called
*/
void left_aileron::move_aileron(int bank_angle, aileron_position alr_position) {
  this->bank_angle = bank_angle;
  this->alr_position = alr_position;
}


int left_aileron::get_bank_angle() const{ return bank_angle;}

aileron_position left_aileron::get_aileron_position() const{ return alr_position;}

