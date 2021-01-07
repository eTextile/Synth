#include <SerialFlash.h>
#include <SPI.h>

#define CS                6
#define LED_PIN           5

void setup() {

  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  while(!Serial); // wait for Arduino Serial Monitor

  delay(100);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("All Files on SPI Flash chip:");

  if (!SerialFlash.begin(CS)) {
    error("Unable to access SPI Flash chip");
  }

  SerialFlash.opendir();
  while (1) {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print("  ");
      Serial.print(filename);
      spaces(20 - strlen(filename));
      Serial.print("  ");
      Serial.print(filesize);
      Serial.print(" bytes");
      Serial.println();
    } else {
      break; // no more files
    }
  }
  digitalWrite(LED_PIN, LOW);
}

void spaces(int num) {
  for (int i = 0; i < num; i++) {
    Serial.print(" ");
  }
}

void loop() {
}

void error(const char *message) {
  while (1) {
    Serial.println(message);
    delay(2500);
  }
}
