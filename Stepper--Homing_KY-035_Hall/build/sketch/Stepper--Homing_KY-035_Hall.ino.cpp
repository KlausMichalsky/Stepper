#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_KY-035_Hall\\Stepper--Homing_KY-035_Hall.ino"
// =======================================================================
//                    🔸 P I C O  —  H O M I N G  (SCARA) 🔸
// =======================================================================
//  Archivo    : Stepper--Homing_KY-035_Hall.ino
//  Autor      : Klaus Michalsky
//  Fecha      : 2025-12-11
// -----------------------------------------------------------------------
//  DESCRIPCIÓN
//  -----------------------------------------------------------------------
//  Rutina de homing para UN motor paso a paso usando AccelStepper
//  y UN sensor Hall KY-035 (activo en LOW) y ctivando el homing con un botón
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
//   • MCU        : Nano / (opcion para RP2040 cambiando pins)
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
#define BTN_HOME A0 // A0 Nano -> cambiar para el Pico

// ------------------ Parámetros ------------------
const int microstepping = 16;
const int reduccion = 1;
const int pasos_por_vuelta_motor = 200;

const float HOMING_FAST_SPEED = 2400.0;
const float HOMING_FINE_SPEED = 1200.0;
const float HOMING_ACCEL = 1000.0;

const long STEPS_90_DEG = microstepping * reduccion * pasos_por_vuelta_motor / 4; // 200 pasos por vuelta, 1/4 de vuelta = 90 grados
const unsigned long HOMING_TIMEOUT = 15000;

// ------------------ Objetos ------------------
AccelStepper motor(AccelStepper::DRIVER, MOTOR_STEP, MOTOR_DIR);
Bounce debouncer;

// ------------------ Estado de Homing ------------------
// A partir de ahora, existe un tipo llamado HomingState que solo puede tomar uno de estos valores
// 🧠 Entonces, formalmente:
// Elemento	Qué es
// enum HomingState { ... }	Definición de un tipo
// HomingState	El tipo de dato
// HOMING_IDLE	Un valor válido de ese tipo
// homingState	Variable de ese tipo
enum HomingState
{
    HOMING_IDLE,
    HOMING_EXIT_MAGNET,
    HOMING_SEARCH_CW,
    HOMING_SEARCH_CCW,
    HOMING_BACK_OFF,
    HOMING_FINE_ENTRY,
    HOMING_FINE_EXIT,
    HOMING_MOVE_CENTER,
    HOMING_DONE,
    HOMING_ERROR
};

// ------------------ Control y variables de Homing ------------------
HomingState homingState = HOMING_IDLE; // declara y asigna estado inicial
unsigned long homingStartTime = 0;

long posEntrada = 0;
long posSalida = 0;
long centro = 0;

// ⚠ bandera de error latched
bool homingFault = false;

// ======================================================
//                        SETUP
// ======================================================
#line 108 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_KY-035_Hall\\Stepper--Homing_KY-035_Hall.ino"
void setup();
#line 137 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_KY-035_Hall\\Stepper--Homing_KY-035_Hall.ino"
void loop();
#line 173 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_KY-035_Hall\\Stepper--Homing_KY-035_Hall.ino"
void homingStep();
#line 108 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_KY-035_Hall\\Stepper--Homing_KY-035_Hall.ino"
void setup()
{
    Serial.begin(115200);

    // setPinsInverted(bool directionInvert, bool stepInvert, bool enableInvert);
    // true significa que la señal se invierte lógicamente
    // Ejemplo: si enableInvert = true, entonces LOW habilita el motor y HIGH lo deshabilita
    motor.setPinsInverted(true, false, false);

    pinMode(HALL_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE, OUTPUT);
    pinMode(BTN_HOME, INPUT_PULLUP);

    digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
    digitalWrite(LED_PIN, LOW);

    debouncer.attach(BTN_HOME);
    debouncer.interval(25);

    motor.setMaxSpeed(HOMING_FAST_SPEED);
    motor.setAcceleration(HOMING_ACCEL);

    Serial.println("Sistema listo. Presiona el botón para homing.");
}

// ======================================================
//                         LOOP
// ======================================================
void loop()
{
    debouncer.update();

    // 🔄 Reset de homingFault al soltar el botón
    if (debouncer.rose() && digitalRead(BTN_HOME) == HIGH)
    {
        homingFault = false;
        Serial.println("🔄 Homing reset");
    }

    // 🔒 si hay error latched, no hacemos nada
    if (homingFault)
    {
        return;
    }
    // 🔹 Inicia homing al apretar el botón
    if (debouncer.fell() && homingState == HOMING_IDLE)
    {
        digitalWrite(MOTOR_ENABLE, LOW); // habilita motor
        digitalWrite(LED_PIN, LOW);
        Serial.println("🔹 Iniciando homing...");
        motor.setCurrentPosition(0);
        homingStartTime = millis();
        homingState = HOMING_EXIT_MAGNET;
    }

    if (homingState != HOMING_IDLE)
    {
        homingStep();
    }
}

// ======================================================
//                  HOMING STEP (NO BLOQUEA)
// ======================================================
void homingStep()
{
    if (millis() - homingStartTime > HOMING_TIMEOUT)
    {
        homingState = HOMING_ERROR;
    }

    switch (homingState)
    {
    case HOMING_EXIT_MAGNET:
        motor.setSpeed(HOMING_FINE_SPEED);
        if (digitalRead(HALL_PIN) == LOW)
        {
            motor.runSpeed();
        }
        else
        {
            homingState = HOMING_SEARCH_CW;
        }
        break;

    case HOMING_SEARCH_CW:
        motor.setSpeed(HOMING_FAST_SPEED);
        motor.runSpeed();

        if (digitalRead(HALL_PIN) == LOW)
        {
            homingState = HOMING_BACK_OFF;
        }
        else if (motor.currentPosition() > STEPS_90_DEG)
        {
            homingState = HOMING_SEARCH_CCW;
        }
        break;

    case HOMING_SEARCH_CCW:
        motor.setSpeed(-HOMING_FAST_SPEED);
        motor.runSpeed();

        if (digitalRead(HALL_PIN) == LOW)
        {
            homingState = HOMING_BACK_OFF;
        }
        else if (motor.currentPosition() < -STEPS_90_DEG)
        {
            homingState = HOMING_ERROR;
        }
        break;

    case HOMING_BACK_OFF:
        motor.setSpeed(-HOMING_FINE_SPEED);
        motor.runSpeed();

        if (digitalRead(HALL_PIN) == HIGH)
        {
            homingState = HOMING_FINE_ENTRY;
        }
        break;

    case HOMING_FINE_ENTRY:
        motor.setSpeed(HOMING_FINE_SPEED);
        motor.runSpeed();

        if (digitalRead(HALL_PIN) == LOW)
        {
            posEntrada = motor.currentPosition();
            homingState = HOMING_FINE_EXIT;
        }
        break;

    case HOMING_FINE_EXIT:
        motor.setSpeed(HOMING_FINE_SPEED);
        motor.runSpeed();

        if (digitalRead(HALL_PIN) == HIGH)
        {
            posSalida = motor.currentPosition();
            centro = (posEntrada + posSalida) / 2;
            motor.moveTo(centro);
            homingState = HOMING_MOVE_CENTER;
        }
        break;

    case HOMING_MOVE_CENTER:
        motor.run();
        if (motor.distanceToGo() == 0)
        {
            motor.setCurrentPosition(0);
            homingState = HOMING_DONE;
        }
        break;

    case HOMING_DONE:
        Serial.println("✅ Homing OK. Centro = 0");
        digitalWrite(LED_PIN, HIGH);
        homingState = HOMING_IDLE;
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        break;

    case HOMING_ERROR:
        Serial.println("❌ ERROR de homing");
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        homingFault = true;               // marca la falla
        homingState = HOMING_IDLE;        // vuelve a IDLE
        break;

    default:
        break;
    }
}
