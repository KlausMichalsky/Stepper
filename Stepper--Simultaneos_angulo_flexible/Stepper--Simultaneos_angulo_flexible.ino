/************************************************************************************************************
 🔹 POSICIONAMIENTO DEL ANGULO DE DOS MOTORES PASO A PASO CON MultiStepper 🔹
Este programa controla dos motores paso a paso usando drivers tipo STEP/DIR,
la librería AccelStepper y el objeto MultiStepper para lograr movimientos
sincronizados. Cada motor se mueve al ángulo deseado de forma sincronizada.
2️⃣ Variante con multi.run(); dentro de while(...)
Hace un bucle llamando repetidamente a multi.run() hasta llegar.
Ventajas:
Más flexible, puedes agregar dentro del bucle acciones adicionales, como leer sensores, actualizar OLED, enviar datos por UART, etc.
Compatible con movimientos continuos o complejos, sin depender de bloqueos grandes.
Permite controlar aceleración y velocidad más precisas, porque cada paso se calcula y ejecuta en tiempo real.
Desventajas:
Requiere un bucle extra (while) y un pequeño delay(1) para que los motores puedan procesar los pasos.
Un poco más de código que la versión simple con runSpeedToPosition().

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
long pasos = (long)((angulo / angulo_por_paso) * microstepping + 0.5); // redondeo seguro

// Objetos de motores
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);
MultiStepper multi;

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

    // Ejecutar movimiento bloqueante hasta llegar a la posición
    while (motor1.distanceToGo() != 0 || motor2.distanceToGo() != 0)
    {
        multi.run();
        delay(1); // pequeño retardo para procesar pasos
    }
}

// Movimiento a la izquierda (una vuelta completa)
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