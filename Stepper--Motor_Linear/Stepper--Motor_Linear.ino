/************************************************************************************************************
 🔹 CONTROL PARA CENTRADO MANUAL DEL BLOQUE (SIRVE PARA EVITAR DAÑO DEL MOTOR)🔹
  - Motor paso a paso controlado con driver STEP1/DIR1.
  - Pines usados: DIR = 10, STEP = 11, ENABLE = 12.
  - Usa AccelStepper con velocidad y aceleración configuradas.
  - Ejecuta movimiento en ambas direcciones empezando en direccion opuesta al motor (~1cm)
  - Ciclo continuo con pausa de 3 segundo entre movimientos.

  Hardware usado
    - DC 5V/12V 15mm/20mm Schrittmotor 55mm Hub (Aliexpress: Motor-house Store)
    - Driver TMC2209 en modo UART
    - Arduino Nano
    - TMC2209 A2->rojo, A1->negro, B1->amarillo, B2->azul
    - TMC2209 MS1, MS2, -> GND

    K. Michalsky – 11.2025
************************************************************************************************************/

#include <AccelStepper.h>

// ==== PINES ====
#define DIR 5
#define STEP 4
#define ENABLE 3

// ==== OBJETOS ====
// AccelStepper motor1(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper motor(AccelStepper::DRIVER, STEP, DIR);

// ==== CONFIGURACIÓN DE MOTORES ====
// 200 pasos (~1cm) con microstepping 1/8
const int microstepping = 8;
const int pasos = 200 * microstepping;
const int direccion = -1;

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
    motor.moveTo(direccion * pasos);
    while (motor.distanceToGo() != 0)
    {
        motor.run();
    }
    delay(3000);

    // --- Motor: volver a 0 ---
    motor.moveTo(-direccion * pasos);
    while (motor.distanceToGo() != 0)
    {
        motor.run();
    }
    delay(3000);
}
