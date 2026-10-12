
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 27
#define LED_COUNT 120

#define BLE_CHANNEL 4
#define TURN_ON_SIGNAL 3
#define TURN_OFF_SIGNAL 5

#define TRAIL_LENGTH 15
#define EFFECT_INTERVAL 24

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

volatile int pendingSignal = 0;
bool lightsOn = false;

unsigned long lastUpdate = 0;
int trailPosition = 0;


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

    if (signal == TURN_ON_SIGNAL || signal == TURN_OFF_SIGNAL)
      pendingSignal = signal;
  }
};


// Keep all LEDs golden with a brighter moving trail
void updateLights() {
  strip.fill(strip.Color(180, 92, 0));

  for (int i = 0; i < TRAIL_LENGTH; i++) {
    int index = (trailPosition + LED_COUNT - i) % LED_COUNT;

    int intensity = 255 - (i * 180 / TRAIL_LENGTH);

    uint8_t red = 180 + (75 * intensity / 255);
    uint8_t green = 92 + (108 * intensity / 255);
    uint8_t blue = 25 * intensity / 255;

    strip.setPixelColor(index, strip.Color(red, green, blue));
  }

  strip.show();
  trailPosition = (trailPosition + 1) % LED_COUNT;
}


// Turn off the entire LED strip
void turnOffLights() {
  strip.clear();
  strip.show();
  Serial.println("All 120 LEDs turned off.");
}


void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(255);
  turnOffLights();

  // Start scanning for BLE signals on channel 4
  NimBLEDevice::init("");

  NimBLEScan* scanner = NimBLEDevice::getScan();
  scanner->setScanCallbacks(new BLEReceiver(), false);
  scanner->setActiveScan(true);
  scanner->setInterval(100);
  scanner->setWindow(99);
  scanner->start(0);

  Serial.println("SMILE TRIP - PYRAMID");
  Serial.println("Waiting for BLE signals on channel 4...");
}


void loop() {
  int signal = pendingSignal;
  pendingSignal = 0;

  // Change lighting state only when needed
  if (signal == TURN_ON_SIGNAL && !lightsOn) {
    lightsOn = true;
    trailPosition = 0;

    Serial.println("Golden lighting activated.");
  }

  else if (signal == TURN_OFF_SIGNAL && lightsOn) {
    lightsOn = false;
    turnOffLights();
  }

  // Update the running effect without blocking BLE reception
  if (lightsOn && millis() - lastUpdate >= EFFECT_INTERVAL) {
    lastUpdate = millis();
    updateLights();
  }

  delay(2);
}
