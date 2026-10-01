#ifndef CONFIG_H
#define CONFIG_H

// ======================================================================
// config.h - Constantes de configuración y calibración
// ======================================================================
// Todos los valores que probablemente necesites ajustar viven ACA.
// Si algo se siente mal calibrado, este es el único archivo que
// deberías necesitar tocar (después de correr el procedimiento de
// calibración, ver README_FIRMWARE.md).
// ======================================================================

// ---------------------------------------------------------------------
// PEDALES
// ---------------------------------------------------------------------
#define PIN_ACCEL A0   // Potenciómetro acelerador (mismo pin que tu sketch de prueba)
#define PIN_BRAKE A1   // Potenciómetro freno

// Valores RAW (0-1023) para "pedal suelto" y "pedal a fondo".
// PLACEHOLDERS - todavía no calibrados para tu hardware real.
// Procedimiento: subí el firmware, abrí el Monitor Serie a 115200,
// mandá "CAL", movés cada pedal a sus dos extremos y anotás los
// valores que aparecen. Después los pegás acá y volvés a subir.
const int ACCEL_MIN = 830;
const int ACCEL_MAX = 920;
const bool ACCEL_INVERTED = true;  // true si al pisar a fondo el valor RAW BAJA en vez de subir

const int BRAKE_MIN = 830;
const int BRAKE_MAX = 940;
const bool BRAKE_INVERTED = true;

// Filtro exponencial (0.0 - 1.0). Más alto = más responsivo pero más
// ruidoso. Más bajo = más suave pero con más retraso. 0.25-0.35 es un
// buen punto de partida para un potenciómetro común de pedal.
const float PEDAL_FILTER_ALPHA = 0.30f;

// ---------------------------------------------------------------------
// VOLANTE (AS5600)
// ---------------------------------------------------------------------
// El AS5600 usa el bus I2C nativo del Leonardo. No se define pin acá:
// Wire.h ya sabe cuáles son (ver tabla de pines en la explicación).

// Valor RAW (0-4095) del AS5600 cuando el volante está FÍSICAMENTE
// centrado. NO asumas que es 2048: depende de cómo haya quedado
// montado el imán sobre el eje. Se obtiene con el comando "CAL".
const int WHEEL_CENTER_RAW = 2048;  // PLACEHOLDER - calibrar antes de usar

// Cuántas unidades RAW representan el 100% de giro HACIA UN LADO.
// El AS5600 entrega 4096 unidades por vuelta completa (360°), o sea
// ~11.38 unidades por grado. Ejemplo: si tu montaje deja girar el
// volante 90° para cada lado desde el centro, son 90 * 11.38 ≈ 1024
// unidades RAW. Ajustalo a lo que tu montaje realmente permite.
// Ver la limitación de una sola vuelta en la explicación entregada.
const int WHEEL_LOCK_RANGE_RAW = 1024;  // PLACEHOLDER

// ---------------------------------------------------------------------
// BTS7960 (MOTOR)
// ---------------------------------------------------------------------
#define PIN_RPWM 9   // PWM, sentido "derecha"
#define PIN_LPWM 10  // PWM, sentido "izquierda"
#define PIN_R_EN 7   // Habilitación del medio puente derecho
#define PIN_L_EN 8   // Habilitación del medio puente izquierdo

// Límite ABSOLUTO de PWM (0-255) permitido en CUALQUIER modo. Esta es
// la última línea de defensa de software. Empezá bajo y subí de a poco
// solamente después de validar que todo lo demás funciona bien.
const int PWM_MAX_LIMIT = 90;

// Límite todavía más bajo, exclusivo del modo de centrado de prueba
// (pensado para la Prueba 6: "motor con PWM extremadamente bajo").
const int PWM_TEST_LIMIT = 35;

// Si no llega un comando FFB válido por Serial dentro de esta ventana,
// el motor se detiene solo. Esto es lo que te protege si el programa
// de la PC se cuelga, se cierra, o se desconecta el cable USB.
const unsigned long MOTOR_WATCHDOG_TIMEOUT_MS = 500;

// Parámetros del modo de centrado de prueba. OJO: esto NO es Force
// Feedback real, es solo un resorte proporcional simple pensado para
// las pruebas de banco. Ver la explicación entregada junto al firmware.
const float CENTERING_GAIN = 0.6f;             // ganancia proporcional
const float CENTERING_DEADZONE_PERCENT = 3.0f; // zona muerta cerca del centro

// Máxima variación de potencia permitida por segundo (0-100 en escala
// de porcentaje). Evita saltos bruscos: cuida la mecánica y es más
// seguro con un motor de este porte.
const float MOTOR_SLEW_RATE_PERCENT_PER_SEC = 200.0f;

// ---------------------------------------------------------------------
// TELEMETRÍA / SERIAL
// ---------------------------------------------------------------------
const unsigned long TELEMETRY_INTERVAL_MS = 40;            // ~25 Hz
const unsigned long CALIBRATION_PRINT_INTERVAL_MS = 150;   // ritmo legible en modo CAL

// En el Leonardo, Serial es USB-CDC nativo: este número NO define la
// velocidad real del enlace (el USB ya es mucho más rápido que
// cualquier baudrate clásico), pero hay que declararlo igual por
// compatibilidad de la API. Usá el mismo valor en tu programa de PC.
const long SERIAL_BAUD = 115200;

#endif
