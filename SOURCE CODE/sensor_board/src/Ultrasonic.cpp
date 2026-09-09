#include "Ultrasonic.h"

void UltrasonicArray::begin() {
  pinMode(US_FRONT.trig, OUTPUT);   pinMode(US_FRONT.echo, INPUT);
  pinMode(US_LEFT45.trig, OUTPUT);  pinMode(US_LEFT45.echo, INPUT);
  pinMode(US_RIGHT45.trig, OUTPUT); pinMode(US_RIGHT45.echo, INPUT);
  pinMode(US_LEFT90.trig, OUTPUT);  pinMode(US_LEFT90.echo, INPUT);
  pinMode(US_RIGHT90.trig, OUTPUT); pinMode(US_RIGHT90.echo, INPUT);

  pinMode(IR_FRONT_PIN, INPUT_PULLUP); // digital IR module: LOW = obstacle (module-dependent, invert if needed)
}

uint16_t UltrasonicArray::readOne(const UltrasonicPins& p) {
  digitalWrite(p.trig, LOW);
  delayMicroseconds(2);
  digitalWrite(p.trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(p.trig, LOW);

  unsigned long duration = pulseIn(p.echo, HIGH, US_TIMEOUT_US);
  if (duration == 0) {
    return US_MAX_DIST_CM; // no echo = treat as clear / out of range
  }
  // speed of sound ~343 m/s -> 0.0343 cm/us, round trip so /2
  uint16_t distanceCm = (uint16_t)(duration * 0.0343f / 2.0f);
  if (distanceCm > US_MAX_DIST_CM) distanceCm = US_MAX_DIST_CM;
  return distanceCm;
}

ScanResult UltrasonicArray::scan() {
  ScanResult r;
  r.front   = readOne(US_FRONT);
  delayMicroseconds(US_POLL_GAP_US);
  r.left45  = readOne(US_LEFT45);
  delayMicroseconds(US_POLL_GAP_US);
  r.right45 = readOne(US_RIGHT45);
  delayMicroseconds(US_POLL_GAP_US);
  r.left90  = readOne(US_LEFT90);
  delayMicroseconds(US_POLL_GAP_US);
  r.right90 = readOne(US_RIGHT90);

  // Digital IR: assumes active-LOW module (common for these boards).
  // If your module is active-HIGH, flip this comparison.
  r.irFront = (digitalRead(IR_FRONT_PIN) == LOW);

  return r;
}
