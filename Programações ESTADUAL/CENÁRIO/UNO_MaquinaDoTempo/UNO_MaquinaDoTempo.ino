#include <IRremote.h>  // Use a versão 2.7.0 (como você informou)

#define TRIGGER_ECHO_PIN 8   // Pino único para Trigger e Echo (ver observação)
#define MAX_DISTANCE_CM  50
#define NUM_LEITURAS     15
#define dist_noRobot     40
#define dist_Robot       36

#define IR_LED_PIN       3   // Pino 3 para evitar conflito de Timer (usa Timer1)

float media;
int step = 1;

IRsend irsend;

void setup() {
  Serial.begin(9600);
  irsend.enableIROut(38); // Inicializa o emissor IR em 38kHz
  pinMode(TRIGGER_ECHO_PIN, OUTPUT);
  digitalWrite(TRIGGER_ECHO_PIN, LOW); // Garante estado inicial
  Serial.println("Iniciando leitura do sensor ultrassônico...");
}

float medirDistancia() {
  // Disparo do pulso ultrassônico
  pinMode(TRIGGER_ECHO_PIN, OUTPUT);
  digitalWrite(TRIGGER_ECHO_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_ECHO_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_ECHO_PIN, LOW);

  // Medição do tempo do eco
  pinMode(TRIGGER_ECHO_PIN, INPUT);
  long duracao = pulseIn(TRIGGER_ECHO_PIN, HIGH, MAX_DISTANCE_CM * 58 * 2); // timeout proporcional à distância

  // Converter tempo em distância (cm)
  float distancia = duracao / 58.0;

  if (distancia <= 0 || distancia > MAX_DISTANCE_CM) {
    return -1;  // leitura inválida
  }
  return distancia;
}

void loop() {
  long soma = 0;
  int leiturasValidas = 0;

  for (int i = 0; i < NUM_LEITURAS; i++) {
    delay(40);
    float leitura = medirDistancia();
    if (leitura > 0) {
      soma += leitura;
      leiturasValidas++;
    }
  }

  if (leiturasValidas > 0) {
    media = (float)soma / leiturasValidas;
    Serial.print("Distância média: ");
    Serial.print(media, 2);
    Serial.println(" cm");
  } else {
    Serial.println("Nenhuma leitura válida detectada.");
    return;
  }

  if (media <= dist_Robot && media > 0 && (step % 2) != 0) {
    step++;
    Serial.print("Robo entrou na maquina, Step number: ");
    Serial.println(step);

    // Envia o código IR NEC 0xF7C03F
    irsend.sendNEC(0xF7C03F, 32);
    Serial.println("Sinal IR enviado!");
  }

  if (media >= dist_noRobot && (step % 2) == 0) {
    step++;
    Serial.print("Robo saiu da maquina, Step number: ");
    Serial.println(step);

    // Envia o código IR NEC 0xF7C03F
    irsend.sendNEC(0xF7C03F, 32);
    Serial.println("Sinal IR enviado!");
  }

  delay(50);
}
