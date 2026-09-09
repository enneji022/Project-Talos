int ledPin = 2;

void setup(){
    pinMode(ledPin, OUTPUT);
    
    Serial.begin(115200);
}

void loop(){
    Serial.print("Hello");
    digitalWrite(ledPin, HIGH);

    delay(500);

    Serial.println("World");
    digitalWrite(ledPin, LOW);

    delay(500);
}