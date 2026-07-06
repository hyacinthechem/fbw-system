//
// Created by Hyacinthe Chemasle on 06/01/2026.
//

#ifndef LEFT_AILERON_H
#define LEFT_AILERON_H

#endif //AILERON_H
#include <string>
#include "aileron_position.h"

class left_aileron {
private:
    int bank_angle;
    aileron_position alr_position;
public:
     left_aileron() {
        bank_angle = 0; // default -> aileron is level
        alr_position = aileron_position::NEUTRAL; // default -> every aileron starts level
    }
    void move_aileron(int angle, aileron_position aileronPosition);
    int get_bank_angle() const;
    aileron_position get_aileron_position() const;
};

