
/* Worked on during Auckland to Hong Kong 2026 on Airbus A350 - 900 */


#include <iostream>
#include <chrono>
#include "left_aileron.h"
#include "right_aileron.h"
#include "aileron_position.h"
#include "heading.h"
#include "FBWSystem.h"


    
    /* Default constructor implementation: this is important for initial aircraft model that has a base altitude of 10,000ft, heading of true north with its cardinal direction
    and a 0 degree bank angle, enforcing level flight with ailerons. true air speed default is 410 knots*/
    FBWSystem::FBWSystem(){
       true_air_speed = 400;
       bank_angle = 0;
       hdg = 0;
       cd_dir = cardinal_direction::NORTH;
    }

    /* System banks the plane to the right by internally calling move_aileron.
       the aileron is moved based on its bank angle and position (enum)
    */ 
    void FBWSystem::bank_right(int bank_angle, left_aileron lft_alr, right_aileron rght_alr, int true_air_speed){
      /* Bankright is detected when new repeated movement
         occurs by pilot to move aircraft towards right
      */
       auto start = std::chrono::steady_clock::now(); // Get the start time of movement operation

       if(bank_angle > 60){
          std::cout << "Dangerous Bank Angle";
          /* Automatically bank to the left at an angle of 30 to reset*/
          bank_left(30, lft_alr, rght_alr, true_air_speed);
       }


      lft_alr.move_aileron(bank_angle, aileron_position::DOWN);
      rght_alr.move_aileron(bank_angle,aileron_position::UP);
   
      auto end = std::chrono::steady_clock::now(); // Get the end time of movement operation

      std::chrono::duration<double> elapsed = start - end;
      double elps_time = elapsed.count();
      
      double gravity_constant = 9.81;
      double arc_calc = tan(bank_angle) * gravity_constant;
      double rate_of_turn = arc_calc / true_air_speed;

      /* Use calculated rate of turn to calculate new_heading for right turn */
      double initial_bearing = get_heading().get_bearing();
      double new_bearing = initial_bearing + (rate_of_turn * elps_time);
      set_heading(new_bearing);
    }

    void FBWSystem::bank_left(int bank_angle, left_aileron lft_alr, right_aileron rght_alr, int true_air_speed){
        /* Bankleft is detected when new repeated movement
           occurs by pilot to move aircraft towards left
        */ 
        
        auto start = std::chrono::steady_clock::now(); // Get the start time of movement operation
        if(bank_angle > 60){
            std::cout << "Dangerous Bank Angle";
            /* Automatically bank to the right at an angle of 30 to reset */
            bank_right(30, lft_alr, rght_alr, true_air_speed); 
        }

        lft_alr.move_aileron(bank_angle, aileron_position::UP);
        rght_alr.move_aileron(bank_angle, aileron_position::DOWN);
        
        auto end = std::chrono::steady_clock::now(); // Get the end time of movement operation
        
        std::chrono::duration<double> elapsed = end - start;
        double elps_time = elapsed.count();
        /* Calculate rate of turn */
        double gravity_constant = 9.81;
        double arc_calc = gravity_constant * tan(bank_angle);
        double rate_of_turn = arc_calc / true_air_speed;

        /* Use calculated rate of turn to calculate new_heading for left turn */
        int initial_hdg_bearing = get_heading().get_bearing();
        int new_bearing = initial_hdg_bearing - (rate_of_turn * elps_time);

        /*Update new calculated heading*/
        set_heading(new_bearing); 
    }


    // FBW -> MoveAileron -> updateHeading -> updateDirection

     void FBWSystem::pull_up(int feedback, double true_air_speed){
        /* Five kinds of feedback
            1. Light
            2. Small
            3. Medium
            4. High
            5. Heavy
        */ 
      
        if(feedback < 0 ){
          std::cout << "Invalid Feedback input, feedback must be positive";
        }

        double angle_of_attack = calculate_angle_of_attack(feedback);

        update_altimeter(true_air_speed, angle_of_attack);
     }

     void FBWSystem::update_altimeter(double true_air_speed, double angle_of_attack){
        
     }

     void FBWSystem::push_down(int feedback, double true_air_speed){
       if(feedback > 0){
          std::cout << "Invalid Feedback input, feedback must be negative";
       }

       calculate_angle_of_attack(feedback);
     }

     double FBWSystem::calculate_angle_of_attack(int feedback){
       double angle_of_attack = feedback * 5;
       return angle_of_attack;
     }




/* Adjust heading after banking
   
   Rate of turn:

   g * tan(bank_angle) / true_airspeed

   Right turn:
     new_heading = initial_heading + ( (rate of turn) * time )

   Left turn:  
     new_heading = initial_heading - ( (rate of turn) * time )
   */ 
