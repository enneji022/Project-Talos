#pragma once
#include <Arduino.h>
#include "config.h"

// Distances in cm from all 5 sensors, one snapshot of the scan.
struct ScanResult {
  uint16_t front;
  uint16_t left45;
  uint16_t right45;
  uint16_t left90;
  uint16_t right90;
  bool     irFront; // true = obstacle confirmed very close (backup layer)
};

class UltrasonicArray {
public:
  void begin();

  // Performs one full sequential sweep of all 5 sensors (blocking).
  // Sequential polling is deliberate: firing HC-SR04s simultaneously
  // causes cross-talk between echoes (see build doc, section 2).
  ScanResult scan();

private:
  uint16_t readOne(const UltrasonicPins& p);
};
