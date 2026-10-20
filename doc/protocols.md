# Leap ROS Max micro-ROS 协议说明

`leap_ros_max` 是基于 ESP32-S3 的两轮底盘驱动，当前仅使用 micro-ROS 通信。

## 运行模式

| 模式 | 运行值 | 传输方式 |
| --- | --- | --- |
| micro-ROS | `micro_ros` | Wi-Fi UDP |
| micro-ROS UART | `microros_uart` | UART2，TX=`GPIO37`，RX=`GPIO38`，460800 baud |

默认 Agent 地址为 `192.168.31.214:8888`，Wi-Fi UDP 本地端口为
`CONFIG_MICRO_ROS_LOCAL_PORT`（默认 `8888`）。UART 模式会直接初始化传输并创建
micro-ROS 会话，无需主机发送额外的启动握手命令。

## 订阅

| 话题 | 类型 | 说明 |
| --- | --- | --- |
| `/cmd_vel` | `geometry_msgs/msg/Twist` | `linear.x/y` 由 m/s 转为 mm/s，`angular.z` 为 rad/s |

## 发布

| 话题 | 类型 | 频率 | 说明 |
| --- | --- | --- | --- |
| `/odom` | `nav_msgs/msg/Odometry` | 50 Hz | 里程计，位置单位 m、速度单位 m/s |
| `/imu` | `sensor_msgs/msg/Imu` | 50 Hz | 四元数、角速度和线加速度 |
| `/scan` | `sensor_msgs/msg/LaserScan` | 10 Hz | 360 点，距离单位 m |
| `/battery_state` | `sensor_msgs/msg/BatteryState` | 10 Hz | 电压和电量百分比 |
| `/ultrasonic` | `sensor_msgs/msg/Range` | 10 Hz | 超声波距离，单位 m |

## 服务

| 服务 | 类型 | 说明 |
| --- | --- | --- |
| `/set_speed_pid` | `leap_interfaces/srv/SetSpeedPid` | 设置并保存速度 PID |
| `/get_speed_pid` | `leap_interfaces/srv/GetSpeedPid` | 获取当前速度 PID |

## 坐标系

- 里程计：`odom` -> `base_link`
- IMU：`imu_link`
- 雷达：`laser_frame`
- 超声波：`ultrasonic_link`

## 单位

内部运动控制使用 mm、mm/s；micro-ROS 的位置和线速度使用 m、m/s；IMU
线加速度使用 m/s²，角速度使用 rad/s；雷达和超声波距离使用 m。
