#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// 👉 Car ESP32 MAC Address
uint8_t address[6] = {0xE0, 0x8C, 0xFE, 0x33, 0x18, 0xAA};

// MPU variables
int16_t Ax, Ay, Az;

// Calibration offsets
long x_offset = 0;
long y_offset = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Bluetooth MASTER mode
  SerialBT.begin("ESP32_GESTURE", true);

  Serial.println("Connecting to Car...");
  if (SerialBT.connect(address)) {
    Serial.println("Connected to Car!");
  } else {
    Serial.println("Connection Failed!");
  }

  // Wake MPU6050
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // 🔥 Calibration (keep MPU still)
  Serial.println("Calibrating... Keep sensor still");
  delay(2000);

  for (int i = 0; i < 100; i++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true);

    Ax = Wire.read() << 8 | Wire.read();
    Ay = Wire.read() << 8 | Wire.read();

    x_offset += Ax;
    y_offset += Ay;

    delay(10);
  }

  x_offset /= 100;
  y_offset /= 100;

  Serial.println("Calibration Done!");
}

void loop() {

  // Read MPU6050
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true);

  Ax = Wire.read() << 8 | Wire.read();
  Ay = Wire.read() << 8 | Wire.read();
  Az = Wire.read() << 8 | Wire.read();

  // Apply calibration offsets
  int x = (Ay - y_offset) / 1000;
  int y = (Ax - x_offset) / 1000;

  char cmd = 'S';

  // 🔥 Gesture logic with dead zone
  if (abs(x) < 2 && abs(y) < 2) {
    cmd = 'S';
  }
  else if (y > 3) cmd = 'F';
  else if (y < -3) cmd = 'B';
  else if (x > 3) cmd = 'R';
  else if (x < -3) cmd = 'L';

  // Send command
  if (SerialBT.connected()) {
    SerialBT.write(cmd);
  }

  // Debug output
  Serial.print("X: "); Serial.print(x);
  Serial.print(" Y: "); Serial.print(y);
  Serial.print(" → Command: "); Serial.println(cmd);

  delay(200);
}
