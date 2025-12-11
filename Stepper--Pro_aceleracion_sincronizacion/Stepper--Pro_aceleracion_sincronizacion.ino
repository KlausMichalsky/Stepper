#include <AccelStepper.h>

// ------------------- PINES -------------------
#define M1_STEP 4
#define M1_DIR 5
#define M1_EN 2

#define M2_STEP 14
#define M2_DIR 13
#define M2_EN 12

// ---------------- CONFIGURACIÓN ----------------
const int MICROSTEPPING = 8;
const float GRADOS_PASO = 1.8;

const long PASOS_VUELTA = (360.0 / GRADOS_PASO) * MICROSTEPPING;

// Reducciones:
const int REDUCCION_M1 = 9; // motor 1 (9:1)
const int REDUCCION_M2 = 6; // motor 2 (6:1)

const long PASOS_360_M1 = PASOS_VUELTA * REDUCCION_M1; // 14400
const long PASOS_360_M2 = PASOS_VUELTA * REDUCCION_M2; // 9600

// ---------------- MOTORES ----------------------
AccelStepper motor1(AccelStepper::DRIVER, M1_STEP, M1_DIR);
AccelStepper motor2(AccelStepper::DRIVER, M2_STEP, M2_DIR);

void setup()
{
    // ENABLE
    pinMode(M1_EN, OUTPUT);
    pinMode(M2_EN, OUTPUT);

    digitalWrite(M1_EN, LOW); // activar
    digitalWrite(M2_EN, LOW);

    // Configuración motores
    motor1.setMaxSpeed(1500);
    motor1.setAcceleration(800);

    motor2.setMaxSpeed(1500);
    motor2.setAcceleration(800);
}

void loop()
{
    mover_360();
    delay(1000);

    mover_360_back();
    delay(1000);
}

// ------------------------------------------------
//   Movimiento sincronizado con aceleración real
// ------------------------------------------------
void mover_360()
{
    long pos1 = motor1.currentPosition() + PASOS_360_M1;
    long pos2 = motor2.currentPosition() + PASOS_360_M2;

    motor1.moveTo(pos1);
    motor2.moveTo(pos2);

    sincronizar();
}

void mover_360_back()
{
    long pos1 = motor1.currentPosition() - PASOS_360_M1;
    long pos2 = motor2.currentPosition() - PASOS_360_M2;

    motor1.moveTo(pos1);
    motor2.moveTo(pos2);

    sincronizar();
}

// ------------------------------------------------
//     SINCRONIZACIÓN REAL SIN MULTISTEPPER
// ------------------------------------------------
void sincronizar()
{
    while (true)
    {

        long d1 = abs(motor1.distanceToGo());
        long d2 = abs(motor2.distanceToGo());

        // Escalar velocidad según el progreso
        float ratio = (float)d1 / (float)d2;

        if (ratio < 1.0)
            motor2.setSpeed(motor1.speed() * (1.0 / ratio));
        else
            motor1.setSpeed(motor2.speed() * ratio);

        motor1.run();
        motor2.run();

        if (d1 == 0 && d2 == 0)
            break;
    }
}
