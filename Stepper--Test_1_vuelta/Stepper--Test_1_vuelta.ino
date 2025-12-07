/************************************************************************************************************
 🔹 CONTROL STEPPER CON ACCELSTEPPER: 1 VUELTA IZQ → 1 VUELTA DER 🔹
  - Motor paso a paso controlado con driver STEP/DIR.
  - Pines usados: DIR=5, STEP=4, ENABLE=2.
  - Usa AccelStepper con velocidad y aceleración configuradas.
  - Ejecuta una vuelta completa hacia un lado y regresa a la posición 0.
  - Pasos por vuelta definidos como (200 * microstepping).
  - Ciclo continuo con pausa de 1 segundo entre movimientos.
    K. Michalsky – 11.2025
************************************************************************************************************/


#include <AccelStepper.h>

// ==== PINES ====
#define DIR 5
#define STEP 4
#define ENABLE 2

// ==== OBJETO ====
AccelStepper motor(AccelStepper::DRIVER, STEP, DIR);

// ==== CONFIGURACIÓN DEL MOTOR ====
// Motor de 200 pasos por vuelta con microstepping 1/16
// 200 * 16 = 3200 pasos por vuelta
const int microstepping = 16;
const int pasosPorVuelta = 200*microstepping;

void setup() {
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW); // LOW = driver activado

  motor.setMaxSpeed(2000);
  motor.setAcceleration(1000);

  motor.setCurrentPosition(0);
}

void loop() {

  // --- 1 vuelta a la izquierda ---
  motor.moveTo(pasosPorVuelta);  // 3200 pasos
  while (motor.distanceToGo() != 0) {
    motor.run();
  }
  delay(1000);

  // --- 1 vuelta a la derecha ---
  motor.moveTo(0);  // volver a la posición inicial
  while (motor.distanceToGo() != 0) {
    motor.run();
  }
  delay(1000);

}
