#include <Arduino.h>
#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Simultaneos_angulo_flexible\\Stepper--Simultaneos_angulo_flexible.ino"
/************************************************************************************************************
 🔹 POSICIONAMIENTO DEL ANGULO DE DOS MOTORES PASO A PASO CON MultiStepper (VERSIÓN PREMIUM) 🔹
Movimiento continuo derecha-izquierda con MultiStepper sin bloquear el loop,
permitiendo agregar otras tareas mientras los motores giran.

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

// Variables de control de dirección
bool moverDerecha = true;
long posiciones[2];

#line 36 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Simultaneos_angulo_flexible\\Stepper--Simultaneos_angulo_flexible.ino"
void setup();
#line 65 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Simultaneos_angulo_flexible\\Stepper--Simultaneos_angulo_flexible.ino"
void loop();
#line 95 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Simultaneos_angulo_flexible\\Stepper--Simultaneos_angulo_flexible.ino"
bool motoresEnMovimiento();
#line 36 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Simultaneos_angulo_flexible\\Stepper--Simultaneos_angulo_flexible.ino"
void setup()
{
    // Desactivar motores
    pinMode(ENABLE1, OUTPUT);
    pinMode(ENABLE2, OUTPUT);
    digitalWrite(ENABLE1, HIGH);
    digitalWrite(ENABLE2, HIGH);

    // Configurar motores
    motor1.setMaxSpeed(2000);     // pasos/seg
    motor1.setAcceleration(2000); // pasos/seg^2
    motor2.setMaxSpeed(2000);
    motor2.setAcceleration(2000);

    // Añadir al MultiStepper
    multi.addStepper(motor1);
    multi.addStepper(motor2);

    // Activar motores
    delay(500);
    digitalWrite(ENABLE1, LOW);
    digitalWrite(ENABLE2, LOW);

    // Inicializar primera posición
    posiciones[0] = motor1.currentPosition() + pasos;
    posiciones[1] = motor2.currentPosition() + pasos;
    multi.moveTo(posiciones);
}

void loop()
{
    // Ejecutar movimiento paso a paso no bloqueante
    if (!motoresEnMovimiento())
    {
        // Cuando llegan a la posición, cambiar dirección
        moverDerecha = !moverDerecha;

        if (moverDerecha)
        {
            posiciones[0] = motor1.currentPosition() + pasos;
            posiciones[1] = motor2.currentPosition() + pasos;
        }
        else
        {
            posiciones[0] = motor1.currentPosition() - pasos;
            posiciones[1] = motor2.currentPosition() - pasos;
        }

        multi.moveTo(posiciones);
    }

    // Ejecutar los motores sin bloquear
    multi.run();

    // Aquí puedes agregar otras tareas mientras los motores se mueven
    // Por ejemplo: leer sensores, actualizar pantalla, enviar datos, etc.
}

// Función para comprobar si los motores aún se están moviendo
bool motoresEnMovimiento()
{
    return (motor1.distanceToGo() != 0 || motor2.distanceToGo() != 0);
}

