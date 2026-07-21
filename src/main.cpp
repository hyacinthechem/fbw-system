#include <iostream>
#include "left_aileron.h"
#include "right_aileron.h"
#include "stabiliser.h"
#include "vertical.h"
#include "horizontal.h"
#include "FBWSystem.cpp"

using namespace std;

int main() {

  /* Create virtual a350 aircraft fbw system*/
  FBWSystem a350_acf;   
  /* Initialise ailerons as flight control surface */
  left_aileron lft_aileron;
  right_aileron rght_aileron;
  /* Utilises move aileron method to bank right */  
  executeRightTurn(a350_acf, lft_aileron, rght_aileron);
  /*Utilises move aileron method to bank left*/
  executeLeftTurn(a350_acf, lft_aileron, rght_aileron);

  return 0;
}

void executeRightTurn(FBWSystem acf, left_aileron lft_aileron, right_aileron rght_aileron){
   /* Get necessary information about flight data to pass as parameters to banking method */
   int bank_angle = acf.get_bank_angle(); 
   double true_air_speed = acf.get_true_air_speed();
   
   /* call abstracted method within FBWSystem */ 
   acf.bank_right(bank_angle, lft_aileron, rght_aileron, true_air_speed);
   
   double bearing = acf.get_heading().get_bearing();
   cardinal_direction cd_dir = acf.get_crd_dir();
   /* Assert that with a bank angle of zero that there is no change in heading or direction */
   printf("%d", bearing);
   printf("%s",cd_dir);
}

void executeLeftTurn(FBWSystem acf, left_aileron lft_aileron, right_aileron rght_aileron){
  /* Get necessary information about flight data to pass as parameters to banking method */
   int bank_angle = acf.get_bank_angle(); 
   double true_air_speed = acf.get_true_air_speed();
   
   /* call abstracted method within FBWSystem */ 
   acf.bank_left(bank_angle, lft_aileron, rght_aileron, true_air_speed);
}
