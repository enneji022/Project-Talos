#pragma once
#include <Arduino.h>
#include "config.h"
#include "NAMI.h"

// ============================================================================
// LOCKED Sensor<->Motor UART protocol (confirmed with team lead 2026-09-09).
//
//   Sensor board -> Motor board:   binary SteerCommand struct (below).
//     NAMI's {speedPct, steer} is translated to differential {leftSpeed,
//     rightSpeed} here, since Motor board only knows per-side PWM, not the
//     concept of "steer". TURN_SPEED_SCALE (config.h) sets how much slower
//     the inside wheel spins during a turn.
//
//   Motor board -> Sensor board:   "E,<leftTickDelta>,<rightTickDelta>\n"
//     Encoder ticks accumulated since the last report (signed, can be
//     negative if a wheel is momentarily reversed). Sent periodically
//     (~30ms) so the Sensor board can dead-reckon pose. This direction is
//     still plain ASCII — only the Sensor->Motor direction is binary.
// ============================================================================

// Must byte-match Motor board's SteerCommand exactly (see motor_board.ino).
struct SteerCommand {
  int16_t leftSpeed;   // -255..255, negative = reverse
  int16_t rightSpeed;  // -255..255, negative = reverse
  uint8_t checksum;    // XOR of all preceding bytes
};

class UartLink {
public:
  void begin();
  void sendCommand(const DriveCommand& cmd);

  // Call every loop. Returns true if a fresh encoder report was parsed,
  // filling leftDelta/rightDelta. Returns false otherwise (nothing new).
  bool pollEncoderReport(long& leftDelta, long& rightDelta);

private:
  String rxBuffer_;
};
