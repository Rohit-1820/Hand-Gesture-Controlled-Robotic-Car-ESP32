# 🤖 Hand Gesture Controlled Car — ESP32 + MPU6050

> A wireless robotic vehicle controlled in real-time through hand gestures using MPU6050 motion sensor and Bluetooth communication between two ESP32 microcontrollers.

![Project Photo](docs/images/robot_photo.jpg)
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

This project presents the design and implementation of a Hand Gesture Controlled Robotic Car using ESP32 microcontrollers, MPU6050 accelerometer & gyroscope sensor, and Bluetooth communication.
The system enables real-time wireless robotic navigation through natural hand gestures. Hand movements(hand tilt and orientation) are detected using the MPU6050 sensor mounted on the transmitter module. The gesture data is processed by an ESP32 microcontroller and transmitted wirelessly via Bluetooth to another ESP32 mounted on the robotic vehicle.

The receiver ESP32 controls the movement of the robotic car through the L298N motor driver using differential drive logic.


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
- ESP32 applies calibration offsets and stability filtering
- Only sends command when gesture changes — no flooding
- Commands transmitted wirelessly via Bluetooth Classic

**Receiver unit:**
- ESP32 receives Bluetooth commands
- Acts only when command changes (deduplication)
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

![ESP32 MPU6050 Wiring](docs/images/transmitter_wiring.jpg)
<!-- 👆 Add your actual wiring photo here -->

| ESP32 Pin | MPU6050 Pin | Function |
|---|---|---|
| 3.3V | VCC | Power Supply |
| GND | GND | Ground |
| GPIO 21 | SDA | I2C Data Line |
| GPIO 22 | SCL | I2C Clock Line |

### Receiver — ESP32 ↔ L298N Motor Driver

![L298N Wiring](docs/images/receiver_wiring.jpg)
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

The MPU6050 accelerometer measures tilt along X and Y axes. After calibration offset correction, the dominant axis is identified and compared against thresholds to generate a clean, flicker-free movement command.

| Hand Gesture | Dominant Axis | Sensor Condition | Command Sent | Robot Movement |
|---|---|---|---|---|
| Tilt Forward | Y | Y > 3 | `F` | Move Forward |
| Tilt Backward | Y | Y < -3 | `B` | Move Backward |
| Tilt Right | X | X > 3 | `R` | Turn Right |
| Tilt Left | X | X < -3 | `L` | Turn Left |
| Neutral / Flat | — | \|X\| < 2 and \|Y\| < 2 | `S` | Stop |

**Key software improvements in the transmitter:**

```cpp
// 1. Dominant axis selection — prevents diagonal confusion
if (abs(y) >= abs(x)) {
    if      (y > 3)  detectedCmd = 'F';
    else if (y < -3) detectedCmd = 'B';
} else {
    if      (x > 3)  detectedCmd = 'R';
    else if (x < -3) detectedCmd = 'L';
}

// 2. Stability filter — gesture must appear 3 times in a row before accepting
if (detectedCmd == pendingCmd) stableCount++;
else { pendingCmd = detectedCmd; stableCount = 0; }

// 3. Send only on change — no command flooding to receiver
if (stableCount >= 3 && detectedCmd != lastCmd) {
    SerialBT.write(detectedCmd);
    lastCmd = detectedCmd;
}
```

These three improvements together eliminate the two main issues seen in basic implementations — continuous rotation and locking in one direction.

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

**Motor control functions (receiver):**

```cpp
void forward()  { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  }
void backward() { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); }
void left()     { digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); }
void right()    { digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  }
void stopMotors(){ digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);  digitalWrite(IN3,LOW);  digitalWrite(IN4,LOW);  }
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

### Step 4 — Power Up & Calibrate

1. Power the robot (receiver) with the Li-Po battery
2. Power the transmitter with a USB power bank
3. **Keep your hand flat and still for 3 seconds** during calibration
4. Wait for "Ready" message in Serial Monitor
5. Tilt your hand to control the robot!

> ⚠️ **Important:** If hand is tilted during calibration, all gestures will be offset. Always calibrate with hand flat.

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
