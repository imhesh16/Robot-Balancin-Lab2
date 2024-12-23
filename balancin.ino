/* Robot balancin
 * descripcion: adaptacion de un codigo de robot balancin con control bluethoo y opcion de seguidor de lineas con un unico sensor (condicion, establecer el robot al lado derecho de la linea negra)
*/
#include <PID_v1.h>
#include <LMotorController.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <SoftwareSerial.h>
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
#define MIN_ABS_SPEED 30
MPU6050 mpu;
//*************************************** Ajustes ***************************************
double MotorVelocidadIzq = 0.7; 
double MotorVelocidadDer = 0.7; 
double PuntoEquilibrio = 177; //174.4 (auentar -> inclinar hacia +x)
double inclinacion = 2;       //inclinacion para avanzar
SoftwareSerial Serial_2 (4, 3);
//-----------------Control de Motores ---------------------------------------------------
//motor A
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;
//motor B
const int IN3 = 10;
const int IN4 = 9;
const int ENB = 11;
//------------------Los Valors de PID cambian con cada diseño --------------------------
double Kp = 70;  //double Kp = 70; 
double Kd = 3;  //double Kd = 3;  
double Ki = 363;  //double Ki = 363; 
//-----------------Control sigue lineas --------------------------------------------------
const int umbral = 500;
const int sensorPin = A0; 
const int tiempoGiroDerecha = 200; // Tiempo de giro hacia la derecha (ms)
const int tiempoAvance = 300;      // Tiempo de avance recto (ms)
bool sigueLinea = false; //sigue inicia desactivado
unsigned long tiempoAnterior = 0;      // Almacena el último tiempo registrado

//***************************************************************************************
int estado = 48;         // inicia detenido
// MPU control/status vars
bool dmpReady = false; // set true if DMP init was successful
uint8_t mpuIntStatus; // holds actual interrupt status byte from MPU
uint8_t devStatus; // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize; // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount; // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q; // [w, x, y, z] quaternion container
VectorFloat gravity; // [x, y, z] gravity vector
float ypr[3]; // [yaw, pitch, roll] yaw/pitch/roll container and gravity vector

//PID                       
double originalSetpoint = PuntoEquilibrio;   //double originalSetpoint = 172.50;

double setpoint = originalSetpoint;
double movingAngleOffset = 0.1;
double input, output;

PID pid(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

double motorSpeedFactorLeft = MotorVelocidadIzq; //double motorSpeedFactorLeft = 0.6;
double motorSpeedFactorRight = MotorVelocidadDer; //double motorSpeedFactorRight = 0.5;


LMotorController motorController(ENA, IN1, IN2, ENB, IN3, IN4, motorSpeedFactorLeft, motorSpeedFactorRight);

volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
void dmpDataReady()
{
 mpuInterrupt = true;
}

void setup()
{
  Serial_2.begin(9600);    // inicia el puerto serial para comunicacion con el Bluetooth
  Serial.begin(9600);  
  pinMode(sensorPin, INPUT);
 // join I2C bus (I2Cdev library doesn't do this automatically)
 #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
 Wire.begin();
 TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
 #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
 Fastwire::setup(400, true);
 #endif

 mpu.initialize();

 devStatus = mpu.dmpInitialize();

 // supply your own gyro offsets here, scaled for min sensitivity
 mpu.setXGyroOffset(202);   //220 --- 762
 mpu.setYGyroOffset(76);    //76 --- -1079
 mpu.setZGyroOffset(-86);   //-85 --- 1179
 mpu.setZAccelOffset(8997); // 1788 factory default for my test chip

 // make sure it worked (returns 0 if so)
 if (devStatus == 0)
 {
 // turn on the DMP, now that it's ready
 mpu.setDMPEnabled(true);

 // enable Arduino interrupt detection
 attachInterrupt(0, dmpDataReady, RISING);
 mpuIntStatus = mpu.getIntStatus();

 // set our DMP Ready flag so the main loop() function knows it's okay to use it
 dmpReady = true;

 // get expected DMP packet size for later comparison
 packetSize = mpu.dmpGetFIFOPacketSize();
 
 //setup PID
 pid.SetMode(AUTOMATIC);
 pid.SetSampleTime(10);
 pid.SetOutputLimits(-255, 255); 
 }
 else
 {
 // ERROR!
 // 1 = initial memory load failed
 // 2 = DMP configuration updates failed
 // (if it's going to break, usually the code will be 1)
 Serial.print(F("DMP Initialization failed (code "));
 Serial.print(devStatus);
 Serial.println(F(")"));
 }
}

void loop()
{
 //********************************************** Control Bluetooh ************************ 
  if(Serial_2.available()>0){        // lee el bluetooth y almacena en estado
    estado = Serial_2.read();
    Serial.println(estado);
  }
  /* app blutu datos de entrada 
   * nada '48'
   * arriba '49'
   * abajo '51'
   * derecha '50'
   * izquierda '52'
   * A '97' -> reestablecer
   * B '98' -> sigue linea
   * C '99'
   * D '100' -> desactivar sigue linea
  */
  if(estado==49){           // Boton desplazar al Frente
    setpoint = PuntoEquilibrio + inclinacion;
    //Serial.println("avanzar");
  }
  else if(estado==97){         // Boton Parar
    setpoint = PuntoEquilibrio; 
  }
  else if(estado==97){
    //activacion sigue linea
    sigueLinea=true;
  }
  else if(estado==100){
    //desactivacion sigue linea
    sigueLinea=false;
  }
  else if(estado==51){          // Boton Reversa
    setpoint = PuntoEquilibrio - inclinacion; 
    //Serial.println("reverza");
  }
  else if(estado==48){
    setpoint = PuntoEquilibrio; 
    //Serial.println("quieto");
  }
  else if(estado==52){
    // Girar sobre el eje de manera positiva (anti horario)
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, output * motorSpeedFactorLeft);
    delay(300);
    motorController.stopMoving();
    //Serial.println("rotar +");
  }
  else if(estado==50){
    // Girar sobre el eje de manera negativa (horario)
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, output * motorSpeedFactorLeft);
    delay(300);
    motorController.stopMoving();
    //Serial.println("rotar -");
  }
 //********************************************** Fin Control Bluetooh ************************
  
 //******************************************** inicio codigo sigue linea ***************
 if (sigueLinea){
  //ambas ruedas giran (funciona solo si esta uy cerca de la linea)
  //puede funcionar con dependencia del radio si solo una rueda gira y la otra es quieta (mayor distancia a recorrer)
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= tiempoAvance) {
    tiempoAnterior = tiempoActual;
    if (buscarLinea()){
      motorController.turnRight(255*MotorVelocidadDer,false); 
      delay(tiempoGiroDerecha);
      motorController.stopMoving();
      setpoint = PuntoEquilibrio + inclinacion; //avanza hasta que busque si hay una linea o no 
    }
  }
 }
 //******************************** Fin sigue linea ***********************
 
 // if programming failed, don't try to do anything
 if (!dmpReady) return;

 // wait for MPU interrupt or extra packet(s) available
 while (!mpuInterrupt && fifoCount < packetSize)
 {
 //no mpu data - performing PID calculations and output to motors 
 pid.Compute();
 motorController.move(output, MIN_ABS_SPEED);

 }

 // reset interrupt flag and get INT_STATUS byte
 mpuInterrupt = false;
 mpuIntStatus = mpu.getIntStatus();

 // get current FIFO count
 fifoCount = mpu.getFIFOCount();

 // check for overflow (this should never happen unless our code is too inefficient)
 if ((mpuIntStatus & 0x10) || fifoCount == 1024)
 {
 // reset so we can continue cleanly
 mpu.resetFIFO();
 Serial.println(F("FIFO overflow!"));

 // otherwise, check for DMP data ready interrupt (this should happen frequently)
 }
 else if (mpuIntStatus & 0x02)
 {
 // wait for correct available data length, should be a VERY short wait
 while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

 // read a packet from FIFO
 mpu.getFIFOBytes(fifoBuffer, packetSize);
 
 // track FIFO count here in case there is > 1 packet available
 // (this lets us immediately read more without waiting for an interrupt)
 fifoCount -= packetSize;

 mpu.dmpGetQuaternion(&q, fifoBuffer);
 mpu.dmpGetGravity(&gravity, &q);
 mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
 input = ypr[1] * 180/M_PI + 180;
 }
}

 
bool buscarLinea(){
  // rota hasta encontrar una linea negra
  setpoint = PuntoEquilibrio; //rotar sin avanzar;
  motorController.turnLeft(255*MotorVelocidadIzq,false);
  int sensorValue = analogRead(sensorPin);
  if (sensorValue < umbral) { // Detecta la línea negra
    motorController.stopMoving();
    return true;
  }
  return false;
}
