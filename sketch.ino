// Rasha Putra Permata
// 26/582550/TK/66380

// Import library
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

// Setup pin
const int pirPin = 14;
const int servoPins[] = {13, 25, 26, 27, 4};

Adafruit_MPU6050 mpu;
Servo myServos[5];

// Setup timer and state variables
unsigned long rotationEndTime = 0;
bool isRotating = false;
bool isResetting = false;
const int initialYaw = 90;
int targetYaw = 90;

const float gyroThreshold = 0.3;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // If sensor is not connected properly, stop the program
  if (!mpu.begin()) {
    Serial.println("MPU6050 error! :[");
    while (1) {
      delay(10);
    }
  }

  for (int i = 0; i < 5; i++) {
    myServos[i].attach(servoPins[i]);
  }

  Serial.println("MPU6050 is good to go! :]");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Control tilt X for servo 1 to 4
  float tiltX = a.acceleration.x;
  int servoAngle = map((int)tiltX, -9, 9, 0, 180);
  servoAngle = constrain(servoAngle, 0, 180);
  int inversed_servoAngle = 180 - servoAngle;

  for (int i = 0; i < 2; i++) {
    myServos[i].write(inversed_servoAngle);
    myServos[i + 2].write(servoAngle);
  }

  // Control rotation yaw Z for servo 5
  float gyroZ = g.gyro.z;
  unsigned long currentTime = millis();

  // If the servo is not currently resetting back to the middle position
  if (!isResetting) {
    // If the sensor spins to the right
    if (gyroZ > gyroThreshold) {
      targetYaw = 180;
      isRotating = true;
    }
    // If the sensor spins to the left
    else if (gyroZ < -gyroThreshold) {
      targetYaw = 0;
      isRotating = true;
    }

    // If the sensor was spinning but now it stopped moving
    else if (isRotating && abs(gyroZ) <= gyroThreshold) {
      isRotating = false;
      isResetting = true;
      rotationEndTime = currentTime;
    }
  }

  // If the servo is currently in the process of resetting back to the middle
  if (isResetting) {
    // If the servo has stayed still for 1 second
    if (currentTime - rotationEndTime >= 1000) {
      targetYaw = initialYaw;

      // If 1.5 seconds have passed in total, let the sensor read movements again
      if (currentTime - rotationEndTime >= 1500) {
        isResetting = false;
      }
    }
  }

  myServos[4].write(targetYaw);

  delay(10);
}
