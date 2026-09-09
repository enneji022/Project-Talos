#pragma once
#include <Arduino.h>

// ============================================================================
// PIN MAP — placeholder assignments for the SENSOR BOARD (decision brain).
// These avoid ESP32 strapping pins (0, 2, 12, 15) and use the input-only
// pins (34, 35, 36, 39) for ECHO lines since those never need to be outputs.
// TEAM LEAD: confirm these match your actual wiring before flashing real HW.
// ============================================================================

// --- Ultrasonic array (5x, sequential polling to avoid cross-talk) ---
// Angles per the build doc: center 0°, ±45°, ±90°
struct UltrasonicPins { uint8_t trig; uint8_t echo; };

static const UltrasonicPins US_FRONT    = {25, 34}; //   0°
static const UltrasonicPins US_LEFT45   = {26, 35}; // -45°
static const UltrasonicPins US_RIGHT45  = {27, 36}; // +45°
static const UltrasonicPins US_LEFT90   = {14, 39}; // -90°
static const UltrasonicPins US_RIGHT90  = {13,  4}; // +90°

// --- Digital IR backup (close-range confirmation, "something's there") ---
static const uint8_t IR_FRONT_PIN = 5;
// static const uint8_t IR_EDGE_PIN = 18; // optional second IR, uncomment if used

// --- IMU (I2C, its own bus, separate from any inter-board I2C) ---
static const uint8_t IMU_SDA_PIN = 21;
static const uint8_t IMU_SCL_PIN = 22;

// --- UART link to Motor board (Serial2) ---
static const uint8_t UART_TX_PIN = 17;
static const uint8_t UART_RX_PIN = 16;
static const uint32_t UART_BAUD  = 115200;

// ============================================================================
// TUNING CONSTANTS — from the handoff doc's "implementation defaults to
// confirm/tune". Adjust these once real sensor noise is measured.
// ============================================================================

// Ultrasonic sensing
static const uint32_t US_TIMEOUT_US   = 20000;  // ~3.4m range ceiling, plenty for a 50x50cm track
static const uint16_t US_MAX_DIST_CM  = 300;     // clamp readings beyond this (no echo / out of range)
static const uint32_t US_POLL_GAP_US  = 8000;    // gap between sensors in the sequential scan

// Speed staging (values are PWM duty % sent to the Motor board, 0-100)
static const uint8_t CRUISE_SPEED_PCT   = 80;
static const uint8_t DECISION_SPEED_PCT = 35;

// Obstacle / corner trigger thresholds
static const uint16_t OBSTACLE_TRIGGER_CM = 40;  // front distance that forces slow-down + decision
static const uint16_t CORNER_DIVERGENCE_CM = 60; // |left90 - right90| beyond this => treat as a corner

// "Roughly equal" tie-break threshold: within this % of each other -> coin flip
static const float TIE_BREAK_PCT = 0.125f; // midpoint of the 10-15% range given

// Loop detection ("same location" for forcing an alternate decision)
static const float LOOP_RADIUS_CM     = 15.0f;
static const float LOOP_HEADING_TOL_DEG = 30.0f;
static const size_t LOOP_HISTORY_MAX  = 64; // cap memory use; oldest entries drop off

// Differential-drive translation (NAMI's speed%+steer -> Motor board's left/right PWM)
static const float TURN_SPEED_SCALE = 0.35f; // inside wheel speed as a fraction of outside wheel during a turn

// Dead-reckoning: TUNE THESE against your real wheels/encoders once built
static const float WHEEL_DIAMETER_CM   = 6.5f;
static const float ENCODER_TICKS_PER_REV = 20.0f; // placeholder, set to real encoder CPR
static const float CM_PER_TICK = (PI * WHEEL_DIAMETER_CM) / ENCODER_TICKS_PER_REV;
