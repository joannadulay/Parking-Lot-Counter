#include <Servo.h>

#define trigPin A4
#define echoPin A5
#define SERVO_PIN 0
Servo gateServo;

#define IR_SENSOR_PIN 1

int stepperPins[] = {A0, A1, A2, A3};

int segmentPins[] = {7, 3, 4, 5, 6, 2, 8, 9};
int digitPins[] = {10, 11, 12, 13};

int carCount = 0;
const int maxCars = 10;
int availableSlots = maxCars;
bool isMaxDisplayed = false;
const int stepsPer90Degrees = 300;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  gateServo.attach(SERVO_PIN);
  gateServo.write(0);
  pinMode(IR_SENSOR_PIN, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(stepperPins[i], OUTPUT);
  }
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
  displayInitialValue(); 
}

void loop() {
  if (digitalRead(IR_SENSOR_PIN) == HIGH && carCount < maxCars) {
    carCount++;
    availableSlots--;
    gateServo.write(45);

    unsigned long startTime = millis();
    while (millis() - startTime < 1000) {
      updateDisplay();
      delay(10);
    }

    while (digitalRead(IR_SENSOR_PIN) == HIGH) {
      delay(100);
    }

    gateServo.write(0);
  }

  if (carCount >= maxCars && !isMaxDisplayed) {
    displayNumber(1, 0, 0, 0);
    delay(1000);
    isMaxDisplayed = true;
  }

  if (carCount >= maxCars && checkUltrasonic()) {
    isMaxDisplayed = false;
    continuousExitDisplay();
    while (checkUltrasonic()) {
      delay(100);
    }
  }

  if (carCount < maxCars || !checkUltrasonic()) {
    updateDisplay();
  }

  delay(10);
}

bool checkUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration / 2) / 29.1;

  return (distance < 10 && distance > 0);
}

void rotateStepper90Degrees() {
  int stepSequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
  };

  for (int step = 0; step < stepsPer90Degrees; step++) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 4; j++) {
        digitalWrite(stepperPins[j], stepSequence[i][j]);
      }
      delay(1);
    }
  }
}

void displayInitialValue() {
  displayNumber(0, 0, 1, 0);
}

void updateDisplay() {
  int d1 = availableSlots % 10;
  int d2 = availableSlots / 10;
  int d3 = carCount % 10;
  int d4 = carCount / 10;

  displayNumber(d1, d2, d3, d4);
}

void continuousExitDisplay() {
  for (int i = 9; i > 0; i--) {
    int d1 = i;
    int d2 = 10 - i;
    int d3;
    int d4;

    rotateStepper90Degrees();
    displayNumber(d1, d3, d2, d4);

    delay(1000);

    while (checkUltrasonic()) {
      updateDisplay();
    }

    if (!checkUltrasonic()) {
      break;
    }
  }
}

void displayNumber(int d1, int d2, int d3, int d4) {
  displayDigit(0, d1);
  displayDigit(1, d2);
  displayDigit(2, d3);
  displayDigit(3, d4);
}

void displayDigit(int digit, int number) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }

  digitalWrite(digitPins[digit], LOW);

  int segments[] = { 
    0b00111111,
    0b00000110,
    0b01111010,
    0b01101110,
    0b01000111,
    0b01101101,
    0b01111101,
    0b00100111,
    0b01111111,
    0b01101111
  };
  
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentPins[i], bitRead(segments[number], i));
  }

  delay(5);

  digitalWrite(digitPins[digit], HIGH);
}
