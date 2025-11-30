/************************************************************************************************************
 🔹 MOVIMIENTO DE DOS MOTORES PASO A PASO CON MultiStepper 🔹
Este programa controla dos motores paso a paso usando drivers tipo STEP/DIR,
la librería AccelStepper y el objeto MultiStepper para lograr movimientos
sincronizados. Cada motor realiza una vuelta completa hacia la derecha y una
vuelta hacia la izquierda de forma continua.

Funciones principales:
- Configura los pines STEP, DIR y ENABLE de cada motor.
- Aplica aceleración y velocidad máxima mediante AccelStepper.
- Usa MultiStepper para mover ambos motores al mismo tiempo y asegurar que
  lleguen coordinados a sus posiciones objetivo.
- Calcula la cantidad de pasos por vuelta usando 200 pasos del motor, 1/16 de
  microstepping y una reducción 9:1.
- En el loop se ejecutan dos funciones:
      mover_derecha(): avanza ambos motores una vuelta completa.
      mover_izquierda(): retrocede ambos motores una vuelta completa.
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
// Crea una instancia: selecciona la versión que quieras usar y descomentándola. Ese es el único cambio necesario.
// AccelStepper myStepper(AccelStepper::FULL4WIRE, AIn1, AIn2, BIn1, BIn2); // funciona para TB6612 (controlador de motor bipolar, de voltaje constante y puente H)
// AccelStepper myStepper(AccelStepper::FULL4WIRE, In1, In3, In2, In4);    // funciona para ULN2003 (controlador de motor unipolar)
AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor2(AccelStepper::DRIVER, STEP2, DIR2);

// DECLARAR OBJETO MULTISTEPPER PARA MOVIMIENTO SINCRONIZADO
// Sirve para controlar varios motores paso a paso al mismo tiempo, de forma que todos lleguen a su destino coordinadamente, aunque cada uno tenga distinta distancia o velocidad
MultiStepper multi;

// CONVERSION DE PASOS
const int pasos_por_vuelta = 200 * 16 * 9; // 200 pasos * 1/16 microstepping * reduccion 9:1
float velocidad = pasos_por_vuelta / 10.0; // 1 vuelta en 10 segundos → 2800 pasos/seg

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
    // Crea un array de tipo long con dos elementos. Cada elemento representa la posición objetivo de un motor dentro del objeto MultiStepper
    long posiciones[2];

    // CALCULAR LA NUEVA POSICION DESTINO PARA CADA MOTOR.
    // currentPosition() devuelve en qué paso está actualmente cada motor.
    // Al sumarle pasos_por_vuelta, haces que ambos motores avancen una vuelta completa más desde donde están.
    // Por ejemplo, si pasos_por_vuelta = 3200 (motor NEMA17 con 1/16 microstepping),
    // entonces los motores avanzarán 3200 pasos adicionales
    posiciones[0] = motor1.currentPosition() + pasos_por_vuelta; // motor1
    posiciones[1] = motor2.currentPosition() + pasos_por_vuelta; // motor2

    // MOVER AMBOS MOTORES SINCRONIZADOS

    // multi.moveTo(posiciones);
    // Solo fija o guarda el destino al que deben ir los motores, se ejecuta una sola vez, antes de moverse.
    // “Motores, su nuevo destino será este, prepárense”
    multi.moveTo(posiciones);
    // multi.runSpeedToPosition();
    // Mueve los motores paso a paso hasta llegar al destino.
    // Se ejecuta muchas veces o bloquea el programa hasta que llegan
    // Calcula cuántos pasos faltan para cada motor.
    // Sincroniza las velocidades para que todos lleguen al mismo tiempo.
    // Los va moviendo paso a paso hasta que llegan.
    multi.runSpeedToPosition();
}

// MOVER AMBOS MOTORES 1 VUELTA COMPLETA HACIA LA IZQUIERDA (ver funcion mover_derecha())
void mover_izquierda()
{
    long posiciones[2];
    posiciones[0] = motor1.currentPosition() - pasos_por_vuelta;
    posiciones[1] = motor2.currentPosition() - pasos_por_vuelta;
    multi.moveTo(posiciones);
    multi.runSpeedToPosition();
}
