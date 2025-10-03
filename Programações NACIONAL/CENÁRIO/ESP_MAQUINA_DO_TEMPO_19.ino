#include <NewPing.h>
#include "BluetoothSerial.h"

// ---------------- SENSOR ULTRASSONICO ----------------
#define TRIG_PIN 32       // Pino TRIG
#define ECHO_PIN 33       // Pino ECHO
#define MAX_DISTANCE 80   // Distância máxima de leitura em cm
#define NUM_LEITURAS 5    // Número de leituras para média

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// ---------------- BLUETOOTH ----------------
BluetoothSerial SerialBT;

// ---------------- VARIÁVEIS DE LÓGICA ----------------
float dist_Robot = 45;    // Limite para "ENTROU" (objeto próximo)
float dist_noRobot = 50;  // Limite para "SAIU" (objeto afastado)
bool objetoDetectado = false;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_MAQUINA");  
  Serial.println("ESP32 pronto! Monitorando sensor ultrassônico com média de leituras.");
}

// ---------------- FUNÇÃO: medir distância média ----------------
float medirDistanciaMedia() {
  long soma = 0;
  int leiturasValidas = 0;

  for (int i = 0; i < NUM_LEITURAS; i++) {
    delay(25); // intervalo menor entre leituras
    unsigned int leitura = sonar.ping_cm();
    if (leitura > 0) {  // leitura válida
      soma += leitura;
      leiturasValidas++;
    }
  }

  if (leiturasValidas > 0) {
    return (float)soma / leiturasValidas;  // retorna média
  } else {
    return -1; // nenhuma leitura válida
  }
}

// ---------------- LOOP ----------------
void loop() {
  float media = medirDistanciaMedia();

  if (media > 0) {
    Serial.print("Distância média: ");
    Serial.print(media, 2);
    Serial.println(" cm");

    // Lógica de detecção com histerese
    if (media <= dist_Robot && !objetoDetectado) {
      objetoDetectado = true;
      Serial.println("OBJETO ENTROU");
      SerialBT.println("OBJETO ENTROU");

      delay(2000); // <<< espera 2 segundos antes de verificar de novo
    }
    else if (media >= dist_noRobot && objetoDetectado) {
      objetoDetectado = false;
      Serial.println("OBJETO SAIU");
      SerialBT.println("OBJETO SAIU");
    }
  } else {
    Serial.println("Nenhuma leitura válida detectada.");
  }

  delay(100);  // pequeno intervalo antes da próxima média
}
