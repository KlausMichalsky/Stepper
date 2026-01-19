// =======================================================================
//                🔸 H O M I N G  —  L I N E A R (SCARA) 🔸
// =======================================================================
//  Archivo    : Stepper--Homing_Linear_KY-035_Hall.ino
//  Autor      : Klaus Michalsky
//  Fecha      : 2025-01-19
//
//  DESCRIPCIÓN
//  -----------------------------------------------------------------------
//  Rutina de homing para UN motor paso a paso usando AccelStepper
//  y un estadoAnteriorSensor Hall KY-035 (activo en LOW) y activando el homing con un botón
//
//  El algoritmo:
//   • Limita la búsqueda a ±10mm mecánicos durante el homing
//   • Arriba flancos de entrada del imán, abajo flanco de salida
//   • Calcula flanco de entrada del imán
//   • Define ese flanco como posición 0 (referencia absoluta)
//   • Usa velocidades rápidas y lentas para optimizar tiempo y precisión
//   • Implementa un timeout y manejo de errores
//
//  HARDWARE
//  -----------------------------------------------------------------------
//   • MCU        : Nano / (opcion para RP2040 cambiando pins)
//   • Driver     : Step/Dir compatible con AccelStepper
//   • Botón      : Inicio de homing (con debounce)
//   • LED        : Estado del homing
//
//  NOTAS IMPORTANTES
//  -----------------------------------------------------------------------
//   • NO usa AS5600 (este archivo es solo para KY-035)
//   • No se usan interrupciones para el estadoAnteriorSensor Hall
//   • No se usa moveTo() durante la búsqueda (solo runSpeed())
//
//  ESTADO
//  -----------------------------------------------------------------------
//  ⚠️ en desarrollo
// =======================================================================

#include <Arduino.h>
#include <AccelStepper.h>
#include <Bounce2.h>

// ------------------ Pines ------------------
#define MOTOR_DIR 5
#define MOTOR_STEP 4
#define MOTOR_ENABLE 3
#define HALL_PIN 2
#define BOTON_PIN A0

// ------------------ Parámetros ------------------
const int MICROSTEPPING = 8;
const int REDUCCION = 1;
const int PASOS_POR_VUELTA_MOTOR = 200;

const float HOMING_VEL_RAPIDA = 1000.0;
const float HOMING_VEL_LENTA = 1000.0;
const float HOMING_ACCEL = 1000.0;
const unsigned long HOMING_TIMEOUT = 3000; // 3 segundos

// ------------------ Objetos ------------------
AccelStepper motor(AccelStepper::DRIVER, MOTOR_STEP, MOTOR_DIR);
Bounce debouncer;

void setup()
{
    Serial.begin(115200);

    // setPinsInverted(bool directionInvert, bool stepInvert, bool enableInvert);
    // true significa que la señal se invierte lógicamente
    // Ejemplo: si enableInvert = true, entonces LOW habilita el motor y HIGH lo deshabilita
    motor.setPinsInverted(true, false, false);

    pinMode(MOTOR_ENABLE, OUTPUT);
    digitalWrite(MOTOR_ENABLE, LOW); // Desactivar driver

    pinMode(HALL_PIN, INPUT_PULLUP);
    pinMode(BOTON_PIN, INPUT_PULLUP);
    debouncer.attach(BOTON_PIN);
    debouncer.interval(25);

    motor.setMaxSpeed(HOMING_VEL_RAPIDA);
    motor.setAcceleration(HOMING_ACCEL);
    motor.setCurrentPosition(0);

    Serial.println("Sistema listo. Presiona el botón para homing.");
    digitalWrite(MOTOR_ENABLE, HIGH); // Activar driver
}

void loop()
{
    debouncer.update();
    if (debouncer.fell())
    {
        Serial.println("Iniciando homing...");
        if (digitalRead(HALL_PIN) == LOW)
        {
            Serial.println("Imán detectado");
            Serial.println(digitalRead(HALL_PIN));

            /*// Mover alejándose del imán
            motor.setMaxSpeed(HOMING_VEL_RAPIDA);
            motor.moveTo(200); // Mover lejos del imán direccion opuesta al motor
            unsigned long startTime = millis();
            while (motor.distanceToGo() != 0)
            {
                motor.run();
                if (millis() - startTime > HOMING_TIMEOUT)
                {
                    Serial.println("Error: Timeout al alejarse del imán");
                    return;
                }
            }*/
        }
        else
        {
            Serial.println("Imán NO detectado");
            Serial.println(digitalRead(HALL_PIN));
        }
    }
}
