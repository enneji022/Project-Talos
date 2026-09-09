const int MOTOR_A_PIN = 27;
const int MOTOR_B_PIN = 26;

void setup() {
  pinMode(MOTOR_A_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);
}

void loop() {
  // Both motors on at partial speed
  analogWrite(MOTOR_A_PIN, 200); // 0-255
  analogWrite(MOTOR_B_PIN, 200);
  delay(2000);

  // Both motors off
  analogWrite(MOTOR_A_PIN, 0);
  analogWrite(MOTOR_B_PIN, 0);
  delay(1000);

  // Both motors at full speed
  analogWrite(MOTOR_A_PIN, 255);
  analogWrite(MOTOR_B_PIN, 255);
  delay(2000);

  // Off again
  analogWrite(MOTOR_A_PIN, 0);
  analogWrite(MOTOR_B_PIN, 0);
  delay(1000);
}