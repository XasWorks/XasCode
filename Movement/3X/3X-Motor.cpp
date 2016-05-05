
namespace 3X {

  Motor::headMotor = 0;

  Motor::Motor() {
    this->nextMotor = headMotor;
    headMotor = this;
  }

  Motor::moveBy(float x, float y, float r) {};


  Motor::getHeadMotor() {
    return Motor::headMotor;
  }
  Motor::getNextMotor() {
    return this->nextMotor;
  }
}
