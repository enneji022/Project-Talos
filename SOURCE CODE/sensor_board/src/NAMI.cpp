#include "NAMI.h"

static Steer opposite(Steer s) {
  if (s == Steer::LEFT) return Steer::RIGHT;
  if (s == Steer::RIGHT) return Steer::LEFT;
  return Steer::STRAIGHT;
}

static float headingDiffDeg(float a, float b) {
  float d = fmodf(fabsf(a - b), 360.0f);
  return d > 180.0f ? 360.0f - d : d;
}

bool NamiController::shouldEnterDecision(const ScanResult& s) const {
  if (s.front <= OBSTACLE_TRIGGER_CM || s.irFront) return true; // obstacle ahead
  if (abs((int)s.left90 - (int)s.right90) >= CORNER_DIVERGENCE_CM) return true; // corner: one wall recedes/vanishes
  return false;
}

Steer NamiController::chooseDirection(const ScanResult& s, const Pose& pose) {
  // Blend the 45 and 90 degree readings on each side into one "space available" figure.
  float leftSpace  = (s.left45 + s.left90) / 2.0f;
  float rightSpace = (s.right45 + s.right90) / 2.0f;

  float larger = max(leftSpace, rightSpace);
  float diff   = fabsf(leftSpace - rightSpace);

  bool roughlyEqual = (larger > 0) && (diff / larger) <= TIE_BREAK_PCT;

  if (roughlyEqual) {
    triggerReason_ = "tie-break (random)";
    return (random(0, 2) == 0) ? Steer::LEFT : Steer::RIGHT;
  }

  if (leftSpace > rightSpace) {
    triggerReason_ = "more room left";
    return Steer::LEFT;
  } else {
    triggerReason_ = "more room right";
    return Steer::RIGHT;
  }
}

DecisionMemory* NamiController::findNearbyMemory(const Pose& pose) {
  for (auto& m : history_) {
    float dx = pose.x - m.x;
    float dy = pose.y - m.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist <= LOOP_RADIUS_CM && headingDiffDeg(pose.headingDeg, m.headingDeg) <= LOOP_HEADING_TOL_DEG) {
      return &m;
    }
  }
  return nullptr;
}

DriveCommand NamiController::update(const ScanResult& s, const Pose& pose) {
  if (!shouldEnterDecision(s)) {
    inDecision_ = false;
    triggerReason_ = "-";
    return { CRUISE_SPEED_PCT, Steer::STRAIGHT };
  }

  inDecision_ = true;

  // First, work out what NAMI would pick based on space alone.
  Steer picked = chooseDirection(s, pose);

  // Loop-prevention overrides the space-based pick if we've been here before.
  DecisionMemory* prev = findNearbyMemory(pose);
  if (prev != nullptr) {
    picked = opposite(prev->lastDecision);
    triggerReason_ = "loop detected -> forced alternate";
    prev->lastDecision = picked;
    prev->x = pose.x;
    prev->y = pose.y;
    prev->headingDeg = pose.headingDeg;
  } else {
    if (history_.size() >= LOOP_HISTORY_MAX) {
      history_.erase(history_.begin()); // drop oldest so memory doesn't grow forever
    }
    history_.push_back({ pose.x, pose.y, pose.headingDeg, picked });
  }

  return { DECISION_SPEED_PCT, picked };
}
