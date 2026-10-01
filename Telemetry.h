#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>

// ======================================================================
// TelemetryLink - Envío de telemetría y lectura de comandos por Serial
// ======================================================================
// PROTOCOLO (una línea de texto terminada en '\n' en cada sentido):
//
//   Arduino -> PC  (telemetría, ~20-30 Hz)
//     TEL,<acelerador%>,<freno%>,<volante%>
//     Ejemplo: TEL,50.0,0.0,-25.5
//
//   PC -> Arduino  (comandos, opcionales)
//     FFB,<valor>   entero -100..100 = potencia y dirección del motor.
//                   Hay que reenviarlo periódicamente: ver
//                   MOTOR_WATCHDOG_TIMEOUT_MS en config.h. Si deja de
//                   llegar, el motor se detiene solo.
//     STOP          corta el motor inmediatamente
//     CENTER        activa el modo de centrado de prueba (NO es Force
//                   Feedback real, ver README_FIRMWARE.md)
//     CAL           entra en modo calibración (imprime valores RAW)
//     RUN           vuelve a operación normal
//
// La lectura de Serial es NO bloqueante: se procesan los bytes que ya
// llegaron al buffer de hardware y se vuelve enseguida. Nunca se
// espera nada con delay() ni con un while bloqueante.
// ======================================================================

class TelemetryLink {
  public:
    void begin(long baudRate);

    // Envía "TEL,..." solo si ya pasó el intervalo configurado.
    void updateTelemetry(unsigned long currentMillis, float accelPct, float brakePct, float steeringPct);

    // Imprime una línea legible con valores RAW, para el modo CAL.
    void updateCalibrationPrint(unsigned long currentMillis, int accelRaw, int brakeRaw, int wheelRaw);

    // Devuelve true y llena 'commandOut' cuando terminó de llegar una
    // línea completa por Serial. Si todavía no llegó nada, no bloquea.
    bool pollCommand(String &commandOut);

  private:
    unsigned long _lastTelemetryMs = 0;
    unsigned long _lastCalPrintMs = 0;
    String _inputBuffer;
};

#endif
