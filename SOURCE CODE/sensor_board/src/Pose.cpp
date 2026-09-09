#include "Pose.h"

bool PoseTracker::begin() {
  Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
  if (!mpu_.begin()) {
    return false;
  }
  mpu_.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu_.setFilterBandwidth(MPU6050_BAND_21_HZ);
  return true;
}

void PoseTracker::resetHeadingBias() {
  // Average a few readings while stationary to null out gyro drift/offset.
  const int N = 100;
  float sum = 0;
  sensors_event_t a, g, temp;
  for (int i = 0; i < N; i++) {
    mpu_.getEvent(&a, &g, &temp);
    sum += g.gyro.z * (180.0f / PI); // rad/s -> deg/s
    delay(5);
  }
  gyroZBiasDegPerSec_ = sum / N;
}

void PoseTracker::update(float dtSeconds, long leftTickDelta, long rightTickDelta) {
  sensors_event_t a, g, temp;
  mpu_.getEvent(&a, &g, &temp);

  float gyroZDegPerSec = (g.gyro.z * (180.0f / PI)) - gyroZBiasDegPerSec_;
  pose_.headingDeg += gyroZDegPerSec * dtSeconds;
  // wrap to [0, 360)
  if (pose_.headingDeg >= 360.0f) pose_.headingDeg -= 360.0f;
  if (pose_.headingDeg < 0.0f)    pose_.headingDeg += 360.0f;

  // Distance traveled this tick, from average of both wheels' encoder deltas.
  float avgTicks = (leftTickDelta + rightTickDelta) / 2.0f;
  float distanceCm = avgTicks * CM_PER_TICK;

  float headingRad = pose_.headingDeg * (PI / 180.0f);
  pose_.x += distanceCm * cos(headingRad);
  pose_.y += distanceCm * sin(headingRad);
}
