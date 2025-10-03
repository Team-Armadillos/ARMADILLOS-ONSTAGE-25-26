#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// --------- Bluetooth ---------
BluetoothSerial SerialBT;

// --------- Servo ---------
Servo meuServo;
int pinoServo = 27; // Pino do servo

// --------- Posições ---------
#define posInicial 0  // posição inicial do servo
#define posFinal 50    // posição final do servo
#define tempoFinal 2000 // tempo em ms que o servo fica na posição final

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP_SMILE");  // Nome do dispositivo Bluetooth
  Serial.println("ESP_SMILE pronto para parear...");

  meuServo.attach(pinoServo);
  meuServo.write(posInicial); // vai para posição inicial
}

void loop() {
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    msg.trim();

    if (msg == "RECONHECIDO") {
      Serial.println("[ESP] Sorriso reconhecido, acionando servo!");

      // Vai para posição final
      meuServo.write(posFinal);
      delay(tempoFinal);

      // Retorna para posição inicial
      meuServo.write(posInicial);
    }
  }
}
