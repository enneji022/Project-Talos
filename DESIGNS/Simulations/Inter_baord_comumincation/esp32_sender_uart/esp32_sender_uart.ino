#include <Arduino.h>

// Wired UART link to the receiver ESP32
#define LINK_TX_PIN 25 // -> receiver's GPIO25 (RX)
#define LINK_RX_PIN 26 // <- receiver's GPIO26 (TX)

HardwareSerial LinkSerial(2); // use the ESP32's UART2 peripheral

const uint8_t FRAME_HEADER = 0xAA; // marks the start of a valid frame

struct MotorCommand {
  uint8_t header;
  bool motorState[4]; // one entry per motor: pins 4, 16, 17, 18
};

void setup() {
  Serial.begin(115200);
  LinkSerial.begin(115200, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);
}

void loop() {
  // Demo pattern: reproduces the same independent-timer feel as the
  // original sketch, but computed here and sent to the receiver over
  // the wire instead of running locally on the motor board.
  unsigned long t = millis();

  MotorCommand cmd;
  cmd.header = FRAME_HEADER;
  cmd.motorState[0] = (t / 1000) % 2; // was pin 4,  1.0s period
  cmd.motorState[1] = (t / 2000) % 2; // was pin 16, 2.0s period
  cmd.motorState[2] = (t / 3000) % 2; // was pin 17, 3.0s period
  cmd.motorState[3] = (t / 4000) % 2; // was pin 18, 4.0s period

  LinkSerial.write((uint8_t *)&cmd, sizeof(cmd));
  delay(200); // send updates ~5x/second so the receiver's failsafe never trips
}
