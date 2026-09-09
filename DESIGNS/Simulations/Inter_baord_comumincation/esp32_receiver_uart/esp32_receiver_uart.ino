#include <Arduino.h>

// Wired UART link to the sender ESP32
#define LINK_TX_PIN 26 // -> sender's GPIO26 (RX)
#define LINK_RX_PIN 25 // <- sender's GPIO25 (TX)

HardwareSerial LinkSerial(2);

const uint8_t FRAME_HEADER = 0xAA;

struct Motor {
    uint8_t pin;
    bool state;
};

// Same pins as your original sketch/schematic
Motor motors[] = {
    {4,  false},
    {16, false},
    {17, false},
    {18, false}
};
const uint8_t NUM_MOTORS = sizeof(motors) / sizeof(motors[0]);

struct MotorCommand {
  uint8_t header;
  bool motorState[4]; // must match the sender's struct exactly
};

unsigned long lastCommandMillis = 0;
const unsigned long COMMAND_TIMEOUT = 1000; // ms - stop motors if link goes quiet

void setup() {
  Serial.begin(115200);
  LinkSerial.begin(115200, SERIAL_8N1, LINK_RX_PIN, LINK_TX_PIN);

  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    pinMode(motors[i].pin, OUTPUT);
    digitalWrite(motors[i].pin, LOW);
  }
}

void loop() {
  // Read frames off the wire, resyncing on the header byte if the
  // stream ever gets misaligned (e.g. after a reset mid-transmission).
  while (LinkSerial.available() >= (int)sizeof(MotorCommand)) {
    if (LinkSerial.peek() != FRAME_HEADER) {
      LinkSerial.read(); // discard one byte and try again
      continue;
    }

    MotorCommand cmd;
    LinkSerial.readBytes((uint8_t *)&cmd, sizeof(cmd));

    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
      motors[i].state = cmd.motorState[i];
      digitalWrite(motors[i].pin, motors[i].state ? HIGH : LOW);
    }
    lastCommandMillis = millis();
  }

  // Failsafe: if the wire goes quiet, force all motors off rather than
  // leaving them stuck in their last state.
  if (millis() - lastCommandMillis > COMMAND_TIMEOUT) {
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
      if (motors[i].state) {
        motors[i].state = false;
        digitalWrite(motors[i].pin, LOW);
      }
    }
  }
}
