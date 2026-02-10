// ========================================================================
//            🔸 A C C E L S T E P P E R  -  O N E   T U R N 🔸
// ========================================================================
//  Archivo    : STEPPER.ino
//  Autor      : Klaus Michalsky
//  Fecha      : Feb-2026
//
//  DESCRIPCION
//  -----------------------------------------------------------------------
//  - Ejecuta una “vuelta” (definida como motorX_vueltas)
//    hacia un lado y regresa a la posición 0.
//  - Pasos por vuelta definidos como (200 * microstepping).
//  - Ciclo continuo con pausa de 1 segundo entre movimientos.
//
//  HARDWARE
//  -----------------------------------------------------------------------
//  MCU     : RP2040-Zero
//  Motor   : NEMA17
//  Driver  : TMC2209
//
//  ESTADO
//  -----------------------------------------------------------------------
//  ✅ Funcional
// ========================================================================

#include <AccelStepper.h>

//  DEFINICION DE PINES
//  -----------------------------------------------------------------------
#define DIR1 7
#define STEP1 8
#define ENABLE1 6

#define DIR2 13
#define STEP2 14
#define ENABLE2 12

// OBJETOS
//  -----------------------------------------------------------------------
// Crea una instancia: selecciona la versión que quieras usar y descomentándola. Ese es el único cambio necesario.
// AccelStepper myStepper(AccelStepper::FULL4WIRE, AIn1, AIn2, BIn1, BIn2); // funciona para TB6612 (controlador de motor bipolar, de voltaje constante y puente H)
// AccelStepper myStepper(AccelStepper::FULL4WIRE, In1, In3, In2, In4);    // funciona para ULN2003 (controlador de motor unipolar)
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);

// CONFIGURACION DE MOTORES
//  -----------------------------------------------------------------------
// Motores de 200 pasos por vuelta con microstepping 1/16
// 200 * 16 = 3200 pasos por vuelta
// ENABLE LOW = driver activado
const int microstepping = 16;
const int pasosPorVuelta = 200 * microstepping;
const int motor1_vueltas = 9;
const int motor2_vueltas = 6;

// =======================================================================
// SETUP
// =======================================================================
void setup()
{
  // Antes de la configuracion deshabilitar motores para evitar pasos indeseados mientras se ejecuta el setup()
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  digitalWrite(ENABLE1, LOW);
  digitalWrite(ENABLE2, LOW);

  motor1.setMaxSpeed(2000);     // pasos/seg
  motor1.setAcceleration(2000); // pasos/seg^2
  motor2.setMaxSpeed(2000);
  motor2.setAcceleration(2000);

  motor1.setCurrentPosition(0);
  motor2.setCurrentPosition(0);
}

// =======================================================================
// LOOP
// =======================================================================
void loop()
{
  // Motor1: 1 vuelta adelante
  motor1.moveTo(motor1_vueltas * pasosPorVuelta);
  while (motor1.distanceToGo() != 0)
  {
    motor1.run();
  }
  delay(1000);

  // Motor1: volver a origen
  motor1.moveTo(0);
  while (motor1.distanceToGo() != 0)
  {
    motor1.run();
  }
  delay(1000);

  // Motor2: 1 vuelta adelante
  motor2.moveTo(motor2_vueltas * pasosPorVuelta);
  while (motor2.distanceToGo() != 0)
  {
    motor2.run();
  }
  delay(1000);

  // Motor2: volver a origen
  motor2.moveTo(0);
  while (motor2.distanceToGo() != 0)
  {
    motor2.run();
  }
  delay(1000);
}
