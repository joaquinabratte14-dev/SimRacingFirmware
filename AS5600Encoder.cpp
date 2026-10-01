#include "AS5600Encoder.h"
#include <Wire.h>

// Dirección I2C fija del AS5600 (hoja de datos AMS-OSRAM, confirmada)
static const uint8_t AS5600_ADDR = 0x36;

// Registros (hoja de datos AMS-OSRAM AS5600)
static const uint8_t REG_RAW_ANGLE_H = 0x0C;  // bits 11:8 (0x0D, bits 7:0, se lee a continuación por auto-incremento)
static const uint8_t REG_STATUS      = 0x0B;

// Bits del registro STATUS
static const uint8_t STATUS_MD = 0x20;  // Magnet Detected
static const uint8_t STATUS_ML = 0x10;  // Magnet too weak (Low)
static const uint8_t STATUS_MH = 0x08;  // Magnet too strong (High)

void AS5600Encoder::begin() {
  // El AS5600 soporta I2C fast-mode (400 kHz). Usarlo ayuda a mantener
  // el loop principal rápido y el joystick respondiendo con fluidez.
  Wire.setClock(400000);
}

uint8_t AS5600Encoder::readRegister(uint8_t reg) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  // false = "repeated start": no soltamos el bus entre el write y el
  // read que sigue, tal como recomienda la hoja de datos.
  uint8_t err = Wire.endTransmission(false);
  if (err != 0) {
    return 0;  // fallo de comunicación I2C; el llamador puede validar con isMagnetDetected()
  }
  Wire.requestFrom((int)AS5600_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

int AS5600Encoder::readRawAngle() {
  // Leemos los dos bytes (0x0C y 0x0D) en UNA sola transacción,
  // aprovechando que el puntero de dirección del AS5600 se autoincrementa
  // (según su hoja de datos). Esto evita el caso raro en que el ángulo
  // cambie justo entre dos lecturas separadas y arme un valor combinado
  // incoherente ("tearing" de lectura).
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(REG_RAW_ANGLE_H);
  uint8_t err = Wire.endTransmission(false);  // repeated start
  if (err != 0) {
    return _lastGoodRaw;  // fallo de comunicación I2C puntual: mantenemos el último valor válido
  }

  Wire.requestFrom((int)AS5600_ADDR, 2);
  if (Wire.available() < 2) {
    return _lastGoodRaw;
  }

  uint8_t hi = Wire.read();
  uint8_t lo = Wire.read();
  int raw = ((int)(hi & 0x0F) << 8) | lo;  // 12 bits válidos: 0-4095
  _lastGoodRaw = raw;
  return raw;
}

float AS5600Encoder::getSteeringPercent(int centerRaw, int lockRangeRaw) {
  int raw = readRawAngle();

  int32_t diff = (int32_t)raw - (int32_t)centerRaw;

  // Corrección de wrap-around 4095 -> 0. Matemáticamente válida SOLO
  // si el recorrido total del volante es menor a 360° (ver nota en el
  // .h). Si diff cae fuera de -2048..2048, lo "envolvemos" al lado
  // corto en vez de tomar el salto largo.
  if (diff > 2048) diff -= 4096;
  if (diff < -2048) diff += 4096;

  if (lockRangeRaw == 0) return 0.0f;  // evita división por cero si está mal configurado

  float percent = ((float)diff / (float)lockRangeRaw) * 100.0f;
  if (percent > 100.0f) percent = 100.0f;
  if (percent < -100.0f) percent = -100.0f;
  return percent;
}

bool AS5600Encoder::isMagnetDetected() {
  return (readRegister(REG_STATUS) & STATUS_MD) != 0;
}

bool AS5600Encoder::isMagnetTooWeak() {
  return (readRegister(REG_STATUS) & STATUS_ML) != 0;
}

bool AS5600Encoder::isMagnetTooStrong() {
  return (readRegister(REG_STATUS) & STATUS_MH) != 0;
}
