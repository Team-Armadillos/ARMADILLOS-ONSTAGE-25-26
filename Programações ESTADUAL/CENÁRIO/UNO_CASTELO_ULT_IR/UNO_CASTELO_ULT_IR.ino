#include <Servo.h>

#define posInicial 0
#define posFinal 90

const int ultraPin = 8;
const int servoPin = 9;
const int numAmostras = 15; // Número de leituras para média
const int pinBotao = 7;

Servo servo;

// Variáveis para controle de LOW por 500ms
unsigned long lowStartTime = 0;
bool lowStarted = false;
bool acaoEmExecucao = false;
unsigned long acaoStartTime = 0;
const unsigned long duracaoAcao = 3000; // 3 segundos

long readUltrasonic() {
  long duration;

  pinMode(ultraPin, OUTPUT);
  digitalWrite(ultraPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultraPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultraPin, LOW);

  pinMode(ultraPin, INPUT);
  duration = pulseIn(ultraPin, HIGH, 20000); // Timeout de 20ms

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
  pinMode(pinBotao, INPUT);
  servo.attach(servoPin);
  servo.write(posInicial);
  Serial.println("Aguardando para iniciar");
  delay(2000);
  servo.write(posFinal);
  Serial.println("Fim do Setup");
}

void loop() {
  // Se uma ação está em andamento, aguarda ela terminar (sem bloquear)
  if (acaoEmExecucao) {
    if (millis() - acaoStartTime >= duracaoAcao) {
      acaoEmExecucao = false;
    }
    return; // Evita continuar com o restante do loop durante a ação
  }

  // Leitura da média do sensor ultrassônico
  float media = readAverageSensor(numAmostras);
  Serial.print("Media da distancia: ");
  Serial.print(media);
  Serial.println(" cm");

  // Controle do servo baseado na distância
  if (media <= 70 && media >= 55) {
    servo.write(posInicial);
    delay(200);
  }

  // Verificação assíncrona do pino 7 em nível LOW por 500ms
  int estadoBotao = digitalRead(pinBotao);

  if (estadoBotao == LOW) {
    if (!lowStarted) {
      lowStartTime = millis();
      lowStarted = true;
    } else if (millis() - lowStartTime >= 30) {
      Serial.println("LEFT");
      acaoEmExecucao = true;
      acaoStartTime = millis();
      lowStarted = false; // Reseta para não disparar múltiplas vezes
    }
  } else {
    // Se o sinal voltar para HIGH, reinicia contagem
    lowStarted = false;
  }
}
