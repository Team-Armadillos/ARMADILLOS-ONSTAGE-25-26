// =================== PINOS ===================
#define TRIG_PIN 13
#define ECHO_PIN 12

// Ponte H - uma única placa com 4 entradas
#define IN1 26  // Motor 1
#define IN2 25
#define IN3 27  // Motor 2
#define IN4 14

#define ENA 33  // PWM Motor 1
#define ENB 32  // PWM Motor 2

// =================== VARIÁVEIS ===================
#define DIST_THRESHOLD 10  // cm
#define MOTOR_RUN_TIME 3800 // 5 segundos

// =================== SETUP ===================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotors();
}

// =================== LOOP ===================
void loop() {
  long duration, distance;

  // Envia pulso ultrassônico
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Leitura do tempo de retorno
  duration = pulseIn(ECHO_PIN, HIGH);

  // Converte para cm
  distance = duration * 0.034 / 2;
  Serial.print("Distancia: ");
  Serial.println(distance);

  if (distance < DIST_THRESHOLD) {
    Serial.println("Objeto detectado! Acionando motores...");
    runMotors();
    delay(MOTOR_RUN_TIME);
    stopMotors();
  }

  delay(100); // Pequena pausa antes da próxima leitura
}

// =================== FUNÇÕES AUXILIARES ===================
void runMotors() {
  // Motor 1 frente
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  // Motor 2 frente
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 255); // velocidade máxima Motor 1
  analogWrite(ENB, 255); // velocidade máxima Motor 2
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
