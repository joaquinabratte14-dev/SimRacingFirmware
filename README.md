\# SimRacingFirmware



Firmware desarrollado para un simulador de conducción tipo F1 basado en \*\*Arduino Leonardo\*\*.



El sistema integra un volante con sensor magnético, pedales analógicos y un motor DC utilizado para generar movimiento en el sistema de dirección. Además, el Arduino funciona como dispositivo HID para que Windows pueda reconocer el volante y los pedales como un controlador.



\## Funciones principales



\* Lectura del volante mediante sensor magnético \*\*AS5600\*\*.

\* Lectura independiente de acelerador y freno mediante potenciómetros.

\* Reconocimiento como joystick HID mediante Arduino Leonardo.

\* Control de un motor DC mediante un controlador \*\*BTS7960\*\*.

\* Sistema de seguridad para el motor mediante límites de PWM y watchdog.

\* Comunicación Serial con una computadora para telemetría y comandos.

\* Calibración por software de pedales y volante.



\## Hardware utilizado



| Componente       | Función                                 |

| ---------------- | --------------------------------------- |

| Arduino Leonardo | Control principal y dispositivo USB HID |

| AS5600           | Medición de la posición del volante     |

| 2 potenciómetros | Acelerador y freno                      |

| BTS7960          | Control del motor DC                    |

| Motor DC 12 V    | Accionamiento del sistema de dirección  |

| Batería 12 V     | Alimentación del motor                  |



\## Conexiones principales



| Señal        | Pin Leonardo |

| ------------ | :----------: |

| Acelerador   |      A0      |

| Freno        |      A1      |

| AS5600 SDA   |   D2 / SDA   |

| AS5600 SCL   |   D3 / SCL   |

| BTS7960 RPWM |      D9      |

| BTS7960 LPWM |      D10     |

| BTS7960 R\_EN |      D7      |

| BTS7960 L\_EN |      D8      |



Los pines restantes quedan disponibles para futuras ampliaciones, como botones, sensores adicionales o medición de corriente.



\## Organización del firmware



El código está dividido en módulos para separar las distintas responsabilidades del sistema:



\* `SimRacingFirmware.ino` — programa principal.

\* `Pedals.cpp / Pedals.h` — lectura y procesamiento del acelerador y freno.

\* `AS5600Encoder.cpp / AS5600Encoder.h` — lectura y procesamiento del sensor AS5600.

\* `MotorControl.cpp / MotorControl.h` — control y seguridad del motor mediante el BTS7960.

\* `Telemetry.cpp / Telemetry.h` — comunicación Serial y procesamiento de comandos.

\* `config.h` — parámetros de configuración y calibración.



\## Comunicación Serial



La comunicación con la PC utiliza una velocidad de \*\*115200 baudios\*\*.



El Arduino envía periódicamente los valores de los controles mediante el formato:



```text

TEL,<acelerador%>,<freno%>,<volante%>

```



Ejemplo:



```text

TEL,50.0,0.0,-25.5

```



También puede recibir comandos desde una aplicación de PC para controlar determinadas funciones del firmware.



\## Seguridad del motor



El firmware incorpora diferentes mecanismos para evitar un funcionamiento accidental del motor:



\* El motor comienza apagado al iniciar el Arduino.

\* Existe un límite máximo de PWM configurado por software.

\* `stopMotor()` deshabilita el PWM y las señales de habilitación del BTS7960.

\* Un watchdog detiene automáticamente el motor si dejan de recibirse comandos de control durante el tiempo establecido.

\* El sistema debe utilizar además un \*\*fusible físico en línea con la alimentación del motor\*\*.



Las protecciones de software no sustituyen las protecciones eléctricas físicas.



\## Calibración



El firmware permite calibrar los valores de los pedales y la posición central del volante mediante comandos enviados por el Monitor Serie.



Los parámetros obtenidos durante la calibración se almacenan en `config.h`.



\## Librerías



El proyecto utiliza:



\* `Wire`, incluida con Arduino IDE.

\* `Arduino Joystick Library`, de Matthew Heironimus.



La librería Joystick permite que el Arduino Leonardo sea reconocido por Windows como un dispositivo HID.



\## Carga del firmware



1\. Abrir `SimRacingFirmware.ino` mediante Arduino IDE.

2\. Seleccionar \*\*Arduino Leonardo\*\* como placa.

3\. Instalar las librerías necesarias.

4\. Conectar el Arduino mediante USB.

5\. Compilar y cargar el firmware.

6\. Realizar la calibración de los pedales y del volante.

7\. Comprobar los datos mediante el Monitor Serie.



\## Documentación técnica



Para conocer con mayor detalle las conexiones, comandos, calibración, funcionamiento del motor y consideraciones de seguridad, consultar:



\*\*\[README\_FIRMWARE.md](README\_FIRMWARE.md)\*\*



\## Estado del proyecto



Este repositorio contiene el firmware utilizado durante el desarrollo del simulador de conducción presentado como proyecto académico.



El sistema se encuentra preparado para futuras ampliaciones, incluyendo mejoras en el control del motor y una posible implementación de Force Feedback mediante HID PID.



