#include <Arduino.h>

struct Motor {
    uint8_t pin;
    unsigned long interval;     // Toggle period in milliseconds
    unsigned long previousMillis; // Last state change timestamp
    bool state;                 // Current state (HIGH / LOW)
};

// Define pins and unique timing intervals for each motor
Motor motors[] = {
    {4,  1000, 0, false}, // Motor 1: Toggles every 1.0 second
    {16, 2000, 0, false}, // Motor 2: Toggles every 2.0 seconds
    {17, 3000, 0, false}, // Motor 3: Toggles every 3.0 seconds
    {18, 4000, 0, false}  // Motor 4: Toggles every 4.0 seconds
};

const uint8_t NUM_MOTORS = sizeof(motors) / sizeof(motors[0]);

void setup() {
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        pinMode(motors[i].pin, OUTPUT);
        digitalWrite(motors[i].pin, LOW);
    }
}

void loop() {
    unsigned long currentMillis = millis();

    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        // Check if individual interval has elapsed
        if (currentMillis - motors[i].previousMillis >= motors[i].interval) {
            motors[i].previousMillis = currentMillis;  // Reset timer
            motors[i].state = !motors[i].state;        // Toggle state
            
            digitalWrite(motors[i].pin, motors[i].state ? HIGH : LOW);
        }
    }
}