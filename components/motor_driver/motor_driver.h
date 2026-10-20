#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

class Tb6612Motor {
 public:
  Tb6612Motor(gpio_num_t pwm_pin, gpio_num_t in1_pin, gpio_num_t in2_pin);

  void Init();
  void SetSpeed(int16_t speed);
  void Brake();

 private:
  gpio_num_t pwm_pin_;
  gpio_num_t in1_pin_;
  gpio_num_t in2_pin_;
  ledc_channel_t pwm_channel_;
  bool initialized_;
  bool usable_;
};

#endif  // MOTOR_DRIVER_H_
