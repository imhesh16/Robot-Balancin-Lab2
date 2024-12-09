# Robot-Balancin-Lab2
Repositorio para el desarrollo y documentación del proyecto Robot Balanceador Lab 2.
# Robot Balancín con Bluetooth y Seguimiento de Línea

Este repositorio contiene el código y las librerías necesarias para implementar un robot balancín controlado mediante Bluetooth con capacidad de seguimiento de líneas. El proyecto utiliza un sistema de control PID para garantizar el equilibrio dinámico del robot y módulos adicionales para funcionalidades avanzadas.

## Descripción del Proyecto

El robot balancín es un dispositivo autoequilibrado que utiliza un sensor inercial MPU6050 para medir la inclinación y ajustar los motores en tiempo real. Además, se puede controlar mediante una aplicación Bluetooth y tiene la capacidad de seguir líneas utilizando un único sensor infrarrojo.

### Características principales:
- Control de equilibrio mediante PID.
- Control remoto a través de Bluetooth (HC-06).
- Seguimiento de líneas negras con un sensor TCRT5000.
- Movimientos básicos: avance, retroceso y giro.
- Funcionalidades adicionales controladas desde una aplicación móvil.

## Componentes Utilizados

### Electrónicos
- **Arduino Uno**: Microcontrolador principal.
- **Sensor MPU6050**: Sensor inercial para medición de inclinación.
- **Módulo Bluetooth HC-06**: Comunicación inalámbrica.
- **Motores DC**: Para propulsión del robot.
- **Puente H (L298N)**: Control de motores.
- **Sensor infrarrojo TCRT5000**: Seguimiento de línea.
- **Encoders ópticos**: Medición de rotación de ruedas.
- **Baterías 18650**: Fuente de alimentación recargable.

### Librerías utilizadas
- [`I2Cdev`](https://github.com/jrowberg/i2cdevlib): Comunicación I2C con el MPU6050.
- [`LMotorController`](https://github.com/): Control de motores DC.
- [`MPU6050_6Axis_MotionApps20`](https://github.com/): Acceso a las funciones DMP del MPU6050.
- [`PID_v1`](https://github.com/br3ttb/Arduino-PID-Library): Implementación del controlador PID.

