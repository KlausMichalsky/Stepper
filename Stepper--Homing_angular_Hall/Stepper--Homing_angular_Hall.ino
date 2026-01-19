// =======================================================================
//               🔹 H O M I N G  —  A N G U L A R (SCARA) 🔹
// =======================================================================
//  Archivo    : Stepper--Homing_Angular_KY-035_Hall.ino
//  Autor      : Klaus Michalsky
//  Fecha      : 2025-12-11
//
//  DESCRIPCIÓN
//  -----------------------------------------------------------------------
//  Rutina de homing para UN motor paso a paso usando AccelStepper
//  y un estadoAnteriorSensor Hall KY-035 (activo en LOW) y activando el homing con un botón
//
//  El algoritmo:
//   • Limita la búsqueda a ±90° mecánicos durante el homing
//   • Detecta flancos de entrada y salida del imán
//   • Calcula el centro geométrico del imán
//   • Define ese centro como posición 0 (referencia absoluta)
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
//  ✅ Funcional
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
const int MICROSTEPPING = 16;
const int REDUCCION = 1;
const int PASOS_POR_VUELTA_MOTOR = 200;

const float HOMING_VEL_RAPIDA = 500.0;
const float HOMING_VEL_LENTA = 100.0;
const float HOMING_ACCEL = 1000.0;

// 200 pasos por vuelta, 1/4 de vuelta = 90 grados
const long STEPS_90_DEG = MICROSTEPPING * REDUCCION * PASOS_POR_VUELTA_MOTOR / 4;
const unsigned long HOMING_TIMEOUT = 15000;

// ------------------ Objetos ------------------
AccelStepper motor(AccelStepper::DRIVER, MOTOR_STEP, MOTOR_DIR);
Bounce debouncer;

// ------------------ Estado de Homing ------------------
// A partir de ahora, existe un tipo llamado EstadoHoming
// que solo puede tomar uno de estos valores
// enum EstadoHoming { ... }	Definición de un tipo
// EstadoHoming	                El tipo de dato
// HOMING_INACTIVO, ...         Valores posibles del tipo EstadoHoming
// estadoHoming	                Variable de ese tipo
enum EstadoHoming
{
    HOMING_INACTIVO,
    HOMING_BUSCAR_PRIMER_FLANCO_CW,
    HOMING_BUSCAR_SEGUNDO_FLANCO_CW,
    HOMING_BUSCAR_PRIMER_FLANCO_CCW,
    HOMING_BUSCAR_SEGUNDO_FLANCO_CCW,
    HOMING_BUSCAR_RAPIDO_CW,
    HOMING_BUSCAR_RAPIDO_CCW,
    HOMING_INVERTIR_FLANCO_CW,
    HOMING_INVERTIR_FLANCO_CCW,
    HOMING_CALC_CENTRO,
    HOMING_MOVER_CENTRO,
    HOMING_OK,
    HOMING_ERROR
};

// ------------------ Control y variables de Homing ------------------
EstadoHoming estadoHoming = HOMING_INACTIVO;
unsigned long homingStartTime = 0;

int8_t CW = 1;          // sentido horario
int8_t CCW = -1;        // sentido antihorario
long primerFlanco = 0;  // posición de entrada al imán
long segundoFlanco = 0; // posición de salida del imán
long centro = 0;        // posición central calculada

// ⚠️ bandera de error
bool homingFallo = false;

// ======================================================
//                        SETUP
// ======================================================
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

    motor.setMaxSpeed(HOMING_VEL_RAPIDA);
    motor.setAcceleration(HOMING_ACCEL);

    Serial.println("Sistema listo. Presiona el botón para homing.");
}

// ======================================================
//                         LOOP
// ======================================================
void loop()
{
    debouncer.update();

    // 🔄 Reset de homingFallo al soltar el botón
    if (debouncer.rose() && digitalRead(BTN_HOME) == HIGH)
    {
        homingFallo = false;
        Serial.println("🔄 Homing reset");
    }

    // 🔒 si hay error latched, no hacemos nada
    if (homingFallo)
    {
        return;
    }
    // 🔹 Inicia homing al apretar el botón
    if (debouncer.fell() && estadoHoming == HOMING_INACTIVO)
    {
        digitalWrite(MOTOR_ENABLE, LOW); // habilita motor
        digitalWrite(LED_PIN, LOW);
        Serial.println("🔹 Iniciando homing...");
        motor.setCurrentPosition(0);
        homingStartTime = millis(); // guarda tiempo de inicio de homing al momento de presionar el botón
        estadoHoming = HOMING_BUSCAR_RAPIDO_CW;
    }

    if (estadoHoming != HOMING_INACTIVO)
    {
        homingStep();
    }
}

// ======================================================
//                  HOMING STEP (NO BLOQUEA)
// ======================================================
void homingStep()
{
    // invierte la logica del KY-035 (LOW = imán presente)
    bool imanPresente = (digitalRead(HALL_PIN) == LOW);

    // ⏱️ Timeout de homing (si tiempo de homing excede el límite)
    if (millis() - homingStartTime > HOMING_TIMEOUT) //
    {
        estadoHoming = HOMING_ERROR;
    }

    switch (estadoHoming)
    {
    case HOMING_BUSCAR_RAPIDO_CW:
        motor.setSpeed(CW * HOMING_VEL_RAPIDA);
        motor.runSpeed();
        if (imanPresente)
        {
            estadoHoming = HOMING_BUSCAR_PRIMER_FLANCO_CW;
        }

        else if (motor.currentPosition() > STEPS_90_DEG)
        {
            estadoHoming = HOMING_BUSCAR_RAPIDO_CCW;
        }
        break;

    case HOMING_BUSCAR_RAPIDO_CCW:
        motor.setSpeed(CCW * HOMING_VEL_RAPIDA);
        motor.runSpeed();
        if (imanPresente)
        {
            estadoHoming = HOMING_BUSCAR_PRIMER_FLANCO_CCW;
        }
        else if (motor.currentPosition() < -STEPS_90_DEG)
        {
            estadoHoming = HOMING_ERROR; // no se encontró el imán en el rango
        }
        break;

    case HOMING_BUSCAR_PRIMER_FLANCO_CW:
        motor.setSpeed(CW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (!imanPresente)
        {
            primerFlanco = motor.currentPosition();
            Serial.print("Primer flanco en: ");
            Serial.println(primerFlanco);
            estadoHoming = HOMING_INVERTIR_FLANCO_CW;
        }
        break;

    case HOMING_BUSCAR_PRIMER_FLANCO_CCW:
        motor.setSpeed(CCW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (!imanPresente)
        {
            primerFlanco = motor.currentPosition();
            Serial.print("Primer flanco en: ");
            Serial.println(primerFlanco);
            estadoHoming = HOMING_INVERTIR_FLANCO_CCW;
        }
        break;

    case HOMING_INVERTIR_FLANCO_CW:
        motor.setSpeed(CCW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (imanPresente)
        {
            estadoHoming = HOMING_BUSCAR_SEGUNDO_FLANCO_CW;
        }
        break;

    case HOMING_INVERTIR_FLANCO_CCW:
        motor.setSpeed(CW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (imanPresente)
        {
            estadoHoming = HOMING_BUSCAR_SEGUNDO_FLANCO_CCW;
        }
        break;

    case HOMING_BUSCAR_SEGUNDO_FLANCO_CW:
        motor.setSpeed(CCW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (!imanPresente)
        {
            segundoFlanco = motor.currentPosition();
            Serial.print("Segundo flanco en: ");
            Serial.println(segundoFlanco);
            estadoHoming = HOMING_CALC_CENTRO;
        }
        break;

    case HOMING_BUSCAR_SEGUNDO_FLANCO_CCW:
        motor.setSpeed(CW * HOMING_VEL_LENTA);
        motor.runSpeed();
        if (!imanPresente)
        {
            segundoFlanco = motor.currentPosition();
            Serial.print("Segundo flanco en: ");
            Serial.println(segundoFlanco);
            estadoHoming = HOMING_CALC_CENTRO;
        }
        break;

    case HOMING_CALC_CENTRO:
        centro = (primerFlanco + segundoFlanco) / 2;
        Serial.print("Centro calculado en: ");
        Serial.println(centro);
        motor.moveTo(centro);
        estadoHoming = HOMING_MOVER_CENTRO;
        break;

    case HOMING_MOVER_CENTRO:
        motor.run();
        if (motor.distanceToGo() == 0)
        {
            motor.setCurrentPosition(0);
            estadoHoming = HOMING_OK;
            digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        }
        break;

    case HOMING_OK:
        Serial.println("✅ Homing OK. Centro = 0");
        digitalWrite(LED_PIN, HIGH);
        estadoHoming = HOMING_INACTIVO;
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        break;

    case HOMING_ERROR:
        Serial.println("❌ ERROR de homing");
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        homingFallo = true;               // marca la falla
        estadoHoming = HOMING_INACTIVO;   // vuelve a IDLE
        break;

    default:
        break;
    }
}
