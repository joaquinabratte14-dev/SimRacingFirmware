// ======================================================================
// SimRacingFirmware.ino
// Firmware de volante + pedales + motor para Arduino Leonardo
// ======================================================================
// QUÉ HACE:
//  - Expone un joystick HID (volante, acelerador, freno) para Windows.
//  - Lee el AS5600 por I2C con corrección de wrap-around y centro
//    calibrable (ver config.h).
//  - Controla un BTS7960 con parada segura, límites de PWM, rampa de
//    potencia (slew-rate) y watchdog.
//  - Envía telemetría por Serial a ~25 Hz y puede recibir comandos de
//    fuerza desde un programa en la PC.
//
// QUÉ NO HACE (léase antes de conectar el motor a la batería):
//  - NO implementa Force Feedback real estilo DirectInput/HID PID. El
//    motor solo hace lo que este firmware calcula localmente (modo
//    centrado de prueba) o lo que un comando explícito por Serial le
//    ordena (modo FFB externo, alimentado por un programa de PC que
//    vos todavía tenés que escribir). Ver la explicación completa
//    entregada junto con este proyecto para la diferencia entre esto
//    y Force Feedback real, y qué haría falta para el segundo caso.
//
// LIBRERÍAS NECESARIAS:
//  - "Joystick" de Matthew Heironimus (Arduino Library Manager)
//  - Wire (incluida con el IDE, no requiere instalación)
// ======================================================================

#include <Wire.h>
#include <Joystick.h>

#include "config.h"
#include "AS5600Encoder.h"
#include "Pedals.h"
#include "MotorControl.h"
#include "Telemetry.h"

// --- Joystick HID ---
// Orden de parámetros: report id, tipo, cant. botones, cant. hats,
// X, Y, Z, Rx, Ry, Rz, rudder, throttle, accelerator, brake, steering.
// Solo habilitamos accelerator/brake/steering: son los ejes pensados
// específicamente para un volante y la mayoría de los juegos de
// carrera los reconocen de forma directa.
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
                    0, 0,
                    false, false, false,
                    false, false, false,
                    false, false,
                    true, true, true);

// --- Objetos principales ---
AS5600Encoder steeringSensor;
PedalInput accelPedal(PIN_ACCEL, ACCEL_MIN, ACCEL_MAX, ACCEL_INVERTED, PEDAL_FILTER_ALPHA);
PedalInput brakePedal(PIN_BRAKE, BRAKE_MIN, BRAKE_MAX, BRAKE_INVERTED, PEDAL_FILTER_ALPHA);
MotorController motor(PIN_RPWM, PIN_LPWM, PIN_R_EN, PIN_L_EN, PWM_MAX_LIMIT, MOTOR_WATCHDOG_TIMEOUT_MS);
TelemetryLink telemetry;

bool calibrationMode = false;

void handleCommand(const String &cmd);

void setup() {
  Wire.begin();
  steeringSensor.begin();
  accelPedal.begin();
  brakePedal.begin();
  motor.begin();          // deja el motor apagado: estado inicial seguro
  telemetry.begin(SERIAL_BAUD);

  Joystick.begin(false);  // false = nosotros llamamos sendState() a mano, una vez por loop
  Joystick.setAcceleratorRange(0, 1000);
  Joystick.setBrakeRange(0, 1000);
  Joystick.setSteeringRange(-1000, 1000);
}

void loop() {
  unsigned long now = millis();

  // 1) Leer entradas (rápido: sin delay, sin bloqueos)
  float accelPct = accelPedal.readPercent();
  float brakePct = brakePedal.readPercent();
  float steeringPct = steeringSensor.getSteeringPercent(WHEEL_CENTER_RAW, WHEEL_LOCK_RANGE_RAW);

  // 2) Procesar comandos entrantes por Serial (no bloqueante)
  String cmd;
  if (telemetry.pollCommand(cmd)) {
    handleCommand(cmd);
  }

  // 3) Actualizar el motor SIEMPRE, en todos los modos (para que el
  //    watchdog y la rampa funcionen aunque no haya comandos nuevos)
  motor.update(steeringPct, now);

  // 4) Actualizar el joystick HID (un solo sendState por loop)
  Joystick.setAccelerator((int32_t)(accelPct * 10));
  Joystick.setBrake((int32_t)(brakePct * 10));
  Joystick.setSteering((int32_t)(steeringPct * 10));
  Joystick.sendState();

  // 5) Telemetría normal, o impresión legible si estamos en modo CAL
  if (calibrationMode) {
    telemetry.updateCalibrationPrint(now, accelPedal.readRaw(), brakePedal.readRaw(), steeringSensor.readRawAngle());
  } else {
    telemetry.updateTelemetry(now, accelPct, brakePct, steeringPct);
  }
}

void handleCommand(const String &cmd) {
  if (cmd.startsWith("FFB,")) {
    int value = cmd.substring(4).toInt();
    motor.setExternalForce(value);
  } else if (cmd == "STOP") {
    motor.stopMotor();
  } else if (cmd == "CENTER") {
    motor.enableCenteringTest();
  } else if (cmd == "CAL") {
    calibrationMode = true;
    motor.stopMotor();  // por seguridad: no queremos el motor activo mientras calibrás a mano
  } else if (cmd == "RUN") {
    calibrationMode = false;
  }
}
