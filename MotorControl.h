#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// ======================================================================
// MotorController - Manejo del BTS7960 y TODA la lógica de seguridad
// ======================================================================
// LEER ANTES DE USAR CON EL MOTOR CONECTADO A LA BATERÍA DE MOTO:
//
// Esta clase controla la POTENCIA del motor (velocidad y dirección).
// El BTS7960 es solamente un driver de potencia: no sabe nada de
// "Force Feedback" y no lo va a producir por sí solo. Esta clase
// ofrece tres modos:
//
//   MODE_STOPPED         Motor sin potencia. Estado inicial SIEMPRE.
//
//   MODE_CENTERING_TEST  Resorte proporcional simple hacia el centro,
//                        calculado LOCALMENTE con el ángulo del
//                        volante. Es una ayuda para las pruebas de
//                        banco (Prueba 6/7). NO es Force Feedback real
//                        de ningún juego: no usa ninguna información
//                        del juego, solo la posición del volante.
//
//   MODE_EXTERNAL_FFB    La potencia la decide un comando recibido por
//                        Serial (ver Telemetry.h). Esto es lo más
//                        cerca de "feedback" que este firmware ofrece
//                        por sí solo, y depende enteramente de que un
//                        programa en la PC calcule esa fuerza a partir
//                        de la telemetría del juego. Sigue sin ser
//                        Force Feedback real estilo DirectInput/HID
//                        PID (ver explicación entregada aparte).
//
// Si no llega un comando externo dentro de MOTOR_WATCHDOG_TIMEOUT_MS,
// el motor se DETIENE por completo. Nunca vuelve solo al modo de
// centrado: ante la duda, la opción segura es apagarse.
// ======================================================================

enum MotorMode {
  MODE_STOPPED,
  MODE_CENTERING_TEST,
  MODE_EXTERNAL_FFB
};

class MotorController {
  public:
    MotorController(uint8_t rpwmPin, uint8_t lpwmPin, uint8_t rEnPin, uint8_t lEnPin,
                     int pwmMaxLimit, unsigned long watchdogTimeoutMs);

    void begin();

    // Corta la potencia INMEDIATAMENTE: PWM a 0 Y ambos EN en LOW
    // (doble capa de seguridad, no depende solo del PWM).
    void stopMotor();

    // Activa el modo de centrado de prueba (Prueba 6/7, bring-up).
    void enableCenteringTest();

    // Recibe un comando de fuerza externo (-100..100). Cambia a
    // MODE_EXTERNAL_FFB y reinicia el watchdog. Hay que llamarlo
    // periódicamente desde la PC o el motor se detiene solo.
    void setExternalForce(int percent);

    // Llamar UNA VEZ POR LOOP, siempre, sin excepciones.
    //   steeringPercent: ángulo actual del volante (-100..100), se usa
    //                     solamente en modo centrado.
    //   currentMillis:    millis() del loop actual.
    void update(float steeringPercent, unsigned long currentMillis);

    MotorMode getMode() const { return _mode; }
    float getLastAppliedPercent() const { return _appliedPercent; }

  private:
    void applyPower(int targetPercent, unsigned long currentMillis, int pwmCap);

    uint8_t _rpwm, _lpwm, _rEn, _lEn;
    int _pwmMaxLimit;
    unsigned long _watchdogTimeoutMs;

    MotorMode _mode;
    unsigned long _lastExternalCommandMs;
    unsigned long _lastUpdateMs;

    int _targetPercent;     // lo que pide el modo activo
    float _appliedPercent;  // lo que realmente se aplicó, tras la rampa (slew-rate)
};

#endif
