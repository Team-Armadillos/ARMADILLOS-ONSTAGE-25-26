
#include <NimBLEDevice.h>
#include <IRremote.hpp>

#define MOTOR_IN1_PIN 27
#define MOTOR_IN2_PIN 26
#define MOTOR_ENABLE_PIN 25
#define IR_TRANSMITTER_PIN 14

#define BLE_CHANNEL 4
#define START_SIGNAL 9
#define STOP_SIGNAL 11

#define IR_ADDRESS 0x00
#define IR_COMMAND 0x45

volatile int pendingSignal = 0;
bool motorRunning = false;

// Read Pybricks BLE broadcasts
class BLEReceiver : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* device) override {
    if (!device->haveManufacturerData()) return;

    std::string data = device->getManufacturerData();
    if (data.size() < 6) return;

    if ((uint8_t)data[0] != 0x97 ||
        (uint8_t)data[1] != 0x03 ||
        (uint8_t)data[2] != BLE_CHANNEL ||
        data[3] != 0x00 ||
        data[4] != 0x61) return;

    int signal = (int8_t)data[5];

    if (signal == START_SIGNAL || signal == STOP_SIGNAL)
      pendingSignal = signal;
  }
};

// Control the stage motor
void setMotor(bool running) {
  digitalWrite(MOTOR_IN1_PIN, running ? HIGH : LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
  ledcWrite(MOTOR_ENABLE_PIN, running ? 77 : 0);

  motorRunning = running;
  Serial.println(running ? "Motor ON - 30%." : "Motor OFF.");
}

// Send the IR command to the stage lights
void transmitIR() {
  IrSender.sendNEC(IR_ADDRESS, IR_COMMAND, 0);
  Serial.println("IR command sent.");
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  ledcAttach(MOTOR_ENABLE_PIN, 1000, 8);

  setMotor(false);
  IrSender.begin(IR_TRANSMITTER_PIN);

  NimBLEDevice::init("");
  NimBLEScan* scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new BLEReceiver(), false);
  scanner->setActiveScan(true);
  scanner->start(0);

  Serial.println("SMILE TRIP - STAGE");
  Serial.println("Waiting for BLE signals on channel 4...");
}

void loop() {
  int signal = pendingSignal;
  pendingSignal = 0;

  // Ignore repeated commands while keeping the current state
  if (signal == START_SIGNAL && !motorRunning) {
    setMotor(true);
    transmitIR();
  }

  else if (signal == STOP_SIGNAL && motorRunning) {
    setMotor(false);
    transmitIR();
  }

  delay(10);
}
