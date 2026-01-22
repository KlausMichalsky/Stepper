#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_linear_Hall\\Stepper--Homing_linear_Hall.ino"
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
#define LED_PIN 6
#define BOTON_PIN A0 // A0 Nano -> cambiar para el Pico

// ------------------ Parámetros ------------------
const int MICROSTEPPING = 8;
const int REDUCCION = 1;
const int PASOS_POR_VUELTA_MOTOR = 200;

const float HOMING_VEL_RAPIDA = 1000.0;
const float HOMING_VEL_LENTA = 500.0;
const float HOMING_ACCEL = 1000.0;

const unsigned long HOMING_TIMEOUT = 12000; // 12 segundos

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
    HOMING_BUSCAR_RAPIDO_ABAJO,
    HOMING_BUSCAR_RAPIDO_ARRIBA,
    HOMING_MOVER_REFERENCIA,
    HOMING_OK,
    HOMING_ERROR
};

// ------------------ Variables ------------------
EstadoHoming estadoHoming = HOMING_INACTIVO; // estado inicial
unsigned long homingStartTime = 0;           // tiempo de inicio de homing

long referencia = 0; // posición de homing
long flanco = 0;     // posición de salida del imán

bool homingFallo = false; // marca si hubo falla en homing

// ======================================================
//                        SETUP
// ======================================================
#line 96 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_linear_Hall\\Stepper--Homing_linear_Hall.ino"
void setup();
#line 126 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_linear_Hall\\Stepper--Homing_linear_Hall.ino"
void loop();
#line 171 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_linear_Hall\\Stepper--Homing_linear_Hall.ino"
void homingStep();
#line 96 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper--Homing_linear_Hall\\Stepper--Homing_linear_Hall.ino"
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
    pinMode(BOTON_PIN, INPUT_PULLUP);

    digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
    digitalWrite(LED_PIN, LOW);

    debouncer.attach(BOTON_PIN);
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
    // fell() → el botón pasó de HIGH → LOW
    // rose() → el botón pasó de LOW → HIGH
    // ⚠️ Importante: por qué no resetear en fell()
    // Si lo hicieras al presionar:
    // podrías borrar el error sin intención
    // incluso mientras el botón sigue apretado
    // creando estados raros
    // 👉 Resetear al soltar es lo correcto.
    if (debouncer.rose() && homingFallo)
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
    if (!homingFallo && debouncer.fell() && estadoHoming == HOMING_INACTIVO)
    {
        digitalWrite(MOTOR_ENABLE, LOW); // habilita motor
        digitalWrite(LED_PIN, LOW);
        Serial.println("🔹 Iniciando homing...");
        motor.setCurrentPosition(0);
        homingStartTime = millis(); // guarda tiempo de inicio de homing al momento de presionar el botón
        estadoHoming = HOMING_BUSCAR_RAPIDO_ABAJO;
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
    // invierte la logica del HAll (imán presente = LOW)
    bool imanPresente = !digitalRead(HALL_PIN);

    // ⏱️ Timeout de homing (si tiempo de homing excede el límite)
    if (millis() - homingStartTime > HOMING_TIMEOUT) //
    {
        estadoHoming = HOMING_ERROR;
    }

    switch (estadoHoming)
    {
    case HOMING_BUSCAR_RAPIDO_ABAJO:
        motor.setSpeed(HOMING_VEL_RAPIDA);
        motor.runSpeed();
        if (!imanPresente)
        {
            flanco = motor.currentPosition();
            Serial.print("Flanco de salida en: ");
            Serial.println(flanco);
            motor.moveTo(flanco - 3000);
            estadoHoming = HOMING_MOVER_REFERENCIA;
        }
        break;

    case HOMING_MOVER_REFERENCIA:
        motor.run();
        if (motor.distanceToGo() == 0)
        {
            motor.setCurrentPosition(0);
            estadoHoming = HOMING_OK;
            digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        }
        break;

    case HOMING_OK:
        Serial.println("✅ Homing OK. Referencia = 0");
        digitalWrite(LED_PIN, HIGH);
        estadoHoming = HOMING_INACTIVO;
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        break;

    case HOMING_ERROR:
        Serial.println("❌ ERROR de homing");
        digitalWrite(MOTOR_ENABLE, HIGH); // deshabilita motor
        homingFallo = true;               // latch error
        estadoHoming = HOMING_INACTIVO;
        break;
    }
}
