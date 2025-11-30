/************************************************************************************************************
 🔹 POSISIONAMIENTO DEL ANGULO DE DOS MOTORES PASO A PASO CON MultiStepper 🔹
Este programa controla dos motores paso a paso usando drivers tipo STEP/DIR,
la librería AccelStepper y el objeto MultiStepper para lograr movimientos
sincronizados. Cada motor se mueve al angulo deseado de forma sincronizada.

Funciones principales:
- Configura los pines STEP, DIR y ENABLE de cada motor.
- Aplica aceleración y velocidad máxima mediante AccelStepper.
- Usa MultiStepper para mover ambos motores al mismo tiempo y asegurar que
  lleguen coordinados a sus posiciones objetivo.
- Calcula la cantidad de pasos por vuelta usando 200 pasos del motor

  K. Michalsky – 11.2025
*************************************************************************************************************/

#include <Bounce2.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

// DEFINICION DE PINS
// Motor1
#define STEP1 5
#define DIR1 4
#define ENABLE1 6

// Motor2
#define STEP2 8
#define DIR2 7
#define ENABLE2 9

// CREAR OBJETOS PARA CADA MOTOR
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);

// DECLARAR OBJETO MULTISTEPPER PARA MOVIMIENTO SINCRONIZADO
MultiStepper multi;

// CONVERSION DE PASOS Y ANGULO
int angulo = 360;                             // ángulo deseado en grados
float angulo_por_paso = 1.8;                  // grados que mueve el motor por paso
long pasos = round(angulo / angulo_por_paso); // redondea al entero más cercano

float velocidad = pasos / 10.0; // 1 vuelta en 10 segundos → 2800 pasos/seg

void setup()
{
    // DESACTIVAR MOTORES
    // Antes de la configuracion deshabilitar motores para evitar pasos indeseados mientras se ejecuta el setup()
    pinMode(ENABLE1, OUTPUT);
    pinMode(ENABLE2, OUTPUT);
    digitalWrite(ENABLE1, HIGH);
    digitalWrite(ENABLE2, HIGH);

    // CONFIGURAR MOTORES
    motor1.setMaxSpeed(2000);     // pasos/seg
    motor1.setAcceleration(1000); // pasos/seg^2
    motor2.setMaxSpeed(2000);
    motor2.setAcceleration(1000);

    // AGREGAR MOTORES AL OBJETO MULTISTEPPER
    multi.addStepper(motor1);
    multi.addStepper(motor2);

    // ACTIVAR MOTOTRES
    // Despues del setup esperar 1seg y activar motores
    delay(1000);
    digitalWrite(ENABLE1, LOW);
    digitalWrite(ENABLE2, LOW);
}

void loop()
{
    mover_derecha();
    mover_izquierda();
}

// MOVER AMBOS MOTORES 1 VUELTA COMPLETA HACIA LA DERECHA
void mover_derecha()
{
    // Crea un array de tipo long con dos elementos.
    // Cada elemento representa la posición objetivo de un motor dentro del objeto MultiStepper
    long posiciones[2];

    posiciones[0] = motor1.currentPosition() + pasos; // motor1
    posiciones[1] = motor2.currentPosition() + pasos; // motor2

    // MOVER AMBOS MOTORES SINCRONIZADOS
    multi.moveTo(posiciones);
    multi.runSpeedToPosition();
}

// MOVER AMBOS MOTORES 1 VUELTA COMPLETA HACIA LA IZQUIERDA (ver funcion mover_derecha())
void mover_izquierda()
{
    long posiciones[2];
    posiciones[0] = motor1.currentPosition() - pasos;
    posiciones[1] = motor2.currentPosition() - pasos;
    multi.moveTo(posiciones);
    multi.runSpeedToPosition();
}
