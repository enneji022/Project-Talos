#include <Arduino.h>

// Sensor Pin Definitions
struct SensorPins {
    int trig;
    int echo;
};

enum SensorIndex {
    FRONT_LEFT = 0,
    FRONT,
    FRONT_RIGHT,
    LEFT,
    RIGHT,
    TOTAL_SENSORS
};

const SensorPins SENSORS[TOTAL_SENSORS] = {
    {13, 34}, // FRONT_LEFT
    {12, 35}, // FRONT
    {14, 32}, // FRONT_RIGHT
    {27, 33}, // LEFT
    {26, 25}  // RIGHT
};

// Distance readings storage (in centimeters)
float distances[TOTAL_SENSORS];

// Configuration Constants
const float WALL_THRESHOLD = 30.0; // Distance in cm considered "near wall"
const float STOP_DISTANCE  = 15.0; // Distance in cm to halt forward progress

// Helper function to query a single HC-SR04 sensor
float getDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Timeout after 30ms (~5 meters max range)
    long duration = pulseIn(echoPin, HIGH, 30000); 
    if (duration == 0) return 400.0; // Return out-of-range value if no pulse

    return (duration * 0.0343) / 2.0;
}

// Read all 5 sensors sequentially to prevent sonic crosstalk
void updateAllSensors() {
    for (int i = 0; i < TOTAL_SENSORS; ++i) {
        distances[i] = getDistance(SENSORS[i].trig, SENSORS[i].echo);
        delay(10); // Small pause to let sound echoes dissipate
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize all GPIOs
    for (int i = 0; i < TOTAL_SENSORS; ++i) {
        pinMode(SENSORS[i].trig, OUTPUT);
        pinMode(SENSORS[i].echo, INPUT);
    }

    Serial.println("--- Self-Driving Car 5-Sensor System Initialized ---");
}

void loop() {
    updateAllSensors();

    // Output raw distances for telemetry/debugging
    Serial.printf("FL: %5.1f cm | F: %5.1f cm | FR: %5.1f cm | L: %5.1f cm | R: %5.1f cm\n",
                  distances[FRONT_LEFT], distances[FRONT], distances[FRONT_RIGHT],
                  distances[LEFT], distances[RIGHT]);

    // Simple reactive steering decision logic
    if (distances[FRONT] < STOP_DISTANCE || 
        distances[FRONT_LEFT] < STOP_DISTANCE || 
        distances[FRONT_RIGHT] < STOP_DISTANCE) {
        
        Serial.println("-> ACTION: Obstacle ahead! STOP and decide turn direction.");
    } else {
        // Calculate lateral error to center between side walls
        float error = distances[LEFT] - distances[RIGHT];

        if (abs(error) < 5.0) {
            Serial.println("-> ACTION: Centered. Drive STRAIGHT.");
        } else if (error > 0) {
            Serial.println("-> ACTION: Too close to RIGHT wall. Steer LEFT.");
        } else {
            Serial.println("-> ACTION: Too close to LEFT wall. Steer RIGHT.");
        }
    }

    delay(100); // Main loop control rate (~10Hz updates)
}