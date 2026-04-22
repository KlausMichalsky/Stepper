// ========================================================================
//      🔸 M U L T I S T E P P E R  -  B L O C K I N G   A N G L E 🔸
// ========================================================================
//  Archivo    : MultiStepper_Blocking-Angle.ino
//  Autor      : Klaus Michalsky
//  Fecha      : Feb-2026
//
//  DESCRIPCION
//  -----------------------------------------------------------------------
//  - Cada motor se mueve al ángulo deseado de forma sincronizada.
//  - Variante con multi.runSpeedToPosition()
//  - Pasos por vuelta definidos como (200 * microstepping).
//
//  Ventajas:
//  - Muy simple llamas una vez y espera a que ambos motores lleguen.
//  - No hace falta un bucle extra para controlar el movimiento.
//  - Ideal para movimientos discretos, como dar una vuelta completa a la derecha o izquierda, y luego hacer otra acción.
//  Desventajas:
//  - Es bloqueante, por lo que mientras los motores se mueven, no puede ejecutarse nada más
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
#include <MultiStepper.h>

//  DEFINICION DE PINES
//  -----------------------------------------------------------------------
#define STEP1 4
#define DIR1 5
#define ENABLE1 2
#define STEP2 8
#define DIR2 7
#define ENABLE2 9

// OBJETOS
//  -----------------------------------------------------------------------
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);
MultiStepper multi;

// PARAMETROS DE CONFIGURACION
//  -----------------------------------------------------------------------
const int microstepping = 8;       // microstepping configurado en el driver
const int angulo = 360;            // ángulo deseado
const float angulo_por_paso = 1.8; // grados por paso del motor

// Cálculo de pasos totales por vuelta considerando microstepping
long pasos = (long)((angulo / angulo_por_paso) * microstepping + 0.5);

// =======================================================================
// SETUP
// =======================================================================
void setup()
{
  pinMode(ENABLE1, OUTPUT);
  pinMode(ENABLE2, OUTPUT);
  digitalWrite(ENABLE1, HIGH);
  digitalWrite(ENABLE2, HIGH);

  motor1.setMaxSpeed(800);
  motor1.setAcceleration(400);
  motor2.setMaxSpeed(800);
  motor2.setAcceleration(400);

  multi.addStepper(motor1);
  multi.addStepper(motor2);

  delay(1000);
  digitalWrite(ENABLE1, LOW);
  digitalWrite(ENABLE2, LOW);
}

// =======================================================================
// LOOP
// =======================================================================
void loop()
{
  mover_derecha();
  delay(500);
  mover_izquierda();
  delay(500);
}

void mover_derecha()
{
  long posiciones[2];
  posiciones[0] = motor1.currentPosition() + pasos;
  posiciones[1] = motor2.currentPosition() + pasos;

  multi.moveTo(posiciones);
  multi.runSpeedToPosition(); // bloquea hasta llegar
}

void mover_izquierda()
{
  long posiciones[2];
  posiciones[0] = motor1.currentPosition() - pasos;
  posiciones[1] = motor2.currentPosition() - pasos;

  multi.moveTo(posiciones);
  multi.runSpeedToPosition(); // bloquea hasta llegar
}
