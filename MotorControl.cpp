#include "MotorControl.h"
#include "config.h"

MotorController::MotorController(uint8_t rpwmPin, uint8_t lpwmPin, uint8_t rEnPin, uint8_t lEnPin,
                                  int pwmMaxLimit, unsigned long watchdogTimeoutMs)
  : _rpwm(rpwmPin), _lpwm(lpwmPin), _rEn(rEnPin), _lEn(lEnPin),
    _pwmMaxLimit(pwmMaxLimit), _watchdogTimeoutMs(watchdogTimeoutMs),
    _mode(MODE_STOPPED), _lastExternalCommandMs(0), _lastUpdateMs(0),
    _targetPercent(0), _appliedPercent(0.0f) {}

void MotorController::begin() {
  pinMode(_rpwm, OUTPUT);
  pinMode(_lpwm, OUTPUT);
  pinMode(_rEn, OUTPUT);
  pinMode(_lEn, OUTPUT);

  // Estado inicial: motor APAGADO. Nunca arranca con potencia, sin
  // importar qué haya en las variables globales al encender.
  stopMotor();
}

void MotorController::stopMotor() {
  analogWrite(_rpwm, 0);
  analogWrite(_lpwm, 0);
  // Deshabilitar los dos medios puentes a nivel de driver, no solo
  // poner el PWM en 0: es una segunda capa de seguridad independiente
  // de la primera.
  digitalWrite(_rEn, LOW);
  digitalWrite(_lEn, LOW);

  _mode = MODE_STOPPED;
  _targetPercent = 0;
  _appliedPercent = 0.0f;
}

void MotorController::enableCenteringTest() {
  if (_mode == MODE_STOPPED) {
    digitalWrite(_rEn, HIGH);
    digitalWrite(_lEn, HIGH);
  }
  _mode = MODE_CENTERING_TEST;
}

void MotorController::setExternalForce(int percent) {
  if (_mode == MODE_STOPPED) {
    digitalWrite(_rEn, HIGH);
    digitalWrite(_lEn, HIGH);
  }
  _mode = MODE_EXTERNAL_FFB;

  if (percent > 100) percent = 100;
  if (percent < -100) percent = -100;
  _targetPercent = percent;
  _lastExternalCommandMs = millis();
}

void MotorController::applyPower(int targetPercent, unsigned long currentMillis, int pwmCap) {
  // --- Rampa (slew-rate limiting): nunca saltar de golpe de potencia ---
  unsigned long dt = currentMillis - _lastUpdateMs;
  if (dt > 100) dt = 100;  // si el loop se frenó un instante, no lo compensamos de una sola vez

  float maxStep = MOTOR_SLEW_RATE_PERCENT_PER_SEC * (dt / 1000.0f);
  if (maxStep < 1.0f) maxStep = 1.0f;  // asegura progreso incluso en loops muy rápidos

  float target = (float)targetPercent;
  if (target > _appliedPercent + maxStep) {
    _appliedPercent += maxStep;
  } else if (target < _appliedPercent - maxStep) {
    _appliedPercent -= maxStep;
  } else {
    _appliedPercent = target;
  }

  if (_appliedPercent > 100.0f) _appliedPercent = 100.0f;
  if (_appliedPercent < -100.0f) _appliedPercent = -100.0f;

  // --- Límite de PWM (además del límite global aplicado por el llamador) ---
  int pwmValue = (int)(abs(_appliedPercent) / 100.0f * pwmCap);
  if (pwmValue > 255) pwmValue = 255;
  if (pwmValue < 0) pwmValue = 0;

  if (_appliedPercent > 0.0f) {
    analogWrite(_rpwm, pwmValue);
    analogWrite(_lpwm, 0);
  } else if (_appliedPercent < 0.0f) {
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, pwmValue);
  } else {
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, 0);
  }

  _lastUpdateMs = currentMillis;
}

void MotorController::update(float steeringPercent, unsigned long currentMillis) {
  switch (_mode) {

    case MODE_STOPPED:
      // No hacer nada: stopMotor() ya dejó todo en 0.
      _lastUpdateMs = currentMillis;
      break;

    case MODE_CENTERING_TEST: {
      float error = -steeringPercent;  // volante a la derecha -> empuja a la izquierda
      if (abs(error) < CENTERING_DEADZONE_PERCENT) error = 0.0f;
      int target = (int)(error * CENTERING_GAIN);
      applyPower(target, currentMillis, min(PWM_TEST_LIMIT, PWM_MAX_LIMIT));
      break;
    }

    case MODE_EXTERNAL_FFB:
      // Vigilancia: si la PC dejó de mandar comandos, parar por completo.
      if (currentMillis - _lastExternalCommandMs > _watchdogTimeoutMs) {
        stopMotor();
        break;
      }
      applyPower(_targetPercent, currentMillis, PWM_MAX_LIMIT);
      break;
  }
}
