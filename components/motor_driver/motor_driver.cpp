#include "motor_driver.h"

// 静态变量为每个电机分配唯一的 LEDC PWM 通道。
static ledc_channel_t next_ledc_channel = LEDC_CHANNEL_0;
static bool is_ledc_timer_init = false;
static constexpr uint32_t kMotorPwmFrequencyHz = 10000;

Tb6612Motor::Tb6612Motor(gpio_num_t pwm_pin, gpio_num_t in1_pin, gpio_num_t in2_pin)
    : pwm_pin_(pwm_pin),
      in1_pin_(in1_pin),
      in2_pin_(in2_pin),
      initialized_(false),
      usable_(pwm_pin != GPIO_NUM_NC && in1_pin != GPIO_NUM_NC &&
              in2_pin != GPIO_NUM_NC) {
  pwm_channel_ = next_ledc_channel;
  next_ledc_channel = static_cast<ledc_channel_t>(next_ledc_channel + 1);
}

void Tb6612Motor::Init() {
  if (!usable_) {
    return;
  }

  // TB6612 的 IN1/IN2 是方向控制脚，PWM 只连接到 PWMA/PWMB。
  gpio_config_t direction_config = {};
  direction_config.pin_bit_mask = (1ULL << in1_pin_) | (1ULL << in2_pin_);
  direction_config.mode = GPIO_MODE_OUTPUT;
  direction_config.pull_up_en = GPIO_PULLUP_DISABLE;
  direction_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  direction_config.intr_type = GPIO_INTR_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&direction_config));
  ESP_ERROR_CHECK(gpio_set_level(in1_pin_, 0));
  ESP_ERROR_CHECK(gpio_set_level(in2_pin_, 0));

  // 只初始化一次定时器。
  if (!is_ledc_timer_init) {
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode      = LEDC_LOW_SPEED_MODE;
    ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
    ledc_timer.timer_num       = LEDC_TIMER_0;
    ledc_timer.freq_hz         = kMotorPwmFrequencyHz;
    ledc_timer.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);
    is_ledc_timer_init = true;
  }

  ledc_channel_config_t pwm_channel = {};
  pwm_channel.speed_mode = LEDC_LOW_SPEED_MODE;
  pwm_channel.channel    = pwm_channel_;
  pwm_channel.timer_sel  = LEDC_TIMER_0;
  pwm_channel.intr_type  = LEDC_INTR_DISABLE;
  pwm_channel.gpio_num   = pwm_pin_;
  pwm_channel.duty       = 0;
  pwm_channel.hpoint     = 0;
  ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));
  initialized_ = true;
}

void Tb6612Motor::SetSpeed(int16_t speed) {
  if (!initialized_) {
    return;
  }

  if (speed > 255) speed = 255;
  if (speed < -255) speed = -255;

  if (speed > 0) {
    // 正转：IN1=1，IN2=0，PWM 控制输出功率。
    ESP_ERROR_CHECK(gpio_set_level(in1_pin_, 1));
    ESP_ERROR_CHECK(gpio_set_level(in2_pin_, 0));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel_, speed));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, pwm_channel_));
  } else if (speed < 0) {
    // 反转：IN1=0，IN2=1，PWM 控制输出功率。
    ESP_ERROR_CHECK(gpio_set_level(in1_pin_, 0));
    ESP_ERROR_CHECK(gpio_set_level(in2_pin_, 1));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel_, -speed));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, pwm_channel_));
  } else {
    // 停止/滑行：PWM=0，IN1=IN2=0。
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel_, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, pwm_channel_));
    ESP_ERROR_CHECK(gpio_set_level(in1_pin_, 0));
    ESP_ERROR_CHECK(gpio_set_level(in2_pin_, 0));
  }
}

void Tb6612Motor::Brake() {
  if (!initialized_) {
    return;
  }

  // TB6612 的 IN1=IN2=1 为短刹车，PWM 保持有效。
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, pwm_channel_, 255));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, pwm_channel_));
  ESP_ERROR_CHECK(gpio_set_level(in1_pin_, 1));
  ESP_ERROR_CHECK(gpio_set_level(in2_pin_, 1));
}
