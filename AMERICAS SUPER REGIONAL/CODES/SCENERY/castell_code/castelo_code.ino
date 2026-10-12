
#include <NimBLEDevice.h>
#include <ESP32Servo.h>

#define SERVO_PIN 4
#define BLE_CHANNEL 4
#define BLE_TRIGGER 6

Servo castleServo;
volatile bool collapseRequested = false;
bool castleCollapsed = false;

// Read Pybricks BLE broadcasts
class BLEReceiver : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* device) override {
    if (!device->haveManufacturerData()) return;

    std::string data = device->getManufacturerData();
    if (data.size() < 6) return;

    if ((uint8_t)data[0] == 0x97 &&
        (uint8_t)data[1] == 0x03 &&
        (uint8_t)data[2] == BLE_CHANNEL &&
        data[3] == 0x00 &&
        data[4] == 0x61 &&
        (int8_t)data[5] == BLE_TRIGGER)
      collapseRequested = true;
  }
};

void setup() {
  Serial.begin(115200);

  castleServo.setPeriodHertz(50);
  castleServo.attach(SERVO_PIN, 500, 2400);
  castleServo.write(0);

  NimBLEDevice::init("");
  NimBLEScan* scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new BLEReceiver(), false);
  scanner->setActiveScan(true);
  scanner->start(0);

  Serial.println("SMILE TRIP - CASTLE");
  Serial.println("Waiting for BLE signal 6 on channel 4...");
}

void loop() {
  if (collapseRequested && !castleCollapsed) {
    castleCollapsed = true;
    collapseRequested = false;

    Serial.println("Castle collapse triggered!");

    // Release the castle mechanism
    for (int pos = 0; pos <= 90; pos++) {
      castleServo.write(pos);
      delay(15);
    }
  }

  delay(10);
}
