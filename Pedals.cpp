#include "Pedals.h"

PedalInput::PedalInput(uint8_t pin, int rawMin, int rawMax, bool inverted, float filterAlpha)
  : _pin(pin), _min(rawMin), _max(rawMax), _inverted(inverted), _alpha(filterAlpha),
    _filtered(0.0f), _firstReading(true) {}

void PedalInput::begin() {
  pinMode(_pin, INPUT);
}

int PedalInput::readRaw() {
  return analogRead(_pin);
}

float PedalInput::readPercent() {
  int raw = analogRead(_pin);

  // Filtro exponencial (EMA): suaviza el ruido típico de un
  // potenciómetro sin introducir demasiado retraso. En la primera
  // lectura no hay nada previo con qué promediar.
  if (_firstReading) {
    _filtered = (float)raw;
    _firstReading = false;
  } else {
    _filtered = _filtered + _alpha * ((float)raw - _filtered);
  }

  float percent;
  if (_max != _min) {
    percent = (_filtered - _min) / (float)(_max - _min) * 100.0f;
  } else {
    percent = 0.0f;  // evita división por cero si quedó mal calibrado (min == max)
  }

  if (_inverted) percent = 100.0f - percent;

  if (percent < 0.0f) percent = 0.0f;
  if (percent > 100.0f) percent = 100.0f;

  return percent;
}
