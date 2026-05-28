# 🤖 Hand Gesture Controlled Car — ESP32 + MPU6050

> A wireless robotic vehicle controlled in real-time through hand gestures using MPU6050 motion sensor and Bluetooth communication between two ESP32 microcontrollers.

![Project Photo]("C:\Users\rohit\OneDrive\Pictures\IMG_20260516_155507.jpg")
<!-- 👆 Replace with your actual robot photo path -->

---

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Circuit Connections](#circuit-connections)
- [Gesture Mapping](#gesture-mapping)
- [Differential Drive Mechanism](#differential-drive-mechanism)
- [Software & Libraries](#software--libraries)
- [Testing Results](#testing-results)
- [Getting Started](#getting-started)
- [Future Enhancements](#future-enhancements)

---

## Overview

This project implements a **wireless hand gesture-controlled robotic car** using embedded system technology. The MPU6050 sensor detects hand tilt and orientation, the ESP32 processes and transmits commands over Bluetooth, and the receiver ESP32 drives the motors through an L298N motor driver.

No physical remote is needed — tilting your hand controls the car in real time.

---

## System Architecture

The system is divided into two units — **Transmitter** and **Receiver** — communicating wirelessly via Bluetooth.

```
┌──────────────────────────────────┐         ┌──────────────────────────────────────┐
│          TRANSMITTER UNIT         │         │            RECEIVER UNIT             │
│                                  │         │                                      │
│  [Hand] → [MPU6050] → [ESP32]   │──BT───▶│  [ESP32] → [L298N] → [DC Motors]    │
│            I2C         Gesture   │         │             Motor       Robot        │
│                        Command   │         │             Driver      Movement     │
└──────────────────────────────────┘         └──────────────────────────────────────┘
```

**Transmitter unit:**
- MPU6050 sensor detects hand orientation via I2C
- ESP32 processes sensor data and maps tilt to movement commands
- Commands transmitted wirelessly via Bluetooth Classic

**Receiver unit:**
- ESP32 receives Bluetooth commands
- L298N motor driver controls DC motors using differential drive logic
- Robot performs forward, backward, left, right, and stop movements

---

## Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| ESP32 DevKit v1 | 2 | Microcontroller — transmitter + receiver |
| MPU6050 | 1 | 6-axis motion sensor (accelerometer + gyroscope) |
| L298N Motor Driver | 1 | Controls direction of DC motors |
| DC Geared Motors (TT) | 4 | Drives the robot wheels |
| 4-Wheel Robot Chassis | 1 | Physical frame |
| Li-Po Battery 7.4V 2000mAh | 1 | Powers the receiver/robot unit |
| USB Power Bank | 1 | Powers the transmitter ESP32 |

---

## Circuit Connections

### Transmitter — ESP32 ↔ MPU6050

![ESP32 MPU6050 Wiring]("C:\Users\rohit\OneDrive\Pictures\transmitter.jpg")
<!-- 👆 Add your actual wiring photo here -->

| ESP32 Pin | MPU6050 Pin | Function |
|---|---|---|
| 3.3V | VCC | Power Supply |
| GND | GND | Ground |
| GPIO 21 | SDA | I2C Data Line |
| GPIO 22 | SCL | I2C Clock Line |

### Receiver — ESP32 ↔ L298N Motor Driver

![L298N Wiring]("C:\Users\rohit\OneDrive\Pictures\receiver.jpeg")
<!-- 👆 Add your actual wiring photo here -->

| ESP32 Pin | L298N Pin | Function |
|---|---|---|
| GPIO 13 | IN1 | Left Motor Forward |
| GPIO 12 | IN2 | Left Motor Backward |
| GPIO 14 | IN3 | Right Motor Forward |
| GPIO 27 | IN4 | Right Motor Backward |
| GND | GND | Common Ground |

### L298N Motor Output Connections

| L298N Output | Connected Motors |
|---|---|
| OUT1 & OUT2 | Left Motor Pair |
| OUT3 & OUT4 | Right Motor Pair |
| 12V | Battery Positive (+) |
| ENA / ENB | Jumper HIGH (constant speed) |

### L298N Pin Description

| Pin | Function |
|---|---|
| IN1, IN2 | Control Motor A Direction |
| IN3, IN4 | Control Motor B Direction |
| ENA, ENB | Enable Pins / Speed Control |
| OUT1, OUT2 | Motor A Output |
| OUT3, OUT4 | Motor B Output |
| 12V | Motor Power Supply |
| GND | Ground |
| 5V | Logic Supply |

---

## Gesture Mapping

The MPU6050 accelerometer measures tilt along X and Y axes. After calibration offset correction, values are compared against thresholds to generate movement commands.

| Hand Gesture | Sensor Condition | Command Sent | Robot Movement |
|---|---|---|---|
| Tilt Forward | Y axis > 3 | `F` | Move Forward |
| Tilt Backward | Y axis < -3 | `B` | Move Backward |
| Tilt Right | X axis > 3 | `R` | Turn Right |
| Tilt Left | X axis < -3 | `L` | Turn Left |
| Neutral / Flat | \|X\| < 2 and \|Y\| < 2 | `S` | Stop |

**Gesture processing logic:**

```cpp
int x = (Ay - y_offset) / 1000;
int y = (Ax - x_offset) / 1000;

char cmd = 'S';
if (abs(x) < 2 && abs(y) < 2)  cmd = 'S';   // dead-zone → stop
else if (y > 3)                 cmd = 'F';   // forward tilt
else if (y < -3)                cmd = 'B';   // backward tilt
else if (x > 3)                 cmd = 'R';   // right tilt
else if (x < -3)                cmd = 'L';   // left tilt
```

---

## Differential Drive Mechanism

Left and right motor pairs are controlled independently to achieve all directional movements.

| Movement | Left Motors | Right Motors |
|---|---|---|
| Forward | Forward ↑ | Forward ↑ |
| Backward | Backward ↓ | Backward ↓ |
| Turn Left | Backward ↓ | Forward ↑ |
| Turn Right | Forward ↑ | Backward ↓ |
| Stop | Stop | Stop |

**Motor control functions:**

```cpp
void forward()  { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  }
void backward() { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); }
void turnLeft() { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  }
void turnRight(){ digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); }
void stopAll()  { digitalWrite(IN1,LOW);  digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW);  }
```

---

## Software & Libraries

**IDE:** Arduino IDE with ESP32 board package  
**Language:** Embedded C / C++

| Library | Purpose |
|---|---|
| `Wire.h` | I2C communication with MPU6050 |
| `BluetoothSerial.h` | Bluetooth Classic communication between ESP32 boards |

**Bluetooth setup:**

```cpp
// Transmitter — master mode
SerialBT.begin("ESP32_GESTURE", true);
SerialBT.connect(receiverMacAddress);

// Receiver — slave mode
SerialBT.begin("ESP32_CAR");
```

---

## Testing Results

### Gesture Recognition Accuracy

| Gesture | Trials | Correct Detections | Accuracy |
|---|---|---|---|
| Forward Tilt | 20 | 19 | 95% |
| Backward Tilt | 20 | 18 | 90% |
| Left Tilt | 20 | 19 | 95% |
| Right Tilt | 20 | 18 | 90% |
| Stop (Neutral) | 20 | 20 | 100% |
| **Overall** | **100** | **94** | **94%** |

### Response Time / Latency

| Parameter | Observed Value |
|---|---|
| Sensor Data Acquisition | ~5 ms per reading |
| Gesture Processing (ESP32) | ~2–3 ms |
| Bluetooth Transmission Latency | ~20–50 ms (avg ~35 ms) |
| Motor Response Time (L298N) | < 5 ms |
| **Total End-to-End Latency** | **~30–60 ms (avg ~42 ms)** |

### Navigation Smoothness

| Navigation Aspect | Observation | Rating |
|---|---|---|
| Forward Motion | Smooth and consistent at fixed speed | Excellent |
| Backward Motion | Smooth, consistent with forward | Excellent |
| Left Turn | Clean spot-turn | Good |
| Right Turn | Clean spot-turn | Good |
| Stop Response | Immediate, no overshoot | Excellent |
| Bluetooth Dropout | None within 5m range | Excellent |

### Overall System Performance

| Parameter | Result |
|---|---|
| Effective Bluetooth Range | Up to 8–10 metres (line-of-sight) |
| Gesture Recognition Accuracy | 94% overall (100% for stop) |
| Average System Latency | ~42 ms end-to-end |
| Continuous Operation Time | ~45 min on 7.4V 2000mAh Li-Po |
| Motor Control Reliability | 100% correct response to commands |
| Calibration Drift Reduction | ~85% |

---

## Getting Started

### Step 1 — Flash the Receiver ESP32

1. Open `receiver/receiver.ino` in Arduino IDE
2. Select **Tools → Board → ESP32 Dev Module**
3. Select the correct COM port → click **Upload**
4. Open Serial Monitor (115200 baud) — **copy the MAC address printed**

### Step 2 — Update Transmitter with MAC Address

In `transmitter/transmitter.ino`, paste the receiver MAC:

```cpp
uint8_t address[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

### Step 3 — Flash the Transmitter ESP32

Upload `transmitter/transmitter.ino` to the second ESP32.

### Step 4 — Power Up & Test

1. Power the robot (receiver) with the Li-Po battery
2. Power the transmitter with a USB power bank
3. Wait ~3–5 seconds for Bluetooth pairing
4. Tilt your hand to control the robot!

---

## Future Enhancements

- [ ] **PWM-based speed control** — vary speed based on degree of hand tilt
- [ ] **WiFi / mobile app control** — extend operating range beyond Bluetooth
- [ ] **Obstacle avoidance** — ultrasonic/IR sensors for autonomous navigation
- [ ] **AI gesture recognition** — ML-based adaptive gesture classification
- [ ] **FPV camera** — ESP32-CAM integration for first-person view navigation

---

## License

MIT License — free to use for educational and non-commercial projects.
