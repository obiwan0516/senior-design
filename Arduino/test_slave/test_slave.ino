void setup() {
    Serial.begin(9600); // Uno uses Serial for USB and TX/RX
}

void loop() {
    Serial.println("Hello from Uno!"); // Send message to Mega
    delay(1000);
}