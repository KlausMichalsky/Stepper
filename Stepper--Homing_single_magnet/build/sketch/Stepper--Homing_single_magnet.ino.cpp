#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_single_magnet\\Stepper--Homing_single_magnet.ino"
// =======================================================================
//                    🔸 P I C O  —  H O M I N G  (SCARA) 🔸
// =======================================================================
//  Archivo    : homing_single_magnet.cpp
//  Autor      : Klaus Michalsky
//  Fecha      : 2025-12-11
// -----------------------------------------------------------------------
//  DESCRIPCIÓN
//  -----------------------------------------------------------------------
//  Rutina de homing para UN motor paso a paso usando AccelStepper
//  y UN sensor Hall KY-035 (activo en LOW).
//
//  El algoritmo:
//   • Limita la búsqueda a ±90° mecánicos durante el homing
//   • Detecta flancos de entrada y salida del imán
//   • Calcula el centro geométrico del imán
//   • Define ese centro como posición 0 (referencia absoluta)
//
//  -----------------------------------------------------------------------
//  HARDWARE
//  -----------------------------------------------------------------------
//   • MCU        : Raspberry Pi Pico / RP2040
//   • Driver     : Step/Dir compatible con AccelStepper
//   • Sensor     : KY-035 (Hall, salida digital, LOW = imán)
//   • Botón      : Inicio de homing (con debounce)
//   • LED        : Estado del homing
//
//  -----------------------------------------------------------------------
//  NOTAS IMPORTANTES
//  -----------------------------------------------------------------------
//   • NO usa AS5600 (este archivo es solo para KY-035)
//   • No se usan interrupciones para el sensor Hall
//   • No se usa moveTo() durante la búsqueda (solo runSpeed())
//   • El valor STEPS_90_DEG debe ajustarse a la mecánica real
//
//  -----------------------------------------------------------------------
//  ESTADO
//  -----------------------------------------------------------------------
//   ✔ Funcional
//   ⚠ Ajuste fino pendiente (velocidades / pasos de retroceso)
// =======================================================================

#include <Arduino.h>
#include <AccelStepper.h>
#include <Bounce2.h>

// ------------------ Pines ------------------
#define MOTOR_ENABLE 6
#define MOTOR_DIR 7
#define MOTOR_STEP 8

#define HALL_PIN 3
#define LED_PIN 2
#define BTN_HOME 28

// ------------------ Parámetros ------------------
const int stepsPerRevolution = 200;     // Pasos por revolución del motor
const int microstepping = 16;           // Microstepping configurado en el driver
const int reduccion = 1;                // Relación de reducción del motor
const float HOMING_FAST_SPEED = 1400.0; // pasos/s
const float HOMING_FINE_SPEED = 600.0;  // pasos/s
const float HOMING_ACCEL = 1000.0;      // pasos/s²

const long STEPS_90_DEG = (stepsPerRevolution * microstepping * reduccion) / 4; // AJUSTAR a tu mecánica

// ------------------ Objetos ------------------
AccelStepper motor(AccelStepper::DRIVER, MOTOR_STEP, MOTOR_DIR);
Bounce debouncer = Bounce();

// =======================================================================
//                    FUNCIÓN DE HOMING (1 IMÁN)
// =======================================================================

#line 74 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_single_magnet\\Stepper--Homing_single_magnet.ino"
bool homingSingleMagnet(AccelStepper &motor, uint8_t hallPin, float fastSpeed);
#line 102 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_single_magnet\\Stepper--Homing_single_magnet.ino"
void setup();
#line 126 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_single_magnet\\Stepper--Homing_single_magnet.ino"
void loop();
#line 74 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_single_magnet\\Stepper--Homing_single_magnet.ino"
bool homingSingleMagnet(AccelStepper &motor, uint8_t hallPin, float fastSpeed)
{
    // Girar CW hasta encontrar el imán
    motor.setSpeed(fastSpeed);           // + = CW
    while (digitalRead(hallPin) == HIGH) // mientras NO haya imán
    {
        motor.runSpeed();
    }
    // entrar lentamente hacia el segundo borde
    motor.setSpeed(HOMING_FINE_SPEED);
    while (digitalRead(hallPin) == LOW) // mientras haya imán
    {
        motor.runSpeed();
    }
    {
        motor.runSpeed();
    }

    // Motor llega al segundo borde
    motor.setSpeed(0); // detener motor

    // homing completado exitosamente
    return true;
}

// =======================================================================
//                              SETUP
// =======================================================================
void setup()
{
    Serial.begin(115200);

    pinMode(HALL_PIN, INPUT_PULLUP); // KY-035
    pinMode(LED_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE, OUTPUT);
    pinMode(BTN_HOME, INPUT_PULLUP);

    digitalWrite(MOTOR_ENABLE, LOW); // habilita driver (ajusta si es activo HIGH)
    digitalWrite(LED_PIN, LOW);

    debouncer.attach(BTN_HOME);
    debouncer.interval(25);

    motor.setMaxSpeed(HOMING_FAST_SPEED);
    motor.setAcceleration(HOMING_ACCEL);

    Serial.println("Sistema listo. Presiona el botón para homing.");
}

// =======================================================================
//                               LOOP
// =======================================================================
void loop()
{
    // Actualizar el debouncer del botón
    debouncer.update();

    if (debouncer.fell())
    {
        Serial.println("🔹 Iniciando homing...");

        // Llamamos a la función mínima de homing
        bool ok = homingSingleMagnet(
            motor,            // motor a controlar
            HALL_PIN,         // pin del sensor Hall
            HOMING_FAST_SPEED // velocidad rápida CW
        );

        if (ok)
        {
            Serial.println("✅ Homing OK. Imán detectado.");
            digitalWrite(LED_PIN, HIGH); // indicar homing completado
        }
        else
        {
            Serial.println("❌ ERROR de homing.");
            while (1)
                ; // bloqueo seguro
        }
    }
}

