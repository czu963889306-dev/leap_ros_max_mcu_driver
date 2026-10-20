#include "board.h"
#include "board_config.h"
#include "system_globals.h"
#include "esp_log.h"

static const char* TAG = "BOARD_INIT";
namespace board_cfg = leap_config::board;

// GPIO0/BOOT is a strapping/download pin. Using it as a runtime button is risky,
// but it is enabled here intentionally for the BOOT double-click action.

// ================= 对象实例化 =================
Lsm6ds3Imu imu(board_cfg::kImuI2cPort, board_cfg::kImuSdaPin, board_cfg::kImuSclPin);

// Map each motor's output channels to the configured IN1/IN2 pins.
At8236Motor motor_fl(board_cfg::kMotorFlIn1, board_cfg::kMotorFlIn2);
At8236Motor motor_fr(board_cfg::kMotorFrIn1, board_cfg::kMotorFrIn2);
At8236Motor motor_bl(board_cfg::kMotorBlIn1, board_cfg::kMotorBlIn2);
At8236Motor motor_br(board_cfg::kMotorBrIn1, board_cfg::kMotorBrIn2);

// 实例化编码器
QuadratureEncoder encoder_fl(board_cfg::kEncoderFlA, board_cfg::kEncoderFlB);
QuadratureEncoder encoder_fr(board_cfg::kEncoderFrA, board_cfg::kEncoderFrB);
QuadratureEncoder encoder_bl(board_cfg::kEncoderBlA, board_cfg::kEncoderBlB);
QuadratureEncoder encoder_br(board_cfg::kEncoderBrA, board_cfg::kEncoderBrB);

// 实例化外设
MotionController robot(motor_fl, motor_fr, motor_bl, motor_br,
                       encoder_fl, encoder_fr, encoder_bl, encoder_br);
UltrasonicSensor ultrasonic(board_cfg::kUltrasonicPin);
Ws2812Driver rgb_led(board_cfg::kRgbPin, board_cfg::kRgbLedCount);
StatusLed status_led(board_cfg::kStatusLedPin);
BatteryAdc battery_adc(board_cfg::kBatteryAdcUnit, board_cfg::kBatteryAdcChannel,
                       board_cfg::kBatteryDividerRatio,
                       board_cfg::kBatteryEmptyVoltage, board_cfg::kBatteryFullVoltage);
ButtonDriver io0_button(board_cfg::kIo0ButtonPin, true);


// ================= 统一初始化逻辑 =================
void board_init() {
    ESP_LOGI(TAG, "Initializing hardware...");
    robot.SetChassisType(system_globals().runtime.chassis_type);
    motor_fl.Init(); encoder_fl.Init();
    motor_fr.Init(); encoder_fr.Init();
    motor_bl.Init(); encoder_bl.Init();
    motor_br.Init(); encoder_br.Init();
    ESP_LOGI(TAG, "Front and rear wheels initialized for %s chassis.",
             SystemGlobals::ChassisTypeToDisplayName(system_globals().runtime.chassis_type));

    // 2. 初始化传感器和外设
    ultrasonic.Init();
    rgb_led.Init();
    status_led.Init();
    io0_button.Init();
    ESP_LOGW(TAG, "BOOT/IO0 is used as a runtime user button. Holding it during reset may enter download mode.");
    // 3. 初始化 IMU
    if (imu.Init()) {
        ESP_LOGI(TAG, "IMU (LSM6DS3) initialized.");
    } else {
        ESP_LOGE(TAG, "IMU Initialization FAILED!");
    }

    ESP_LOGI(TAG, "Hardware board ready.");
}
