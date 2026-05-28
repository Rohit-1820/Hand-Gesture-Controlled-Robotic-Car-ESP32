#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

// Motor Pins
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

char lastCmd = 'S';   // store last command (important)

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_CAR"); // Bluetooth name

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors(); // safety

  Serial.println("Bluetooth Started. Connect to ESP32_CAR");
}

void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();

    Serial.print("Received: ");
    Serial.println(cmd);

    // 👉 Only act if command changes (VERY IMPORTANT)
    if (cmd != lastCmd) {
      lastCmd = cmd;

      switch (cmd) {
        case 'F':
          forward();
          break;

        case 'B':
          backward();
          break;

        case 'L':
          left();
          break;

        case 'R':
          right();
          break;

        case 'S':
          stopMotors();
          break;
      }
    }
  }
}

// 🚗 Forward
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// 🔙 Backward
void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// ⬅️ Left
void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// ➡️ Right
void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// 🛑 Stop
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
