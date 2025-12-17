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
const int reduccion = 9;                // Relación de reducción del motor
const int microstepping = 16;           // Microstepping configurado en el driver
const float HOMING_FAST_SPEED = 2400.0; // pasos/s
const float HOMING_FINE_SPEED = 1200.0; // pasos/s
const float HOMING_ACCEL = 1000.0;      // pasos/s²

const long STEPS_90_DEG = 7200; // AJUSTAR a tu mecánica

// ------------------ Objetos ------------------
AccelStepper motor(AccelStepper::DRIVER, MOTOR_STEP, MOTOR_DIR);
Bounce debouncer = Bounce();

// =======================================================================
//                    FUNCIÓN DE HOMING (1 IMÁN)
// =======================================================================
bool homingSingleMagnet(
    AccelStepper &motor,
    uint8_t hallPin,
    long maxSteps90,
    float fastSpeed,
    float fineSpeed)
{
    const unsigned long TIMEOUT_MS = 15000;
    unsigned long startT;

    motor.setCurrentPosition(0);

    // ---------- FASE 1: salir del imán si arrancamos encima ----------
    motor.setSpeed(fineSpeed);
    startT = millis();
    while (digitalRead(hallPin) == LOW) // LOW = imán
    {
        motor.runSpeed();
        if (abs(motor.currentPosition()) > maxSteps90)
            return false;
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }

    // ---------- FASE 2: buscar imán CW ----------
    motor.setSpeed(fastSpeed);
    startT = millis();
    while (digitalRead(hallPin) == HIGH)
    {
        motor.runSpeed();
        if (motor.currentPosition() > maxSteps90)
            break;
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }

    // ---------- Si no se encontró, buscar CCW ----------
    if (digitalRead(hallPin) == HIGH)
    {
        // NO usar moveTo()
        // invertir dirección inmediatamente, misma velocidad rápida
        motor.setSpeed(-fastSpeed);
        startT = millis();

        while (digitalRead(hallPin) == HIGH)
        {
            motor.runSpeed();
            if (motor.currentPosition() < -maxSteps90)
                return false;
            if (millis() - startT > TIMEOUT_MS)
                return false;
        }
    }

    // ---------- FASE 3: transición a búsqueda fina ----------

    // pequeño retroceso para salir del imán
    motor.setSpeed(-fineSpeed);
    for (int i = 0; i < 100; i++) // AJUSTA: 50–200 pasos
        motor.runSpeed();

    // ahora entrar lentamente al imán
    motor.setSpeed(fineSpeed);
    while (digitalRead(hallPin) == HIGH)
        motor.runSpeed();

    long posEntrada = motor.currentPosition();

    // salir del imán
    while (digitalRead(hallPin) == LOW)
        motor.runSpeed();

    long posSalida = motor.currentPosition();

    long centro = (posEntrada + posSalida) / 2;

    // ---------- FASE 4: ir al centro ----------
    motor.moveTo(centro);
    while (motor.distanceToGo() != 0)
        motor.run();

    motor.setCurrentPosition(0);
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
    debouncer.update();

    if (debouncer.fell())
    {
        Serial.println("🔹 Iniciando homing...");

        bool ok = homingSingleMagnet(
            motor,
            HALL_PIN,
            STEPS_90_DEG,
            HOMING_FAST_SPEED,
            HOMING_FINE_SPEED);

        if (ok)
        {
            Serial.println("✅ Homing OK. Centro establecido como 0.");
            digitalWrite(LED_PIN, HIGH);
        }
        else
        {
            Serial.println("❌ ERROR de homing.");
            while (1)
                ; // bloqueo seguro
        }
    }
}
