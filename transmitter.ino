#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// 👉 Your Receiver ESP32 MAC Address
uint8_t address[6] = {0xE0, 0x8C, 0xFE, 0x33, 0x18, 0xAA};

// MPU6050 raw values
int16_t Ax, Ay, Az;

// Calibration offsets
long x_offset = 0;
long y_offset = 0;

// Last sent command
char lastCmd = ' ';

// Stability filter variables
int stableCount = 0;
char pendingCmd = 'S';

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Bluetooth MASTER mode
  SerialBT.begin("ESP32_GESTURE", true);
  Serial.println("Connecting to Car...");

  bool connected = SerialBT.connect(address);
  if (connected) {
    Serial.println("Connected to Car!");
  } else {
    Serial.println("Connection Failed! Check MAC address.");
  }

  // Wake MPU6050
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Calibration — keep hand flat and still
  Serial.println("Calibrating... Keep sensor FLAT and STILL for 3 seconds");
  delay(3000);

  long sumX = 0, sumY = 0;
  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true);
    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read(); // discard Az
    sumX += ax;
    sumY += ay;
    delay(10);
  }

  x_offset = sumX / 200;
  y_offset = sumY / 200;

  Serial.print("Calibration Done! X offset: ");
  Serial.print(x_offset);
  Serial.print("  Y offset: ");
  Serial.println(y_offset);
  Serial.println("Ready — move hand to control car.");
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

  // Apply calibration
  int x = (Ay - y_offset) / 1000;
  int y = (Ax - x_offset) / 1000;

  // Gesture Detection with Dead Zone
  char detectedCmd = 'S';

  if (abs(x) < 2 && abs(y) < 2) {
    detectedCmd = 'S';  // inside dead zone → stop
  }
  else if (abs(y) >= abs(x)) {
    // Y axis dominant
    if      (y > 3)  detectedCmd = 'F';
    else if (y < -3) detectedCmd = 'B';
  } else {
    // X axis dominant
    if      (x > 3)  detectedCmd = 'R';
    else if (x < -3) detectedCmd = 'L';
  }

  // Stability Filter — same gesture must appear 3 times in a row
  if (detectedCmd == pendingCmd) {
    stableCount++;
  } else {
    pendingCmd = detectedCmd;
    stableCount = 0;
  }

  // Send only when gesture is stable AND command has changed
  if (stableCount >= 3 && detectedCmd != lastCmd) {
    if (SerialBT.connected()) {
      SerialBT.write(detectedCmd);
      lastCmd = detectedCmd;
      Serial.print(">>> SENT: ");
      Serial.println(detectedCmd);
    } else {
      Serial.println("Bluetooth disconnected! Reconnecting...");
      SerialBT.connect(address);
    }
  }

  // Debug output
  Serial.print("X: "); Serial.print(x);
  Serial.print("  Y: "); Serial.print(y);
  Serial.print("  Detected: "); Serial.print(detectedCmd);
  Serial.print("  Stable: "); Serial.print(stableCount);
  Serial.print("  Last Sent: "); Serial.println(lastCmd);

  delay(80);
}
