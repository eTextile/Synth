// 16Bits, 44100, LSB_first

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioControlSGTL5000 sgtl5000;

AudioPlaySerialflashRaw  playFlashRaw1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playFlashRaw1, 0, i2s1, 0);
AudioConnection          patchCord2(playFlashRaw1, 0, i2s1, 1);

void setup() {
  Serial.begin(9600);
  AudioMemory(10);
  sgtl5000.enable();
  sgtl5000.volume(0.1);

  delay(2000);

  if (!SerialFlash.begin(6)) {
    error("Unable to access SPI Flash");
  }
}

void loop() {
  playFlashRaw1.play("A.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("B.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("C.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("D.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("E.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("F.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("G.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("H.RAW");
  while (playFlashRaw1.isPlaying());
  playFlashRaw1.play("I.RAW");
  while (playFlashRaw1.isPlaying());
}

void error(const char *message) {
  while (1) {
    Serial.println(message);
    delay(2500);
  }
}
