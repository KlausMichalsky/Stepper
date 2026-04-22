# AccelStepper
### Control de Motores stepper con la libreria AccelStepper
####  HARDWARE:
  - MCU        : RP2040-Zero / (opcion para Arduino-Nano cambiando pins)
  - Motor      : Nema17
  - Driver     : Step/Dir compatible con AccelStepper


# Homing
### Angular
### Rutina de homing para 1 "Nema17" usando AccelStepper y 1 "hall effect sensor KY-035" (Base para rotot SCARA)
####  ALGORITMO:
  - Limita la búsqueda a ±90° mecánicos durante el homing
  - Detecta flancos de entrada y salida del imán
  - Calcula el centro geométrico del imán
  - Define ese centro como posición 0 (referencia absoluta)   
  - Usa velocidades rápidas y lentas para optimizar tiempo y precisión
  - Implementa un timeout y manejo de errores

####  HARDWARE:
  - MCU        : RP2040-Zero / (opcion para Arduino-Nano cambiando pins)
  - Motor      : Nema17
  - Driver     : Step/Dir compatible con AccelStepper
  - Botón      : Inicio de homing (con debounce)
  - LED        : Estado del homing
  - Iman       : 5x5mm


### Linear
### Rutina de homing para un "55mm Linear Mini Stepper" usando AccelStepper y un "hall effect sensor KY-035" (Base para rotot SCARA)
####  ALGORITMO:
  - Limita la búsqueda a ±10mm mecánicos durante el homing
  - Detecta flancos de entrada (movimiento hacia arriba) y salida (movimiento hacia abajo) del imán
  - Define ese flanco como posición 0 (referencia absoluta)
  - Usa velocidades rápidas y lentas para optimizar tiempo y precisión
  - Implementa un timeout y manejo de errores

####  HARDWARE:
  - MCU        : RP2040-Zero / (opcion para Arduino-Nano cambiando pins)
  - Motor      : DC 5V/12V 15mm/20mm Schrittmotor 55mm Hub (Aliexpress: Motor-house Store)
  - Driver     : Step/Dir compatible con AccelStepper
  - Botón      : Inicio de homing (con debounce)
  - LED        : Estado del homing
  - Iman       : 5x5mm


# MultiStepper
Control de Motores stepper con la libreria AccelStepper y MultiStepper
####  HARDWARE:
  - MCU        : RP2040-Zero / (opcion para Arduino-Nano cambiando pins)
  - Motor      : Nema17
  - Driver     : Step/Dir compatible con AccelStepper
