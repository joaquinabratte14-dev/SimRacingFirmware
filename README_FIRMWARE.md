# Firmware volante + pedales + motor — Arduino Leonardo

Referencia rápida del proyecto. El análisis completo (por qué se tomó
cada decisión) está en la conversación de Claude donde se generó esto.

## Qué hace este firmware

- Joystick HID (volante, acelerador, freno) reconocido por Windows.
- Lectura del AS5600 por I2C, con corrección de wrap-around y centro
  calibrable en software.
- Control del BTS7960 con parada segura, límites de PWM, rampa de
  potencia y watchdog.
- Telemetría por Serial a ~25 Hz + canal de comandos entrante.

## Qué NO hace

No implementa Force Feedback real estilo DirectInput/HID PID. El motor
solo hace lo que este firmware calcula localmente (modo de centrado de
prueba, que es un resorte simple, NO feedback del juego) o lo que un
comando explícito le manda un programa de PC por Serial (modo FFB
externo). Ver la sección "Camino hacia FFB real" más abajo.

## Tabla de pines

| Señal              | Pin Leonardo | Notas                              |
|--------------------|:------------:|-------------------------------------|
| Acelerador (pot.)  | A0           | Igual que tu sketch de prueba       |
| Freno (pot.)       | A1           | Igual que tu sketch de prueba       |
| AS5600 SDA         | D2 / SDA     | I2C nativo del Leonardo             |
| AS5600 SCL         | D3 / SCL     | I2C nativo del Leonardo             |
| BTS7960 RPWM       | D9           | PWM, sentido derecha                |
| BTS7960 LPWM       | D10          | PWM, sentido izquierda              |
| BTS7960 R_EN       | D7           | Habilitación medio puente derecho   |
| BTS7960 L_EN       | D8           | Habilitación medio puente izquierdo |
| BTS7960 R_IS, L_IS | sin conectar | Sensado de corriente, opcional      |

No se usan D0/D1 (libres) ni D4/D6/D11/A2-A5 (libres para futuros
botones, current-sensing, etc.).

## Librerías necesarias

1. **Wire** — incluida con el IDE, no requiere instalación.
2. **Joystick**, de Matthew Heironimus:
   - Arduino IDE → *Programa → Incluir Librería → Administrar Librerías*
   - Buscar "**Joystick**" → instalar la de **Matthew Heironimus**
   - Repo: https://github.com/MHeironimus/ArduinoJoystickLibrary

## Protocolo Serial (115200 baudios)

**Arduino → PC** (cada ~40 ms):
```
TEL,<acelerador%>,<freno%>,<volante%>
TEL,50.0,0.0,-25.5
```

**PC → Arduino** (opcional):
```
FFB,<valor>   entero -100..100, potencia y dirección del motor.
              Hay que reenviarlo seguido: si deja de llegar por más de
              MOTOR_WATCHDOG_TIMEOUT_MS, el motor se detiene solo.
STOP          corta el motor ya mismo
CENTER        activa el modo de centrado de prueba (no es FFB real)
CAL           modo calibración: imprime valores RAW en vez de TEL
RUN           vuelve a operación normal
```

## Calibración de pedales

1. Cargar el firmware, abrir el Monitor Serie a 115200 baudios.
2. Enviar `CAL`.
3. Mover cada pedal a fondo y soltarlo del todo, anotando los valores
   `accel=` y `freno=` que aparecen en cada extremo.
4. Cargar esos valores en `config.h`: `ACCEL_MIN`, `ACCEL_MAX`,
   `BRAKE_MIN`, `BRAKE_MAX`. Si un pedal responde al revés, poner su
   `_INVERTED` en `true`.
5. Enviar `RUN` y volver a subir el firmware con los valores nuevos.

## Calibración del centro del volante

1. Con el firmware cargado, enviar `CAL`.
2. Centrar el volante físicamente (mirando de frente, ruedas derechas).
3. Anotar el valor `volante_raw=`.
4. Cargar ese valor como `WHEEL_CENTER_RAW` en `config.h`.
5. Girar el volante hasta el tope físico de un lado y anotar cuánto se
   movió `volante_raw` respecto al centro: ese es tu `WHEEL_LOCK_RANGE_RAW`.
6. Enviar `RUN` y volver a subir el firmware.

## Seguridad del motor (batería de moto)

- `stopMotor()` corta PWM **y** deshabilita ambos `EN`: doble capa.
- El motor arranca siempre apagado (nunca a máxima potencia al encender).
- `PWM_MAX_LIMIT` es un techo absoluto de software, en todos los modos.
- Watchdog: sin comandos FFB frescos, el motor se para solo.
- Recomendación que el firmware NO puede reemplazar: un **fusible en
  línea** entre la batería y la entrada B+ del BTS7960, dimensionado a
  la corriente de arranque/estancamiento real de tu motor. Ninguna
  protección de software sustituye a un fusible físico.

## Camino hacia Force Feedback real (opcional, a futuro)

Force Feedback real (que el propio juego mande efectos de resorte,
amortiguación, fuerza constante, etc. vía DirectInput/HID PID) SÍ es
técnicamente posible en un chip 32u4 como el del Leonardo, pero con una
librería distinta y bastante más específica:
`ArduinoJoystickWithFFBLibrary` (basada en el proyecto `hoantv/VNWheel`),
que implementa el protocolo HID PID y expone `getForce()` con los
efectos que Windows le manda al dispositivo.

Es un proyecto aparte, más chico y con menos historial que la librería
Joystick estándar, que además pide calcular velocidad/aceleración del
volante a partir del AS5600 (derivadas de la posición, sensibles al
ruido). Tiene sentido evaluarlo recién cuando la base actual (pedales,
volante, motor básico, telemetría) esté probada y estable.
