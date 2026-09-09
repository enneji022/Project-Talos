// Motor A (DcMotor-15) - via IN1/IN2/EnA
const int IN1 = 27;
const int IN2 = 26;
const int ENA = 14;

// Motor B (DcMotor-16) - via IN3/IN4/EnB
const int IN3 = 25;
const int IN4 = 33;
const int ENB = 32;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void motorA(int speed, bool forward) {
  digitalWrite(IN1, forward ? HIGH : LOW);
  digitalWrite(IN2, forward ? LOW : HIGH);
  analogWrite(ENA, speed); // 0-255
}

void motorB(int speed, bool forward) {
  digitalWrite(IN3, forward ? HIGH : LOW);
  digitalWrite(IN4, forward ? LOW : HIGH);
  analogWrite(ENB, speed); // 0-255
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void loop() {
  // Both motors forward
  motorA(200, true);
  motorB(200, true);
  delay(2000);

  stopMotors();
  delay(1000);

  // Both motors backward
  motorA(200, false);
  motorB(200, false);
  delay(2000);

  stopMotors();
  delay(1000);

  // Spin in place: A forward, B backward
  motorA(180, true);
  motorB(180, false);
  delay(1500);

  stopMotors();
  delay(1000);
}