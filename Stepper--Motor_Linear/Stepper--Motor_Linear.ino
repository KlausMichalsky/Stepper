/************************************************************************************************************
 🔹 CONTROL PARA CENTRADO MANUAL DEL BLOQUE (SIRVE PARA EVITAR DAÑO DEL MOTOR)🔹
  - Motor paso a paso controlado con driver STEP1/DIR1.
  - Pines usados: DIR = 10, STEP = 11, ENABLE = 12.
  - Usa AccelStepper con velocidad y aceleración configuradas.
  - Ejecuta movimiento en ambas direcciones empezando en direccion opuesta al motor (mas o menos 1cm)
  - Ciclo continuo con pausa de 3 segundo entre movimientos.

  Hardware usado
    - DC 5V/12V 15mm/20mm Schrittmotor 55mm Hub (Aliexpress: Motor-house Store)
    - Driver TMC2209 en modo UART
    - TMC2209 A2->rojo, A1->negro, B1->amarillo, B2->azul
    - TMC2209 MS1, MS2, -> GND

    K. Michalsky – 11.2025
************************************************************************************************************/

#include <AccelStepper.h>

// ==== PINES ====
#define DIR 10
#define STEP 11
#define ENABLE 12

// ==== OBJETOS ====
// AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor(AccelStepper::DRIVER, STEP, DIR);

// ==== CONFIGURACIÓN DE MOTORES ====
// Motores de 200 pasos por vuelta con microstepping 1/16
// 200 * 16 = 3200 pasos por vuelta
const int microstepping = 8;
const int pasosPorVuelta = 200 * microstepping;
const int vueltas = 1;

void setup()
{
    pinMode(ENABLE, OUTPUT);
    digitalWrite(ENABLE, LOW); // LOW = driver activado

    motor.setMaxSpeed(2000);
    motor.setAcceleration(1500);

    motor.setCurrentPosition(0);
}

void loop()
{
    // --- Motor: 1cm direccion opuesta al motor ---
    motor.moveTo(
        vueltas * pasosPorVuelta);
    while (motor.distanceToGo() != 0)
    {
        motor.run();
    }
    delay(3000);

    // --- Motor: volver a 0 ---
    motor.moveTo(-vueltas * pasosPorVuelta);
    while (motor.distanceToGo() != 0)
    {
        motor.run();
    }
    delay(3000);
}
