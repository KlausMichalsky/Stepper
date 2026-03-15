// ========================================================================
//         🔸 A C C E L S T E P P E R  -  S Y N C H R O N I Z E D 🔸
// ========================================================================
//  Archivo    : STEPPER--AccelStepper_two_motors_synchronized.ino
//  Autor      : Klaus Michalsky
//  Fecha      : Mar-2026
//
//  DESCRIPCION
//  -----------------------------------------------------------------------
//  Test para mover dos motores paso a paso de forma sincronizada usando
//  AccelStepper. Los motores pueden recorrer distancias diferentes pero
//  llegan al destino aproximadamente al mismo tiempo.
//
//  El código:
//  - Convierte ángulos a pasos considerando microstepping y reducción.
//  - Calcula la distancia en pasos de cada motor.
//  - Escala velocidad y aceleración proporcionalmente.
//
//  Esto permite movimientos suaves con aceleración manteniendo la
//  sincronización.
//
//  HARDWARE
//  -----------------------------------------------------------------------
//  MCU     : RP2040-Zero
//  Motor   : 2x NEMA17
//  Driver  : 2x TMC2209
//
//  ESTADO
//  -----------------------------------------------------------------------
//  ✅ Funcional
// ========================================================================

#include <Arduino.h>
#include <AccelStepper.h>

// ----------------------
// Pines Motor 1
// ----------------------
#define MOTOR1_ENABLE 6
#define MOTOR1_DIR 7
#define MOTOR1_STEP 8

// ----------------------
// Pines Motor 2
// ----------------------
#define MOTOR2_ENABLE 12
#define MOTOR2_DIR 13
#define MOTOR2_STEP 14

// ----------------------
// Configuración mecánica
// ----------------------
#define MICROSTEPPING 16
#define STEPS_PER_REV 200

#define REDUCTION_M1 9
#define REDUCTION_M2 6

#define ACCELERATION 1000.0
#define BASE_SPEED 1500.0

// ----------------------

AccelStepper motor1(AccelStepper::DRIVER, MOTOR1_STEP, MOTOR1_DIR);
AccelStepper motor2(AccelStepper::DRIVER, MOTOR2_STEP, MOTOR2_DIR);

// ----------------------

long angleToStepsMotor1(float angle)
{
    float stepsPerRev = MICROSTEPPING * STEPS_PER_REV * REDUCTION_M1;
    return (angle / 360.0) * stepsPerRev;
}

long angleToStepsMotor2(float angle)
{
    float stepsPerRev = MICROSTEPPING * STEPS_PER_REV * REDUCTION_M2;
    return (angle / 360.0) * stepsPerRev;
}

// ----------------------

void setup()
{
    Serial.begin(115200);

    pinMode(MOTOR1_ENABLE, OUTPUT);
    pinMode(MOTOR2_ENABLE, OUTPUT);

    digitalWrite(MOTOR1_ENABLE, LOW);
    digitalWrite(MOTOR2_ENABLE, LOW);

    delay(2000);

    Serial.println("TEST MOVIMIENTO SINCRONIZADO");

    // Angulos de prueba
    float angleMotor1 = 90.0;
    float angleMotor2 = 45.0;

    long target1 = angleToStepsMotor1(angleMotor1);
    long target2 = angleToStepsMotor2(angleMotor2);

    long delta1 = abs(target1 - motor1.currentPosition());
    long delta2 = abs(target2 - motor2.currentPosition());

    long maxDelta = max(delta1, delta2);

    float ratio1 = (float)delta1 / maxDelta;
    float ratio2 = (float)delta2 / maxDelta;

    motor1.setMaxSpeed(BASE_SPEED * ratio1);
    motor2.setMaxSpeed(BASE_SPEED * ratio2);

    motor1.setAcceleration(ACCELERATION * ratio1);
    motor2.setAcceleration(ACCELERATION * ratio2);

    motor1.moveTo(target1);
    motor2.moveTo(target2);

    Serial.println("Motores moviendose...");
}

// ----------------------

void loop()
{
    motor1.run();
    motor2.run();

    if (!motor1.isRunning() && !motor2.isRunning())
    {
        Serial.println("Movimiento terminado");
        while (1)
            ;
    }
}