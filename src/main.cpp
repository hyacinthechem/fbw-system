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
  
  /* Utilises move aileron method to bank right */  
  executeRightTurn(a350_acf);
  /*Utilises move aileron method to bank left*/
  executeLeftTurn(a350_acf);

  return 0;
}

void executeRightTurn(FBWSystem acf){

   acf.bank_right()
}

void executeLeftTurn(FBWSystem acf){

}
