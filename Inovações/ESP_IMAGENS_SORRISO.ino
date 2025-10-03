#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

const int BUF_SIZE = 1024;
uint8_t buffer[BUF_SIZE];

void setup() {
  Serial.begin(115200);       // Serial USB
  SerialBT.begin("ESP32_SMILE"); // Bluetooth SPP
}

void loop() {
  // Recebe dados da USB e envia via Bluetooth
  while (Serial.available() > 0) {
    int len = Serial.readBytes(buffer, BUF_SIZE);
    SerialBT.write(buffer, len);
  }
  delay(10); // Pequeno delay para evitar overflow
}
