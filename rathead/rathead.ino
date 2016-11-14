#define IR_LED_PIN 3
#define INDICATOR_PIN 13

#define NUM_GATES 4

unsigned led_cycles = 0;
bool led_enabled = true;
bool led_state = false;
ISR(TIMER1_COMPA_vect) {
  if (led_enabled) {
    led_state = !led_state;
    digitalWrite(IR_LED_PIN, led_state ? HIGH : LOW);
  }
  led_cycles++;
  if ((led_enabled && led_cycles > 120) ||
      (!led_enabled && led_cycles > 6 * 120)) {
    led_enabled = !led_enabled;
    digitalWrite(IR_LED_PIN, LOW);
    led_cycles = 0;
  }
}


void setup() {
  cli();

  TCCR1A = 0;
  TCCR1B = 1 << WGM12 | 1 << CS10;
  OCR1A = 211;
  TCNT1 = 0;
  TIMSK1 = 1 << OCIE1A;

  sei();

  for (int i = 0; i < NUM_GATES; i++) {
    pinMode(i + 4, INPUT);
  }

  pinMode(IR_LED_PIN, OUTPUT);
  pinMode(INDICATOR_PIN, OUTPUT);
}


long lastMillis[NUM_GATES];
bool states[NUM_GATES];

void loop() {
  for (int i = 0; i < NUM_GATES; i++) {
    int pin = i + 4;
    if (digitalRead(pin) == LOW) {
      if (states[i]) {
        Serial.print(i + 1);
        Serial.print(" ");
        Serial.println((millis() - lastMillis[i]) + 150);
      }
      lastMillis[i] = millis();
    }

    if (millis() > lastMillis[i] + 150) {
      states[i] = true;
    } else {
      states[i] = false;
    }
  }

  bool hasRat = false;
  for (int i = 0; i < NUM_GATES; i++) {
    if (states[i]) {
      hasRat = true;
      break;
    }
  }
  digitalWrite(INDICATOR_PIN, hasRat ? HIGH : LOW);
}
