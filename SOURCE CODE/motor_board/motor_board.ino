/*
 * MOTOR BOARD FIRMWARE — Robo Grand Prix
 * ----------------------------------------------------------
 * Role: "dumb executor". Receives a steering command over UART2
 * from the Sensor board (which runs the NAMI decision logic) and
 * drives 4 independent motors via 2x TB6612FNG in skid-steer mode.
 *
 * DRIVER: TB6612FNG (NOT L298P — earlier drafts assumed L298P's
 * 2-pin/motor scheme with EN hardwired to 5V. TB6612FNG uses 3
 * pins/motor: a dedicated PWM pin for speed, plus IN1/IN2 logic
 * pins for direction. STBY is shared across both driver chips.)
 *
 * ENCODERS: single-channel (A only) per wheel. Direction is known
 * from what we commanded, so we only need pulse rate for speed
 * feedback, not quadrature direction sensing. If your build uses
 * full A+B quadrature encoders, tell your Claude and the pin map
 * below needs revisiting — ESP32 only has 4 safe input-only pins
 * (34/35/36/39), so full quadrature costs 4 more pins than we have
 * budgeted.
 *
 * UART PROTOCOL (Sensor -> Motor):
 * This firmware expects a STEERING COMMAND, not raw sensor data.
 * NOTE: as of the current sensor_sim.ino draft, the Sensor board is
 * still sending raw distances, not a decision. That mismatch needs
 * reconciling with Claude #1 / the supervisor chat before final
 * integration — this file defines what Motor board WILL expect once
 * that's resolved. Update parseCommand() if the agreed struct changes.
 *
 * PIN MAP (see handoff doc / build breakdown for reasoning):
 *   Motor 0 (LF): PWM=4  IN1=5  IN2=18
 *   Motor 1 (LR): PWM=19 IN1=21 IN2=22
 *   Motor 2 (RF): PWM=23 IN1=25 IN2=26
 *   Motor 3 (RR): PWM=27 IN1=32 IN2=33
 *   STBY (shared):     13
 *   UART2 to Sensor:   RX2=16  TX2=17
 *   E-stop sense:      14
 *   Encoders (A only): 34, 35, 36, 39   (M0, M1, M2, M3)
 *
 * !! PLACEHOLDER PINS !! — these are NOT yet confirmed against your
 * physical wiring. Do not power motors until you've checked every
 * pin against the actual TB6612FNG wiring on the board. Getting a
 * PWM pin swapped with an IN1/IN2 pin won't damage anything, but
 * getting VCC/GND pins wrong on the driver chip WILL.
 */

#include <Arduino.h>

// ---------------------------------------------------------------------
// PIN DEFINITIONS
// ---------------------------------------------------------------------

struct MotorPins {
    uint8_t pwm;
    uint8_t in1;
    uint8_t in2;
    uint8_t channel; // only used on older arduino-esp32 cores (see PWM helpers below)
};

const uint8_t MOTOR_COUNT = 4;
// Index: 0 = left-front, 1 = left-rear, 2 = right-front, 3 = right-rear
MotorPins motors[MOTOR_COUNT] = {
    { 4,  5,  18, 0 },  // M0 LF
    { 19, 21, 22, 1 },  // M1 LR
    { 23, 25, 26, 2 },  // M2 RF
    { 27, 32, 33, 3 },  // M3 RR
};

// ---------------------------------------------------------------------
// PWM COMPATIBILITY SHIM
// arduino-esp32 core 3.x uses pin-based LEDC: ledcAttach(pin,freq,res),
// ledcWrite(pin,duty). Older 2.x cores use channel-based LEDC instead:
// ledcSetup(channel,freq,res), ledcAttachPin(pin,channel),
// ledcWrite(channel,duty). ESP_ARDUINO_VERSION_MAJOR (defined by the
// core itself) tells us which one is installed, so this compiles either
// way without you needing to know or care which core version you have.
// ---------------------------------------------------------------------
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #define PWM_ATTACH(m, freq, res) ledcAttach((m).pwm, (freq), (res))
  #define PWM_WRITE(m, duty)       ledcWrite((m).pwm, (duty))
#else
  #define PWM_ATTACH(m, freq, res) do { ledcSetup((m).channel, (freq), (res)); ledcAttachPin((m).pwm, (m).channel); } while (0)
  #define PWM_WRITE(m, duty)       ledcWrite((m).channel, (duty))
#endif

const uint8_t STBY_PIN = 13;

const uint8_t ENCODER_PINS[MOTOR_COUNT] = { 34, 35, 36, 39 };

const uint8_t ESTOP_SENSE_PIN = 14;   // reads state of the hardware e-stop line
                                       // (does NOT replace the hardware relay cutoff —
                                       // that's the actual safety mechanism. this is so
                                       // firmware can also refuse to command motion.)

#define RX2_PIN 16
#define TX2_PIN 17

// ---------------------------------------------------------------------
// PWM CONFIG
// ---------------------------------------------------------------------

const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;   // 0-255
const int MAX_DUTY = 255;

// ---------------------------------------------------------------------
// UART COMMAND PROTOCOL (Sensor board -> Motor board)
// Placeholder struct — confirm/update once Claude #1's NAMI output
// format is finalized and reconciled in the supervisor chat.
// ---------------------------------------------------------------------

struct SteerCommand {
    int16_t leftSpeed;   // -255..255, negative = reverse
    int16_t rightSpeed;  // -255..255, negative = reverse
    uint8_t checksum;
};

const unsigned long COMMAND_TIMEOUT_MS = 500;  // failsafe: if no command
                                                // received in this window,
                                                // stop all motors
unsigned long lastCommandMillis = 0;

// -- Encoder reporting back to Sensor board (needed for its dead-reckoning) --
uint32_t lastEncoderCounts[MOTOR_COUNT] = { 0, 0, 0, 0 };
unsigned long lastEncoderReportMs = 0;
const unsigned long ENCODER_REPORT_MS = 30;
SteerCommand lastCmd = { 0, 0, 0 }; // used to sign single-channel encoder deltas

uint8_t calcChecksum(const SteerCommand& cmd) {
    uint8_t sum = 0;
    const uint8_t* ptr = (const uint8_t*)&cmd;
    for (size_t i = 0; i < sizeof(SteerCommand) - 1; i++) sum ^= ptr[i];
    return sum;
}

// ---------------------------------------------------------------------
// ENCODER PULSE COUNTING
// ---------------------------------------------------------------------

volatile uint32_t encoderCounts[MOTOR_COUNT] = { 0, 0, 0, 0 };

void IRAM_ATTR encISR0() { encoderCounts[0]++; }
void IRAM_ATTR encISR1() { encoderCounts[1]++; }
void IRAM_ATTR encISR2() { encoderCounts[2]++; }
void IRAM_ATTR encISR3() { encoderCounts[3]++; }

// ---------------------------------------------------------------------
// MOTOR CONTROL
// ---------------------------------------------------------------------

void setMotor(uint8_t index, int16_t speed /* -255..255 */) {
    speed = constrain(speed, -MAX_DUTY, MAX_DUTY);
    MotorPins &m = motors[index];

    if (speed >= 0) {
        digitalWrite(m.in1, HIGH);
        digitalWrite(m.in2, LOW);
        PWM_WRITE(m, speed);
    } else {
        digitalWrite(m.in1, LOW);
        digitalWrite(m.in2, HIGH);
        PWM_WRITE(m, -speed);
    }
}

void stopAllMotors() {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        digitalWrite(motors[i].in1, LOW);
        digitalWrite(motors[i].in2, LOW);
        PWM_WRITE(motors[i], 0);
    }
}

// Skid-steer: left side motors mirror leftSpeed, right side mirror rightSpeed
void applySteerCommand(const SteerCommand& cmd) {
    setMotor(0, cmd.leftSpeed);   // LF
    setMotor(1, cmd.leftSpeed);   // LR
    setMotor(2, cmd.rightSpeed);  // RF
    setMotor(3, cmd.rightSpeed);  // RR
}

// ---------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

    pinMode(STBY_PIN, OUTPUT);
    digitalWrite(STBY_PIN, HIGH);  // enable both driver chips

    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        pinMode(motors[i].in1, OUTPUT);
        pinMode(motors[i].in2, OUTPUT);
        PWM_ATTACH(motors[i], PWM_FREQ, PWM_RESOLUTION);
        PWM_WRITE(motors[i], 0);
    }

    // Encoders: input-only pins, no internal pullup available on 34-39.
    // Fine if your encoder module has push-pull output (most do); if you
    // see noisy/floating readings, that's why.
    pinMode(ENCODER_PINS[0], INPUT);
    pinMode(ENCODER_PINS[1], INPUT);
    pinMode(ENCODER_PINS[2], INPUT);
    pinMode(ENCODER_PINS[3], INPUT);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PINS[0]), encISR0, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PINS[1]), encISR1, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PINS[2]), encISR2, RISING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PINS[3]), encISR3, RISING);

    pinMode(ESTOP_SENSE_PIN, INPUT);

    stopAllMotors();
    lastCommandMillis = millis();

    Serial.println("--- Motor ESP32 Initialized ---");
}

// ---------------------------------------------------------------------
// UART RECEIVE
// ---------------------------------------------------------------------

bool readCommand(SteerCommand& out) {
    if (Serial2.available() < (int)sizeof(SteerCommand)) return false;

    SteerCommand cmd;
    Serial2.readBytes((uint8_t*)&cmd, sizeof(SteerCommand));

    if (calcChecksum(cmd) != cmd.checksum) {
        Serial.println("UART checksum mismatch, dropping packet");
        return false;
    }

    out = cmd;
    lastCmd = cmd;
    return true;
}

// ---------------------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------------------

void loop() {
    bool estopTripped = (digitalRead(ESTOP_SENSE_PIN) == HIGH);
    // NOTE: confirm actual polarity (HIGH = tripped vs LOW = tripped)
    // against your e-stop circuit before relying on this in testing.

    SteerCommand cmd;
    if (readCommand(cmd)) {
        lastCommandMillis = millis();
        if (!estopTripped) {
            applySteerCommand(cmd);
        }
    }

    bool commandStale = (millis() - lastCommandMillis) > COMMAND_TIMEOUT_MS;
    if (estopTripped || commandStale) {
        stopAllMotors();
    }

    // Report encoder deltas back to the Sensor board so it can dead-reckon pose.
    // Single-channel (A only) encoders can't sense direction themselves, so we
    // sign the delta using whatever direction we last commanded that side.
    if (millis() - lastEncoderReportMs > ENCODER_REPORT_MS) {
        lastEncoderReportMs = millis();

        long leftDelta  = ((long)(encoderCounts[0] - lastEncoderCounts[0]) +
                            (long)(encoderCounts[1] - lastEncoderCounts[1])) / 2;
        long rightDelta = ((long)(encoderCounts[2] - lastEncoderCounts[2]) +
                            (long)(encoderCounts[3] - lastEncoderCounts[3])) / 2;

        if (lastCmd.leftSpeed  < 0) leftDelta  = -leftDelta;
        if (lastCmd.rightSpeed < 0) rightDelta = -rightDelta;

        for (uint8_t i = 0; i < MOTOR_COUNT; i++) lastEncoderCounts[i] = encoderCounts[i];

        Serial2.printf("E,%ld,%ld\n", leftDelta, rightDelta);
    }

    // Lightweight telemetry for debugging in Wokwi/PlatformIO monitor
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        lastPrint = millis();
        Serial.print("Enc: ");
        for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
            Serial.print(encoderCounts[i]);
            Serial.print(" ");
        }
        Serial.print("| e-stop: ");
        Serial.println(estopTripped ? "TRIPPED" : "clear");
    }
}
