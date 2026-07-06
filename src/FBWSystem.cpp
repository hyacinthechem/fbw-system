
/* Worked on during Auckland to Hong Kong 2026 on Airbus A350 - 900 */


#include <iostream>
#include "left_aileron.h"
#include "right_aileron.h"
#include "aileron_position.h"
#include "heading.h"
#include "FBWSystem.h"


    
    /* Empty constructor*/
    FBWSystem::FBWSystem(){}

    /* System banks the plane to the right by internally calling move_aileron.
       the aileron is moved based on its bank angle and position (enum)
    */ 
    void FBWSystem::bank_right(int bank_angle, left_aileron lft_alr, right_aileron rght_alr){
      /* Bankright is detected when new repeated movement
         occurs by pilot to move aircraft towards right
      */
       if(bank_angle > 60){
          std::cout << "Dangerous Bank Angle";
          /* Automatically bank to the left at an angle of 30 to reset*/
          bank_left(30, lft_alr, rght_alr);
       }


      lft_alr.move_aileron(bank_angle, aileron_position::DOWN);
      rght_alr.move_aileron(bank_angle,aileron_position::UP);
    
    }

    void FBWSystem::bank_left(int bank_angle, left_aileron lft_alr, right_aileron rght_alr){
        /* Bankleft is detected when new repeated movement
           occurs by pilot to move aircraft towards left
        */ 

        if(bank_angle > 60){
            std::cout << "Dangerous Bank Angle";
            /* Automatically bank to the right at an angle of 30 to reset */
            bank_right(30, lft_alr, rght_alr); 
        }

        lft_alr.move_aileron(bank_angle, aileron_position::UP);
        rght_alr.move_aileron(bank_angle, aileron_position::DOWN);
    }


    // FBW -> MoveAileron -> updateHeading -> updateDirection

   /* Adjust heading after banking
   
   Rate of turn:

   g * tan(bank_angle) / true_airspeed

   Right turn:
     new_heading = initial_heading + ( (rate of turn) * time )

   Left turn:  
     new_heading = initial_heading - ( (rate of turn) * time )
   */ 


