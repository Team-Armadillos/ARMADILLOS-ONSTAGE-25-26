#include <ESP32Servo.h>

// ==================== CONFIGURAÇÕES ====================
#define posInicial 90
#define posFinal 180

// Sensor 1 (TRIG/ECHO no mesmo pino)
#define ultra1Pin 32    

// Sensor 2 (TRIG e ECHO separados)
#define ultra2Trig 14
#define ultra2Echo 27

// Servo
#define servoPin 33        

// Lógica
#define numAmostras 10     
#define DIST_THRESHOLD1 50  // cm
#define DIST_THRESHOLD2   90 // cm
#define SERVO_TIME -800     // ms
#define TIMEOUT_SENSOR2 3000 // ms para autenticação dupla

Servo servo;

bool aguardandoSensor2 = false;
unsigned long inicioEspera = 0;

bool servoAtivo = false;
unsigned long servoInicio = 0;

// ==================== FUNÇÕES ====================
// Sensor 1 (TRIG/ECHO juntos)
long readUltrasonic1() {
  long duration;
  pinMode(ultra1Pin, OUTPUT);
  digitalWrite(ultra1Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultra1Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultra1Pin, LOW);

  pinMode(ultra1Pin, INPUT);
  duration = pulseIn(ultra1Pin, HIGH, 20000);
  return duration;
}

float readSensor1() {
  long duration = readUltrasonic1();
  return duration * 0.0343 / 2.0;
}

// Sensor 2 (TRIG/ECHO separados)
long readUltrasonic2() {
  digitalWrite(ultra2Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(ultra2Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultra2Trig, LOW);

  long duration = pulseIn(ultra2Echo, HIGH, 20000);
  return duration;
}

float readSensor2() {
  long duration = readUltrasonic2();
  return duration * 0.0343 / 2.0;
}

// Média de leituras
float readAverage(float (*sensorFunc)(), int samples) {
  float soma = 0;
  int validas = 0;
  for (int i = 0; i < samples; i++) {
    float leitura = sensorFunc();
    if (leitura > 0 && leitura < 400) {
      soma += leitura;
      validas++;
    }
    delay(10);
  }
  if (validas == 0) return 999; 
  return soma / validas;
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  servo.attach(servoPin);
  servo.write(posInicial);

  pinMode(ultra2Trig, OUTPUT);
  pinMode(ultra2Echo, INPUT);

  Serial.println("Sistema iniciado");
}

// ==================== LOOP ====================
void loop() {
  float media1 = readAverage(readSensor1, numAmostras);
  float media2 = readAverage(readSensor2, numAmostras);

  Serial.print("Sensor1: ");
  Serial.print(media1);
  Serial.print(" cm | Sensor2: ");
  Serial.print(media2);
  Serial.println(" cm");

  // Se não está aguardando o segundo, verifica sensor 1
  if (!aguardandoSensor2 && media1 < DIST_THRESHOLD2) {
    aguardandoSensor2 = true;
    inicioEspera = millis();
    Serial.println("Sensor 1 detectou! Aguardando Sensor 2...");
  }

  // Se está aguardando, verificar sensor 2
  if (aguardandoSensor2) {
    if (media2 < DIST_THRESHOLD1) {
      // Ambos detectaram → aciona servo
      servo.write(posFinal);
      servoAtivo = true;
      servoInicio = millis();
      aguardandoSensor2 = false;
      Serial.println("Sensor 2 confirmou! Servo acionado.");
    } else if (millis() - inicioEspera > TIMEOUT_SENSOR2) {
      // Passou tempo e não confirmou
      aguardandoSensor2 = false;
      Serial.println("Timeout! Reiniciando lógica...");
    }
  }

  // Retorna o servo após tempo
  if (servoAtivo && millis() - servoInicio >= SERVO_TIME) {
    servo.write(posInicial);
    servoAtivo = false;
    Serial.println("Servo retornou à posição inicial.");
  }
}
