#include <Servo.h>
#define posInicial 0
#define posFinal 90
const int ultraPin = 8;
const int servoPin = 9;
const int numAmostras = 15; // Número de leituras para média
Servo servo;

long readUltrasonic() {
  long duration;

  pinMode(ultraPin, OUTPUT);
  digitalWrite(ultraPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultraPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultraPin, LOW);

  pinMode(ultraPin, INPUT);
  duration = pulseIn(ultraPin, HIGH, 20000); // Timeout de 30ms

  return duration;

}
float readSensor() {
  long duration = readUltrasonic();
  float distance = duration * 0.0343 / 2;
  return distance;
}
float readAverageSensor(int samples) {
  float soma = 0;
  int validas = 0;
  for (int i = 0; i < samples; i++) {
    float leitura = readSensor();
    if (leitura > 2 && leitura < 400) { // Faixa típica do HC-SR04
      soma += leitura;
      validas++;
    }
    delay(10); // Pequeno intervalo entre leituras
  }
  if (validas == 0) return 0;
  return soma / validas;
}

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT);
  servo.attach(servoPin);
  servo.write(posFinal);
  delay(500);
  servo.write(posInicial);

  Serial.println("Aguardando para iniciar");
  delay(2000);
  servo.write(posFinal);
  Serial.println("Fim do Setup");
}

void loop() {
  float media = readAverageSensor(numAmostras);
  Serial.print("Media da distancia: ");
  Serial.print(media);
  Serial.println(" cm");
  //Serial.println(digitalRead(7));

  if(media <= 70 && media >= 55){
    servo.write(posInicial);
    delay(200);
  }

  /*if(digitalRead(digitalRead(7)) == 0){
    Serial.println("LEFT");
    delay(3000);
  }*/
}


