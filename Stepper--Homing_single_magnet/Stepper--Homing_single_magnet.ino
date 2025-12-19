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

HomingState homingState = HOMING_IDLE;
unsigned long homingStartTime = 0;

long posEntrada = 0;
long posSalida = 0;
long centro = 0;

// ======================================================
//                        SETUP
// ======================================================
void setup()
{
    Serial.begin(115200);

    pinMode(HALL_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(MOTOR_ENABLE, OUTPUT);
    pinMode(BTN_HOME, INPUT_PULLUP);

    digitalWrite(MOTOR_ENABLE, LOW);
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

    if (debouncer.fell() && homingState == HOMING_IDLE)
    {
        Serial.println("🔹 Iniciando homing....");
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
        break;

    case HOMING_ERROR:
        Serial.println("❌ ERROR de homing");
        while (1)
            ; // bloqueo seguro
        break;

    default:
        break;
    }
}