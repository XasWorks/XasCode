/*
 *  Header file for the standard 3X-Motor actuators
 *
 * Xasin, 5.05.2016
 *
*/


#ifndef _3X_MOTOR_H
#define _3X_MOTOR_H

namespace X3 {

  class Motor {
  private:
    Motor *nextMotor = 0;
    static Motor *headMotor;

  public:
    Motor();

    virtual void stepBy(float x, float y, float r);

    static Motor * getHeadMotor();
    Motor * getNextMotor();
  };

}

#endif
