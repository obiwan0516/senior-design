const int pwmPin = 11; // Pin for PWM output

void setup() {
  pinMode(pwmPin, OUTPUT); // Set the PWM pin as an output

  // Configure Timer 2 for Fast PWM mode
  TCCR2A = B10100011; // Fast PWM, non-inverting mode
  // No prescaling (1)
  // TCCR2B = (TCCR2B & B11111000) | B00000000; // Prescaler = 1

  // Prescale by 8; 7.797kHz
  TCCR2B = (TCCR2B & B11111000) | B00000010; // Prescaler = 8

  // Prescale by 32; 62.40kHz
  // TCCR2B = (TCCR2B & B11111000) | B00000001; // Prescaler = 32

  // Prescale by 64; 1.951kHz
  // TCCR2B = (TCCR2B & B11111000) | B00000011; // Prescaler = 64

  // Prescale by 128; 975Hz
  // TCCR2B = (TCCR2B & B11111000) | B00000100; // Prescaler = 128

  // Prescale by 256; 487Hz
  // TCCR2B = (TCCR2B & B11111000) | B00000101; // Prescaler = 256

  // Prescale by 1024; 244Hz
  // TCCR2B = (TCCR2B & B11111000) | B00000110; // Prescaler = 1024
  analogWrite(pwmPin, 128); // 50% duty cycle
}

void loop() {
  // Main loop code here
}