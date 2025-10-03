#include <IRremote.h>

// ---------------- PINOS ----------------
#define TRIG1 7
#define ECHO1 6

#define IN1 10
#define IN2 11
#define ENA 5

#define IR_LED1 3   // Emissor IR 1
#define IR_LED2 9   // Emissor IR 2

// ---------------- VARIÁVEIS ----------------
bool motorLigado = false;
unsigned long motorStartTime = 0;
bool esperaNovaVerificacao = false;
unsigned long tempoEspera = 0;

bool emPausa = false;
unsigned long pausaInicio = 0;

// ---------------- NEC CONFIG ----------------
const uint8_t address = 0x00;
const uint8_t command = 0x45;

// Criamos um segundo objeto IR para o emissor 2
IRsend IrSender2(IR_LED2);

// ---------------- FUNÇÃO MEDIR ----------------
long medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duracao = pulseIn(echo, HIGH, 30000); // timeout 30ms
  if (duracao == 0) return -1;
  long distancia = duracao * 0.034 / 2;
  if (distancia > 50) return -1;
  return distancia;
}

// ---------------- FUNÇÃO MÉDIA ----------------
long mediaDistancia(int trig, int echo, int leituras = 30) {
  long soma = 0;
  int cont = 0;
  for (int i = 0; i < leituras; i++) {
    long d = medirDistancia(trig, echo);
    if (d > 0) {
      soma += d;
      cont++;
    }
    delay(33); // ~30 leituras/segundo
  }
  if (cont == 0) return -1;
  return soma / cont;
}

// ---------------- FUNÇÃO PARA ENVIAR IR ----------------
void enviarIR() {
  // Emissor 1 (pino 3)
  IrSender.sendNEC(address, command, 0);
  // Emissor 2 (pino 8)
  IrSender2.sendNEC(address, command, 0);

  Serial.println("Sinal IR enviado nos dois emissores!");
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  // Inicializa os emissores
  IrSender.begin(IR_LED1);
  IrSender2.begin(IR_LED2);

  Serial.println("Sistema iniciado!");
}

// ---------------- LOOP ----------------
void loop() {
  // ---- PAUSA GLOBAL ----
  if (emPausa) {
    if (millis() - pausaInicio >= 10000) {
      emPausa = false;
      Serial.println("Pausa de 10s terminou. Reiniciando lógica...");
    } else {
      return; // espera sem executar a lógica
    }
  }

  long d1 = mediaDistancia(TRIG1, ECHO1, 30); // média de 30 leituras (~1s)

  if (d1 > 0) {
    Serial.print("Sensor (média): "); Serial.print(d1); Serial.println(" cm");
  }

  // ====== LIGA MOTOR (objeto <30cm) ======
  if (!motorLigado && !esperaNovaVerificacao && d1 > 0 && d1 < 30) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 200);
    motorStartTime = millis();
    motorLigado = true;

    Serial.println("Motor ligado e sinal IR enviado!");
    enviarIR();

    esperaNovaVerificacao = true;
    tempoEspera = millis();
  }

  // ====== ESPERA 10s ======
  if (esperaNovaVerificacao && millis() - tempoEspera < 10000) {
    // apenas espera
  }
  // ====== CHECAGEM APÓS 10s ======
  else if (esperaNovaVerificacao && d1 > 0 && d1 < 30 && motorLigado) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    motorLigado = false;

    Serial.println("Sensor detectou objeto após 10s: Motor desligado e sinal IR enviado!");
    enviarIR();

    esperaNovaVerificacao = false;

    // Ativa pausa global
    emPausa = true;
    pausaInicio = millis();
  }

  // ====== DESLIGA MOTOR APÓS 60s da ativação ======
  if (motorLigado && millis() - motorStartTime >= 60000) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    motorLigado = false;
    Serial.println("Motor desligado após 60 segundos.");
    enviarIR();

    // Ativa pausa global
    emPausa = true;
    pausaInicio = millis();
  }
}
