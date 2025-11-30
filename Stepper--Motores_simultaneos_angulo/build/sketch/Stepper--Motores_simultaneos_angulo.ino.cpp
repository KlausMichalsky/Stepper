#include <Arduino.h>
#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
/************************************************************************************************************
 🔹 POSICIONAMIENTO DEL ANGULO DE DOS MOTORES PASO A PASO CON MultiStepper 🔹
Este programa controla dos motores paso a paso usando drivers tipo STEP/DIR,
la librería AccelStepper y el objeto MultiStepper para lograr movimientos
sincronizados. Cada motor se mueve al ángulo deseado de forma sincronizada.

  K. Michalsky – 11.2025
*************************************************************************************************************/

#include <AccelStepper.h>
#include <MultiStepper.h>

// DEFINICION DE PINS
#define STEP1 4
#define DIR1 5
#define ENABLE1 2
#define STEP2 8
#define DIR2 7
#define ENABLE2 9

const int microstepping = 8;       // microstepping configurado en el driver
const int angulo = 360;            // ángulo deseado
const float angulo_por_paso = 1.8; // grados por paso del motor

// Cálculo de pasos totales por vuelta considerando microstepping
long pasos = (long)((angulo / angulo_por_paso) * microstepping + 0.5);

// Objetos de motores
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);
MultiStepper multi;

#line 33 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
void setup();
#line 57 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
void loop();
#line 66 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
void mover_derecha();
#line 77 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
void mover_izquierda();
#line 33 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Motores_simultaneos_angulo\\Stepper--Motores_simultaneos_angulo.ino"
void setup()
{
  // Desactivar motores
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  digitalWrite(ENABLE1, HIGH);
  digitalWrite(ENABLE2, HIGH);

  // Configurar motores
  motor1.setMaxSpeed(800);     // pasos/seguro
  motor1.setAcceleration(400); // pasos/seg^2
  motor2.setMaxSpeed(800);
  motor2.setAcceleration(400);

  // Añadir al MultiStepper
  multi.addStepper(motor1);
  multi.addStepper(motor2);

  // Activar motores
  delay(1000);
  digitalWrite(ENABLE1, LOW);
  digitalWrite(ENABLE2, LOW);
}

void loop()
{
  mover_derecha();
  delay(500);
  mover_izquierda();
  delay(500);
}

// Movimiento a la derecha (una vuelta completa)
void mover_derecha()
{
  long posiciones[2];
  posiciones[0] = motor1.currentPosition() + pasos;
  posiciones[1] = motor2.currentPosition() + pasos;

  multi.moveTo(posiciones);
  multi.runSpeedToPosition(); // bloquea hasta llegar
}

// Movimiento a la izquierda (una vuelta completa)
void mover_izquierda()
{
  long posiciones[2];
  posiciones[0] = motor1.currentPosition() - pasos;
  posiciones[1] = motor2.currentPosition() - pasos;

  multi.moveTo(posiciones);
  multi.runSpeedToPosition(); // bloquea hasta llegar
}

