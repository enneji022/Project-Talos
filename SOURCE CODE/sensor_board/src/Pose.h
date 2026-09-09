#pragma once
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "config.h"

// Tracks the robot's position/heading for NAMI's loop-detection requirement.
//
// WHY NOT JUST THE IMU: an accelerometer alone gives you acceleration, and
// turning that into position means integrating twice. Any small sensor bias
// balloons into meters of drift within a few seconds — not usable here.
// Instead: heading comes from integrating the IMU's gyro Z-axis (much more
// stable over short racing timescales), and distance traveled comes from
// wheel encoder ticks reported by the Motor board over UART. This combo
// ("dead reckoning") is the standard way small robots estimate pose without GPS.
struct Pose {
  float x = 0, y = 0;       // cm, arbitrary origin at power-on
  float headingDeg = 0;     // 0 = whichever way the robot was facing at start
};

class PoseTracker {
public:
  bool begin(); // returns false if the IMU wasn't found
  void update(float dtSeconds, long leftTickDelta, long rightTickDelta);
  Pose getPose() const { return pose_; }
  void resetHeadingBias(); // call once at rest during setup() to zero gyro drift

private:
  Adafruit_MPU6050 mpu_;
  Pose pose_;
  float gyroZBiasDegPerSec_ = 0;
};
