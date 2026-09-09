#include <Arduino.h>
#include "config.h"
#include "Ultrasonic.h"
#include "Pose.h"
#include "NAMI.h"
#include "UartLink.h"

UltrasonicArray sensors;
PoseTracker     poseTracker;
NamiController  nami;
UartLink        uart;

unsigned long lastUpdateMs = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("NAMI Sensor Board booting...");

  sensors.begin();
  uart.begin();

  if (!poseTracker.begin()) {
    Serial.println("WARNING: MPU6050 not found. Check I2C wiring (SDA/SCL) and address 0x68.");
  } else {
    Serial.println("Calibrating gyro bias, keep the robot still...");
    poseTracker.resetHeadingBias();
    Serial.println("Calibration done.");
  }

  randomSeed(analogRead(0)); // used for the tie-break coin flip
  lastUpdateMs = millis();
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastUpdateMs) / 1000.0f;
  lastUpdateMs = now;

  // 1. Pull in any encoder feedback the Motor board has sent since last loop.
  long leftDelta = 0, rightDelta = 0;
  uart.pollEncoderReport(leftDelta, rightDelta);

  // 2. Update our position/heading estimate (IMU heading + encoder distance).
  poseTracker.update(dt, leftDelta, rightDelta);
  Pose pose = poseTracker.getPose();

  // 3. Read the sensor array.
  ScanResult scan = sensors.scan();

  // 4. Run NAMI decision logic.
  DriveCommand cmd = nami.update(scan, pose);

  // 5. Send the resulting command to the Motor board.
  uart.sendCommand(cmd);

  // Debug output — useful in the Wokwi serial monitor while tuning thresholds.
  Serial.printf(
    "[%s] F=%u L45=%u R45=%u L90=%u R90=%u IR=%d | pose=(%.1f,%.1f,%.0f) | %s -> speed=%u steer=%d\n",
    nami.lastStateLabel(), scan.front, scan.left45, scan.right45, scan.left90, scan.right90,
    scan.irFront, pose.x, pose.y, pose.headingDeg,
    nami.lastTriggerReason(), cmd.speedPct, (int)cmd.steer
  );
}
