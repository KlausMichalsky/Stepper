/************************************************************************************************************
 🔹 CONTROL STEPPER CON ACCELSTEPPER: 1 VUELTA IZQ → 1 VUELTA DER 🔹
  - Motor1 paso a paso controlado con driver STEP1/DIR1.
  - Pines usados: DIR1 = 7, STEP1 = 8, ENABLE1 = 6.
  - Motor2 paso a paso controlado con driver STEP2/DIR2.
  - Pines usados: DIR2 = 13, STEP2 = 14, ENABLE2 = 12.
  - Usa AccelStepper con velocidad y aceleración configuradas.
  - Ejecuta una “vuelta” (definida como motorX_vueltas) hacia un lado y regresa a la posición 0.
  - Pasos por vuelta definidos como (200 * microstepping).
  - Ciclo continuo con pausa de 1 segundo entre movimientos.
    K. Michalsky – 11.2025
************************************************************************************************************/

#include <AccelStepper.h>

// ==== PINES ====
#define DIR1    7
#define STEP1   8
#define ENABLE1 6

#define DIR2    13
#define STEP2   14
#define ENABLE2 12

// ==== OBJETOS ====
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);

// ==== CONFIGURACIÓN DE MOTORES ====
// Motores de 200 pasos por vuelta con microstepping 1/16
// 200 * 16 = 3200 pasos por vuelta
const int microstepping   = 16;
const int pasosPorVuelta  = 200 * microstepping;
const int motor1_vueltas  = 9;
const int motor2_vueltas  = 6;

void setup() {
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  digitalWrite(ENABLE1, LOW);  // LOW = driver activado (según tu versión)
  digitalWrite(ENABLE2, LOW);

  motor1.setMaxSpeed(5000);     // ajustar velocidad máxima
  motor1.setAcceleration(5000);
  motor2.setMaxSpeed(5000);
  motor2.setAcceleration(5000);

  motor1.setCurrentPosition(0);
  motor2.setCurrentPosition(0);
}

void loop() {
  // --- Motor1: 1 vuelta adelante ---
  motor1.moveTo(motor1_vueltas * pasosPorVuelta);
  while (motor1.distanceToGo() != 0) {
    motor1.run();
  }
  delay(1000);

  // --- Motor1: volver a 0 ---
  motor1.moveTo(0);
  while (motor1.distanceToGo() != 0) {
    motor1.run();
  }
  delay(1000);

  // --- Motor2: 1 vuelta adelante ---
  motor2.moveTo(motor2_vueltas * pasosPorVuelta);
  while (motor2.distanceToGo() != 0) {
    motor2.run();
  }
  delay(1000);

  // --- Motor2: volver a 0 ---
  motor2.moveTo(0);
  while (motor2.distanceToGo() != 0) {
    motor2.run();
  }
  delay(1000);
}
