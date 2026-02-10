// ========================================================================
//     🔸 A C C E L S T E P P E R  -  L I N E A R   M O V E M E N T 🔸
// ========================================================================
//  Archivo    : STEPPER.ino
//  Autor      : Klaus Michalsky
//  Fecha      : Feb-2026
//
//  DESCRIPCION
//  -----------------------------------------------------------------------
//  - Mueve un motor paso a paso lineal (~1cm) hacia ambos lados.
//  - Ejecuta movimiento en ambas direcciones
//    empezando en direccion opuesta al motor (~1cm)
//  - Ciclo continuo con pausa de 3 segundo entre movimientos.
//
//  NOTAS IMPORTANTES
//  -----------------------------------------------------------------------
//   - Para evitar que el motor linear choque con el soporte del eje
//   - Probar con pasos cortos
//
//  HARDWARE
//  -----------------------------------------------------------------------
//  MCU     : RP2040-Zero
//  Motor   : DC 5V/12V 15mm/20mm Schrittmotor 55mm Hub
//            (Aliexpress: Motor-house Store)
//  Driver  : TMC2209 A2->rojo, A1->negro, B1->amarillo, B2->azul,
//            MS1, MS2, -> GND
//
//  ESTADO
//  -----------------------------------------------------------------------
//  ✅ Funcional
// ========================================================================

#include <AccelStepper.h>

//  DEFINICION DE PINES
//  -----------------------------------------------------------------------
#define DIR 5
#define STEP 4
#define ENABLE 3

// OBJETOS
//  -----------------------------------------------------------------------
AccelStepper motor(AccelStepper::DRIVER, STEP, DIR);

// CONFIGURACION DE MOTORES
//  -----------------------------------------------------------------------
// 200 pasos (~1cm) con microstepping 1/8
// ENABLE LOW = driver activado
const int microstepping = 8;
const int pasos = 200 * microstepping;
const int direccion = -1;

// =======================================================================
// SETUP
// =======================================================================
void setup()
{
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);

  motor.setMaxSpeed(2000);
  motor.setAcceleration(1500);

  motor.setCurrentPosition(0);
}

// =======================================================================
// LOOP
// =======================================================================
void loop()
{
  // Mover 1cm en direccion opuesta al motor
  motor.moveTo(direccion * pasos);
  while (motor.distanceToGo() != 0)
  {
    motor.run();
  }
  delay(3000);

  // Volver a origen
  motor.moveTo(-direccion * pasos);
  while (motor.distanceToGo() != 0)
  {
    motor.run();
  }
  delay(3000);
}