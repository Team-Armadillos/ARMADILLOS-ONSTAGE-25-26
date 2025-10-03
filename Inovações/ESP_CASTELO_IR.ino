#include "BluetoothSerial.h"

BluetoothSerial ESP32_BT; // Cria instância Bluetooth Serial

const int irPin = 13;      // Pino analógico do sensor IR
const int limite = 2000;   // Ajuste conforme o sensor
bool objetoDetectado = false;

void setup() {
  Serial.begin(115200);       // Serial USB (opcional)
  ESP32_BT.begin("ESP32_CASTELO"); // Nome do dispositivo Bluetooth
  pinMode(irPin, INPUT);
  Serial.println("ESP32_CASTELO pronto! Monitorando sensor IR...");
}

void loop() {
  int valor = analogRead(irPin);

  if (valor > limite && !objetoDetectado) {
    objetoDetectado = true; // Marca que detectou objeto
  } 
  else if (valor <= limite && objetoDetectado) {
    objetoDetectado = false; // Objeto saiu
    Serial.println("DETECTADO");       // Serial USB
    ESP32_BT.println("DETECTADO");     // Serial Bluetooth
  }

  delay(100);
}


