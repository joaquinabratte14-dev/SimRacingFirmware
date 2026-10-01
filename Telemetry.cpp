#include "Telemetry.h"
#include "config.h"

void TelemetryLink::begin(long baudRate) {
  Serial.begin(baudRate);
  _inputBuffer.reserve(32);
}

void TelemetryLink::updateTelemetry(unsigned long currentMillis, float accelPct, float brakePct, float steeringPct) {
  if (currentMillis - _lastTelemetryMs < TELEMETRY_INTERVAL_MS) return;
  _lastTelemetryMs = currentMillis;

  Serial.print("TEL,");
  Serial.print(accelPct, 1);
  Serial.print(",");
  Serial.print(brakePct, 1);
  Serial.print(",");
  Serial.println(steeringPct, 1);
}

void TelemetryLink::updateCalibrationPrint(unsigned long currentMillis, int accelRaw, int brakeRaw, int wheelRaw) {
  if (currentMillis - _lastCalPrintMs < CALIBRATION_PRINT_INTERVAL_MS) return;
  _lastCalPrintMs = currentMillis;

  Serial.print("CAL,accel=");
  Serial.print(accelRaw);
  Serial.print(",freno=");
  Serial.print(brakeRaw);
  Serial.print(",volante_raw=");
  Serial.println(wheelRaw);
}

bool TelemetryLink::pollCommand(String &commandOut) {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (_inputBuffer.length() > 0) {
        commandOut = _inputBuffer;
        _inputBuffer = "";
        return true;
      }
      // línea vacía (típico el '\r' antes del '\n'): seguimos leyendo
    } else {
      _inputBuffer += c;
      if (_inputBuffer.length() > 40) {
        // corta un buffer corrupto o gigante en vez de crecer sin límite
        _inputBuffer = "";
      }
    }
  }
  return false;
}
