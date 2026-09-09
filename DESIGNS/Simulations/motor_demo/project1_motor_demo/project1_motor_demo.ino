/*
 * PROJECT 1 — MOTOR ESP: independent 4-motor control demo
 * ----------------------------------------------------------
 * Drive scheme (per motor): 2 GPIOs, EN hardwired to 5V rail.
 *   - PWM on IN1, IN2 held LOW  -> Forward, speed = IN1 duty
 *   - PWM on IN2, IN1 held LOW  -> Reverse, speed = IN2 duty
 *   - Both LOW                  -> Coast (EN is always enabled,
 *                                  but IN1=IN2=0 means zero drive)
 *
 * IMPORTANT: EnA/EnB pins on both L298P chips must be hardwired to
 * the 5V rail in the circuit itself (NOT connected to any GPIO).
 * See the wiring correction table before flashing this.
 *
 * PWM API used here is the ESP32 Arduino core 3.x style:
 *   ledcAttach(pin, freq, resolution) + ledcWrite(pin, duty)
 * If you're on core 2.x instead, swap in:
 *   ledcSetup(channel, freq, resolution); ledcAttachPin(pin, channel);
 *   ledcWrite(channel, duty);
 * (channel management is manual on 2.x — ping me if you need that version)
 */

#include <Arduino.h>

// ---------------------------------------------------------------------
// PIN DEFINITIONS — matches your confirmed wiring
// ---------------------------------------------------------------------

// Motor indices: 0 = left-front, 1 = left-rear, 2 = right-front, 3 = right-rear
// TODO: confirm this matches DcMotor-13/14/15/16 physical positions in your build

const int MOTOR_COUNT = 4;

struct MotorPins {
    int in1;  // PWM forward
    int in2;  // PWM reverse
};

MotorPins motors[MOTOR_COUNT] = {
    { 13, 12 },  // Motor 0 — left driver, channel A  (DcMotor-13)
    { 14, 27 },  // Motor 1 — left driver, channel B  (DcMotor-15)
    { 26, 25 },  // Motor 2 — right driver, channel A (DcMotor-14)
    { 33, 32 },  // Motor 3 — right driver, channel B (DcMotor-16)
};

// ---------------------------------------------------------------------
// PWM CONFIG
// ---------------------------------------------------------------------

const int PWM_FREQ = 5000;       // Hz — above audible range, safe default for small gear motors
const int PWM_RESOLUTION = 8;    // bits -> duty range 0-255
const int MAX_DUTY = 255;

// ---------------------------------------------------------------------
// DEMO TIMING
// ---------------------------------------------------------------------

const unsigned long CYCLE_INTERVAL_MS = 2500;  // how often motor states change
unsigned long lastCycleTime = 0;

enum Direction { FWD, REV, STOP };

// ---------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Project 1 — 4 independent motor demo starting");

    for (int i = 0; i < MOTOR_COUNT; i++) {
        ledcAttach(motors[i].in1, PWM_FREQ, PWM_RESOLUTION);
        ledcAttach(motors[i].in2, PWM_FREQ, PWM_RESOLUTION);
        // start stopped
        ledcWrite(motors[i].in1, 0);
        ledcWrite(motors[i].in2, 0);
    }

    randomSeed(analogRead(0));  // TODO: use a genuinely floating ADC pin for better randomness
}

// ---------------------------------------------------------------------
// MOTOR CONTROL
// ---------------------------------------------------------------------

void setMotor(int index, Direction dir, int speed /* 0-255 */) {
    speed = constrain(speed, 0, MAX_DUTY);
    MotorPins &m = motors[index];

    switch (dir) {
        case FWD:
            ledcWrite(m.in1, speed);
            ledcWrite(m.in2, 0);
            break;
        case REV:
            ledcWrite(m.in1, 0);
            ledcWrite(m.in2, speed);
            break;
        case STOP:
        default:
            ledcWrite(m.in1, 0);
            ledcWrite(m.in2, 0);
            break;
    }
}

const char* dirName(Direction d) {
    switch (d) {
        case FWD: return "FWD";
        case REV: return "REV";
        default:  return "STOP";
    }
}

// ---------------------------------------------------------------------
// MAIN LOOP — random subset of motors, random speed/direction each cycle
// ---------------------------------------------------------------------

void loop() {
    unsigned long now = millis();

    if (now - lastCycleTime >= CYCLE_INTERVAL_MS) {
        lastCycleTime = now;

        Serial.println("--- new cycle ---");

        for (int i = 0; i < MOTOR_COUNT; i++) {
            bool active = random(0, 2) == 1;  // ~50% chance this motor runs this cycle

            if (active) {
                int speed = random(80, MAX_DUTY + 1);  // avoid very low PWM that may not overcome static friction
                Direction dir = (random(0, 2) == 1) ? FWD : REV;
                setMotor(i, dir, speed);

                Serial.print("Motor ");
                Serial.print(i);
                Serial.print(": ");
                Serial.print(dirName(dir));
                Serial.print(" speed=");
                Serial.println(speed);
            } else {
                setMotor(i, STOP, 0);
                Serial.print("Motor ");
                Serial.print(i);
                Serial.println(": STOP");
            }
        }
    }
}
