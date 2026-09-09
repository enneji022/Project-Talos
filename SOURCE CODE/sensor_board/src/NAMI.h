#pragma once
#include <Arduino.h>
#include <vector>
#include "config.h"
#include "Ultrasonic.h"
#include "Pose.h"

enum class Steer : int8_t { LEFT = -1, STRAIGHT = 0, RIGHT = 1 };

struct DriveCommand {
  uint8_t speedPct; // 0-100, sent to Motor board
  Steer   steer;
};

// One remembered decision, for the loop-prevention rule:
// "if the robot detects it has returned to a place it's already been,
//  it must make a different decision than it made last time at that spot."
struct DecisionMemory {
  float x, y, headingDeg;
  Steer lastDecision;
};

class NamiController {
public:
  // scan: this cycle's sensor snapshot
  // pose: current estimated position/heading
  // Returns the drive command to send to the Motor board over UART.
  DriveCommand update(const ScanResult& scan, const Pose& pose);

  // For the judge-facing explanation / debugging: was the last cycle
  // in cruise or in an active decision, and why.
  const char* lastStateLabel() const { return inDecision_ ? "DECISION" : "CRUISE"; }
  const char* lastTriggerReason() const { return triggerReason_; }

private:
  bool  inDecision_ = false;
  const char* triggerReason_ = "-";
  std::vector<DecisionMemory> history_;

  bool shouldEnterDecision(const ScanResult& s) const;
  Steer chooseDirection(const ScanResult& s, const Pose& pose);
  DecisionMemory* findNearbyMemory(const Pose& pose);
};
