// ========================================================================
//  🔸 M U L T I S T E P P E R  -  N O N   B L O C K I N G   A N G L E 🔸
// ========================================================================
//  Archivo    : MultiStepper_Non-Blocking-Angle.ino
//  Autor      : Klaus Michalsky
//  Fecha      : Feb-2026
//
//  DESCRIPCION
//  -----------------------------------------------------------------------
//  - Cada motor se mueve al ángulo deseado de forma sincronizada.
//  - Variante con multi.run(); dentro de while(...)
//  - Pasos por vuelta definidos como (200 * microstepping).
//  - Hace un bucle llamando repetidamente a multi.run() hasta llegar.
//
//  Ventajas:
//  - Puedes agregar dentro del bucle acciones adicionales
//    como leer sensores, actualizar OLED, enviar datos por UART.
//  - Compatible con movimientos continuos o complejos, sin depender de bloqueos grandes.
//  - Permite controlar aceleración y velocidad más precisas, porque cada paso se calcula
//    y ejecuta en tiempo real.
//  Desventajas:
//  - Requiere un bucle extra (while) y un pequeño delay(1)
//    para que los motores puedan procesar los pasos.
//  - Un poco más de código que la versión simple con runSpeedToPosition().
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
long pasos = (long)((angulo / angulo_por_paso) * microstepping + 0.5); // redondeo seguro

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

    // Ejecutar movimiento bloqueante hasta llegar a la posición
    while (motor1.distanceToGo() != 0 || motor2.distanceToGo() != 0)
    {
        multi.run();
        delay(1); // pequeño retardo para procesar pasos
    }
}

void mover_izquierda()
{
    long posiciones[2];
    posiciones[0] = motor1.currentPosition() - pasos;
    posiciones[1] = motor2.currentPosition() - pasos;

    multi.moveTo(posiciones);

    while (motor1.distanceToGo() != 0 || motor2.distanceToGo() != 0)
    {
        multi.run();
        delay(1);
    }
}
