#ifndef PEDALS_H
#define PEDALS_H

#include <Arduino.h>

// ======================================================================
// PedalInput - Lectura, filtrado y calibración de un pedal analógico
// ======================================================================

class PedalInput {
  public:
    PedalInput(uint8_t pin, int rawMin, int rawMax, bool inverted, float filterAlpha);

    void begin();

    // Lectura RAW sin filtrar (0-1023). Se usa solo en modo
    // calibración (comando "CAL"), para ver los extremos reales.
    int readRaw();

    // Lectura filtrada, calibrada e invertida si corresponde,
    // convertida a 0-100%.
    float readPercent();

  private:
    uint8_t _pin;
    int _min;
    int _max;
    bool _inverted;
    float _alpha;
    float _filtered;
    bool _firstReading;
};

#endif
