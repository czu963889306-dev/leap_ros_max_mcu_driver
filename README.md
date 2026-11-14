# leap_ros_max_mcu_driver

Leap ROS MAX 的 ESP32-S3 下位机固件，提供四轮底盘控制、里程计、IMU、STL26 雷达、电池监测、手柄控制和 Wi-Fi 配置。

## 主要功能

- 支持四轮麦克纳姆轮和四轮差速模式，可在配网页选择并保存。
- 支持 micro-ROS UART 和 Wi-Fi UDP，默认 UART；提供 `/cmd_vel`、`/odom`、`/imu`、`/scan`、`/battery_state` 和 PID 参数服务。
- STL26 连接 UART1、RX GPIO41，波特率 230400。雷达按 360 个方向保存最近的非零回波，不再按每圈 500 个原始点截断，也不再屏蔽 10 cm 内的回波。
- 雷达角度按 `(90 - 原始角度 + 360) % 360` 映射，所有方向开放。ROS `LaserScan` 声明的最小量程为 0.02 m。
- 与 micro-ROS Agent 每 5 秒同步时间，失败后每秒重试；最近成功校准的时间最多保留 30 秒，时钟回拨时暂停带时间戳的发布以防时间戳倒退。
- 支持 I²C 手柄接管、状态灯、电池电压采集和 Wi-Fi 配网页。

## 通信与配置

micro-ROS UART 使用 UART0、GPIO43 TX、GPIO44 RX、921600 波特率，默认启动等待 8 秒。UDP 默认 Agent 地址为 `192.168.31.214:8888`；运行模式、Agent 地址和底盘模式可在配网页配置并保存。

硬件引脚、雷达串口、底盘参数与同步周期分别位于 `main/config/board_config.h`、`network_config.h`、`motion_config.h` 和 `ros_config.h`。完整话题、参数与底盘运动学见 [协议说明](doc/protocols.md)。

## 构建与验证

加载 ESP-IDF 环境后执行：

```bash
idf.py build
```

应用固件为 `build/leap_ros_max.bin`，完整烧录包为 `build/merged-binary.bin`，完整包的烧录地址为 `0x0`。

无需硬件的主机测试：

```bash
bash tests/run_lidar_360_test.sh
cmake -S tests -B /tmp/leap-max-time-sync-tests
cmake --build /tmp/leap-max-time-sync-tests
ctest --test-dir /tmp/leap-max-time-sync-tests --output-on-failure
```

上板后应验证雷达方向、近距离回波、运动控制、两种通信模式及时间同步。
