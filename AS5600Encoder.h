#ifndef AS5600_ENCODER_H
#define AS5600_ENCODER_H

#include <Arduino.h>

// ======================================================================
// AS5600Encoder - Lectura del encoder magnético del volante
// ======================================================================
// Lee el registro RAW ANGLE (0x0C/0x0D) del AS5600 por I2C. A propósito
// NO se usan los registros ZPOS/MPOS/MANG ni el comando BURN de este
// chip: esos graban de forma PERMANENTE en la memoria OTP interna (una
// cantidad limitada de veces, y no se puede deshacer). Toda la
// calibración de este proyecto se hace en software del lado del
// Arduino, así se puede recalibrar todas las veces que haga falta sin
// ningún riesgo para el sensor.
//
// LIMITACIÓN IMPORTANTE (hardware, no de este código): el AS5600 es un
// sensor ABSOLUTO DE UNA SOLA VUELTA (0-4095 en 0-360°). La corrección
// de wrap-around de getSteeringPercent() es válida SIEMPRE QUE el
// recorrido mecánico total del volante (tope a tope) sea menor a 360°.
// Si tu montaje permite girar más de una vuelta completa, este sensor
// por sí solo NO puede darte una posición absoluta confiable: hace
// falta reducción mecánica (que el imán gire menos que el volante) o
// un sensor multivuelta distinto. Ver la explicación completa
// entregada junto con este firmware.
// ======================================================================

class AS5600Encoder {
  public:
    void begin();

    // Lee el registro RAW ANGLE (0-4095) directamente del sensor.
    int readRawAngle();

    // Aplica el offset de centro y la corrección de wrap-around, y
    // escala el resultado a -100..100.
    //   centerRaw:    valor RAW correspondiente al centro mecánico
    //   lockRangeRaw: unidades RAW que representan el 100% de giro
    float getSteeringPercent(int centerRaw, int lockRangeRaw);

    // Diagnóstico, registro STATUS (0x0B) del AS5600.
    bool isMagnetDetected();   // bit MD
    bool isMagnetTooWeak();    // bit ML
    bool isMagnetTooStrong();  // bit MH

  private:
    uint8_t readRegister(uint8_t reg);
    int _lastGoodRaw = 2048;  // valor de respaldo si una lectura I2C puntual falla
};

#endif
