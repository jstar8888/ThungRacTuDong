#include <Servo.h>

Servo myservo;

const int servoPin = 6;
const int trigPin = 7;
const int echoPin = 8;
const int trigTrashPin = 9;
const int echoTrashPin = 10;

const int openAngle = 90;
const int closeAngle = 10; // tránh 0 độ để giảm căng

enum LidState { CLOSE, OPEN, HOLD };
LidState lidState = CLOSE;

unsigned long holdStartTime = 0;
const unsigned long holdTimeout = 5000; // 5 giây
const int dayThreshold = 8;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigTrashPin, OUTPUT);
  pinMode(echoTrashPin, INPUT);

  myservo.attach(servoPin);
  myservo.write(closeAngle);
  delay(500);
  myservo.detach();
  lidState = CLOSE;
}

long measureDistanceCM(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  if (distance == 0) return 200;
  return distance;
}

String checkTrashLevel(int trigPin, int echoPin, int threshold) {
  long distance = measureDistanceCM(trigPin, echoPin);
  Serial.println(distance);
  if (distance <= threshold) {
    return "FULL";
  } else {
    return "AVAILABLE";
  }
}

void moveServo(int angle) {
  myservo.attach(servoPin);
  myservo.write(angle);
  delay(600); // thời gian để servo quay xong
  myservo.detach();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.println(command);

    if (command == "open") {
      moveServo(openAngle);
      lidState = OPEN;
      holdStartTime = millis();
    } else if (command == "close") {
      moveServo(closeAngle);
      lidState = CLOSE;
      delay(3000);
      String status = checkTrashLevel(trigTrashPin, echoTrashPin, dayThreshold);
      Serial.println(status);
    }
  }

  long dist = measureDistanceCM(trigPin, echoPin);

  if (lidState == OPEN) {
    if (dist < 10) {
      lidState = HOLD;
      holdStartTime = millis();
    } else if (millis() - holdStartTime > holdTimeout) {
      moveServo(closeAngle);
      lidState = CLOSE;
      delay(3000);
      Serial.println(checkTrashLevel(trigTrashPin, echoTrashPin, dayThreshold));
    }
  }
  else if (lidState == HOLD) {
    if (dist < 30) {
      holdStartTime = millis();
    } else if (millis() - holdStartTime > holdTimeout) {
      moveServo(closeAngle);
      lidState = CLOSE;
      delay(3000);
      Serial.println(checkTrashLevel(trigTrashPin, echoTrashPin, dayThreshold));
    }
  }
  else if (lidState == CLOSE && dist < 15) {
    moveServo(openAngle);
    lidState = OPEN;
    holdStartTime = millis();
  }
}
