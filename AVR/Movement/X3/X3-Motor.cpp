
#include "../X3/X3-Motor.h"

namespace X3 {

  Motor * Motor::headMotor = 0;

  Motor::Motor() {
    this->nextMotor = headMotor;
    headMotor = this;
  }

  void Motor::stepBy(float x, float y, float r) {};


  Motor * Motor::getHeadMotor() {
    return Motor::headMotor;
  }
  Motor * Motor::getNextMotor() {
    return this->nextMotor;
  }
}
