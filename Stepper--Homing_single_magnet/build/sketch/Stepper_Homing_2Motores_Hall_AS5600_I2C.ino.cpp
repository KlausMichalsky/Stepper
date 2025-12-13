#line 1 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
// =======================================================================
//                     🔸P I C O   —   H O M I N G (Testcode)🔸
// =======================================================================
//  Archivo    : homing_dual.cpp
//  Autor      : Klaus Michalsky
//  Fecha      : 2025-12-11
// -----------------------------------------------------------------------
//  ▫️ DESCRIPCIÓN
//      Homing dual para dos motores paso a paso usando AccelStepper,
//      con sensores AS5600 (I2C) y KY-035 (activo LOW).
//      Centrado entre 2 imanes y aplicación de offset software a 0° ±180°.
//
//  ▫️ RESPONSABILIDADES
//      - Ejecutar homing preciso de ambos motores.
//      - Filtrar lecturas de AS5600 con filtro exponencial.
//      - Controlar LED de estado durante homing y parpadeos finales.
//      - Manejar inicio mediante botón con debounce y ISR.
// =======================================================================

#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <Bounce2.h>

// ---------- Pines ----------
#define MOTOR1_ENABLE 6
#define MOTOR1_DIR 7
#define MOTOR1_STEP 8

#define MOTOR2_ENABLE 12
#define MOTOR2_DIR 13
#define MOTOR2_STEP 14

#define AS5600_1_SDA 4
#define AS5600_1_SCL 5

#define AS5600_2_SDA 26
#define AS5600_2_SCL 27

#define HALL_1 3
#define HALL_2 15

#define LED_PIN 2
#define BOTON 28

volatile bool estadoBoton = LOW;
Bounce debouncer = Bounce();

// ---------- Objetos ----------
AccelStepper motor1(AccelStepper::DRIVER, MOTOR1_STEP, MOTOR1_DIR);
AccelStepper motor2(AccelStepper::DRIVER, MOTOR2_STEP, MOTOR2_DIR);

// ---------- Parámetros de movimiento (ajustables) ----------
const float HOMING_FAST_SPEED = 2400.0; // pasos/s (búsqueda rápida)
const float HOMING_FINE_SPEED = 1200.0; // pasos/s (búsqueda fina)
const float HOMING_ACCEL = 1000.0;      // pasos/s^2

// ---------- Filtro exponencial ----------
float alpha = 0.10; // coeficiente del filtro
float angFiltrado1 = 0.0, angFiltrado2 = 0.0;
float angOffset1 = 0.0, angOffset2 = 0.0;
bool motor1Centrado = false, motor2Centrado = false;

// ---------- LED parpadeo ----------
unsigned long lastLedToggle = 0;
const unsigned long ledInterval = 300; // ms
bool ledState = false;

unsigned long ledFinalLastToggle = 0;
int ledFinalCount = 0;
bool ledFinalPhase = false; // para los 2 parpadeos finales

// ---------- Utils I2C AS5600 ----------
const uint8_t AS5600_ADDR = 0x36;

// Lee el registro RAW (12 bits) del AS5600 usando el bus pasado (Wire o Wire1)
#line 77 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
uint16_t readAS5600_raw(TwoWire &i2c);
#line 94 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
float rawToDegrees(uint16_t raw);
#line 104 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
bool homingAccel(AccelStepper &motor, uint8_t hallPin, float fastSpeed, float fineSpeed, float accel, long &outCentroSteps);
#line 218 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
void setup();
#line 256 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
void loop();
#line 408 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
void cambiarEstadoBoton();
#line 77 "C:\\Users\\Benutzer1\\Documents\\Arduino\\Stepper\\Stepper_Homing_2Motores_Hall_AS5600_I2C\\Stepper_Homing_2Motores_Hall_AS5600_I2C.ino"
uint16_t readAS5600_raw(TwoWire &i2c)
{
    uint16_t angle = 0;
    i2c.beginTransmission(AS5600_ADDR);
    i2c.write(0x0E); // registro ANGLE (MSB)
    if (i2c.endTransmission(false) != 0)
        return 0; // error
    if (i2c.requestFrom(AS5600_ADDR, (uint8_t)2) == 2)
    {
        uint8_t msb = i2c.read();
        uint8_t lsb = i2c.read();
        angle = ((uint16_t)msb << 8) | (uint16_t)lsb;
        angle &= 0x0FFF; // 12-bit
    }
    return angle;
}

float rawToDegrees(uint16_t raw)
{
    return ((float)raw * 360.0f) / 4096.0f; // raw 0..4095 -> 0..360
}

// ---------- Homing para un motor con Hall (KY-035 activo LOW) ----------
// Se mueve en una dirección hasta detectar LOW (imán).
// Luego retrocede / busca fino, registra posición1,
// invierte sentido y busca la otra activación -> posición2,
// calcula centro y mueve al centro usando moveTo().
bool homingAccel(AccelStepper &motor, uint8_t hallPin, float fastSpeed, float fineSpeed, float accel, long &outCentroSteps)
{
    // Precondición: motor.setMaxSpeed y setAcceleration ya seteados
    // Asegurar que el sensor está en HIGH (no en imán) antes de comenzar; si está LOW, salir de ahí primero
    // (girar un poco para liberarlo)
    const long BACKOFF_STEPS = 50;          // pasos para "despegar" antes de la búsqueda fina
    const unsigned long TIMEOUT_MS = 20000; // timeout arbitrario por si algo falla

    unsigned long startT = millis();

    // 1) búsqueda rápida: dirección positiva
    motor.setSpeed(fastSpeed);
    while (digitalRead(hallPin) == LOW)
    { // si ya está LOW, salimos avanzando un poco
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    // ahora buscar hasta que aparezca LOW (imán)
    startT = millis();
    while (digitalRead(hallPin) == HIGH)
    {
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    // detectado primer borde (rápido). Ahora hacemos búsqueda fina:
    // retrocedemos un poco para aproximar limpiamente desde el lado contrario
    motor.setCurrentPosition(0); // referencia local
    motor.setSpeed(-fineSpeed);
    for (int i = 0; i < BACKOFF_STEPS; ++i)
        motor.runSpeed(); // pequeño backoff

    // avanzar lentamente hasta el punto exacto (flanco)
    startT = millis();
    while (digitalRead(hallPin) == HIGH)
    {
        motor.setSpeed(fineSpeed);
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    // avanzar hasta que vuelva a HIGH (para definir un borde consistente)
    startT = millis();
    while (digitalRead(hallPin) == LOW)
    {
        motor.setSpeed(fineSpeed);
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    // Guardamos posición exacta 1
    long pos1 = motor.currentPosition();

    // 2) cambiar dirección y buscar segundo imán
    // Asegurar que empezamos fuera del imán
    motor.setSpeed(-fineSpeed); // invertimos sentido
    // avanzamos un poco para alejarnos del primer imán
    for (int i = 0; i < BACKOFF_STEPS; ++i)
        motor.runSpeed();

    // ahora buscar la siguiente activación (en sentido invertido)
    startT = millis();
    while (digitalRead(hallPin) == HIGH)
    {
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    // búsqueda fina del segundo borde
    // retroceder un poco y re-enfocar igual que antes
    motor.setSpeed(fineSpeed); // acercar con velocidad fina desde el otro lado
    for (int i = 0; i < BACKOFF_STEPS; ++i)
        motor.runSpeed();

    startT = millis();
    while (digitalRead(hallPin) == HIGH)
    {
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    startT = millis();
    while (digitalRead(hallPin) == LOW)
    {
        motor.runSpeed();
        if (millis() - startT > TIMEOUT_MS)
            return false;
    }
    long pos2 = motor.currentPosition();

    // Si pos2 == pos1 por alguna razón, fallamos
    if (pos1 == pos2)
        return false;

    // Calcular centro (media)
    long centro = (pos1 + pos2) / 2;
    outCentroSteps = centro;

    // Mover al centro (usaremos moveTo y run hasta llegar)
    motor.moveTo(centro);
    while (motor.distanceToGo() != 0)
    {
        motor.run();
        // no timeout aquí por ahora; en caso de problema el codigo quedará bloqueado
    }

    // establecer la posición actual como 0 para el motor
    motor.setCurrentPosition(0);

    return true;
}

// ---------- Setup & Loop ----------
void setup()
{
    Serial.begin(115200);
    attachInterrupt(digitalPinToInterrupt(BOTON), cambiarEstadoBoton, CHANGE);

    debouncer.attach(BOTON, INPUT_PULLUP);
    debouncer.interval(25); // 50 ms de debounce

    // Pines
    pinMode(HALL_1, INPUT_PULLUP); // KY-035 -> normalmente HIGH, LOW cuando imán
    pinMode(HALL_2, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Habilitar drivers (activo LOW en muchos drivers; adaptar si el tuyo es distinto)
    pinMode(MOTOR1_ENABLE, OUTPUT);
    pinMode(MOTOR2_ENABLE, OUTPUT);
    digitalWrite(MOTOR1_ENABLE, LOW); // habilita (si tu driver requiere HIGH para habilitar, cámbialo)
    digitalWrite(MOTOR2_ENABLE, LOW);

    // I2C
    Wire.setSDA(AS5600_1_SDA);
    Wire.setSCL(AS5600_1_SCL);
    Wire.begin();

    Wire1.setSDA(AS5600_2_SDA);
    Wire1.setSCL(AS5600_2_SCL);
    Wire1.begin();

    // Configurar AccelStepper
    motor1.setMaxSpeed(HOMING_FAST_SPEED);
    motor1.setAcceleration(HOMING_ACCEL);
    motor2.setMaxSpeed(HOMING_FAST_SPEED);
    motor2.setAcceleration(HOMING_ACCEL);

    delay(200);
}

void loop()
{
    debouncer.update();
    if (debouncer.fell())
    {


        
        Serial.println("🔹 Inicio Homing motor1...");
        long centroM1 = 0;
        motor1.setMaxSpeed(HOMING_FAST_SPEED);
        motor1.setAcceleration(HOMING_ACCEL);
        bool ok1 = homingAccel(motor1, HALL_1, HOMING_FAST_SPEED, HOMING_FINE_SPEED, HOMING_ACCEL, centroM1);
        if (!ok1)
        {
            Serial.println("⚠️ Homing motor1 falló o timeout.");
        }
        else
        {
            Serial.print("✅ Motor1 centrado (steps): ");
            Serial.println(centroM1);
        }


        // Parpadeo rápido breve mientras se inicia siguiente homing
        digitalWrite(LED_PIN, LOW);
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        delay(200);










        Serial.println("🔹 Inicio Homing motor2...");
        digitalWrite(LED_PIN, HIGH);

        long centroM2 = 0;
        // homing motor2 primero
        motor2.setMaxSpeed(HOMING_FAST_SPEED);
        motor2.setAcceleration(HOMING_ACCEL);
        bool ok2 = homingAccel(motor2, HALL_2, HOMING_FAST_SPEED, HOMING_FINE_SPEED, HOMING_ACCEL, centroM2);
        if (!ok2)
        {
            Serial.println("⚠️ Homing motor2 falló o timeout.");
        }
        else
        {
            Serial.print("✅ Motor2 centrado (steps): ");
            Serial.println(centroM2);
        }




        // HOMING finalizado: setear offsets AS5600 (software) = lecturas actuales --> 0°
        // Leemos los AS5600 (convertir raw->grados) y los guardamos como offset
        uint16_t raw1 = readAS5600_raw(Wire);
        uint16_t raw2 = readAS5600_raw(Wire1);
        angFiltrado1 = rawToDegrees(raw1);
        angFiltrado2 = rawToDegrees(raw2);
        angOffset1 = angFiltrado1;
        angOffset2 = angFiltrado2;

        // Inicializamos el filtro exponencial con la lectura inicial
        // (ya asignado arriba)
        motor1Centrado = ok1;
        motor2Centrado = ok2;

        // Parpadeo final de LEDs: 2 ciclos (1s ON / 1s OFF) -> implementamos en loop principal
        ledFinalLastToggle = millis();
        ledFinalCount = 0;
        ledFinalPhase = true; // empezamos con ON
        digitalWrite(LED_PIN, HIGH);

        // Enviar 0,0 al Pico (motor1, motor2)
        Serial.println("0,0");

        Serial.println("🔹 Homing completado. Offset AS5600 aplicado (software).");

        estadoBoton = !estadoBoton;
    }

    unsigned long now = millis();

    // Parpadeo final: 2 ciclos completos = 4 toggles (ON->OFF->ON->OFF) equivalen a 2 ciclos
    if (ledFinalPhase && ledFinalCount < 4)
    {
        if (now - ledFinalLastToggle >= 1000)
        { // 1s ON/OFF
            bool s = digitalRead(LED_PIN);
            digitalWrite(LED_PIN, !s);
            ledFinalLastToggle = now;
            ledFinalCount++;
        }
        if (ledFinalCount >= 4)
        {
            digitalWrite(LED_PIN, LOW); // apagar al terminar
            ledFinalPhase = false;
        }
    }

    // Si ya están centrados, leer AS5600 y filtrar, luego normalizar a ±180°
    if (motor1Centrado)
    {
        uint16_t r1 = readAS5600_raw(Wire);
        float ang1 = rawToDegrees(r1);
        angFiltrado1 = alpha * ang1 + (1.0 - alpha) * angFiltrado1;
        float angRel1 = angFiltrado1 - angOffset1;
        // normalizar:
        if (angRel1 > 180.0)
            angRel1 -= 360.0;
        if (angRel1 < -180.0)
            angRel1 += 360.0;
        // imprime cada 200ms
        static unsigned long lastPrint1 = 0;
        if (now - lastPrint1 > 200)
        {
            Serial.print("M1 (±180°): ");
            Serial.println(angRel1, 1);
            lastPrint1 = now;
        }
    }

    if (motor2Centrado)
    {
        uint16_t r2 = readAS5600_raw(Wire1);
        float ang2 = rawToDegrees(r2);
        angFiltrado2 = alpha * ang2 + (1.0 - alpha) * angFiltrado2;
        float angRel2 = angFiltrado2 - angOffset2;
        if (angRel2 > 180.0)
            angRel2 -= 360.0;
        if (angRel2 < -180.0)
            angRel2 += 360.0;
        static unsigned long lastPrint2 = 0;
        if (now - lastPrint2 > 200)
        {
            Serial.print("M2 (±180°): ");
            Serial.println(angRel2, 1);
            lastPrint2 = now;
        }
    }

    // aquí podríamos agregar lógica para "deshabilitar motores" después de X s si quieres
    // o aceptar comandos por Serial/USB para re-homing. Por ahora no para mantenerlo simple.
}

void cambiarEstadoBoton()
{
    estadoBoton = digitalRead(BOTON) == LOW;
}
