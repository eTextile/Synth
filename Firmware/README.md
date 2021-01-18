# eTextile-Synthetizer
### Exploring music through textile

## Requirements

- eTextile-Synthetizer PCB
- Teensy 4.0

- Arduino IDE 1.8.13 or higher
  - https://www.arduino.cc/en/Main/Software
- Arduino IDE additional board
  - Teensyduino(1.53): https://www.pjrc.com/teensy/teensyduino.html
- Arduino IDE additional library : **Include in Teensyduino**
  - **SPI**: https://github.com/PaulStoffregen/SPI
  - **ADC**: https://github.com/pedvide/ADC
  - **Encoder** : https://github.com/PaulStoffregen/Encoder 
  - **elapsedMillis** : https://github.com/pfeerick/elapsedMillis
  - **Audio** : https://github.com/PaulStoffregen/Audio
  - **MIDI** : https://github.com/PaulStoffregen/MIDI
- Arduino IDE additional library : **Install with Arduino library manager**
  - **Bounce2** : https://github.com/thomasfredericks/Bounce2
- Arduino IDE additional library : **Install by hand in /Documents/Arduino/library**
  - **OSC** : https://github.com/CNMAT/OSC

### Arduino IDE Settings for Teensy 4.0
- Board:        Teensy 4.0
- USB Type:     MIDI
- CPUSpeed:     600MHz
- Optimize      Faster

### eTextile-Synthetizer Firmware
- Load /Synth-master/Firmware/main/main.ino 

### eTextile-Synthetizer / Benchmark
  - ADC_INPUT : 1853 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : 987 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 950 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : 955 FPS

## eTextile-Synthetizer / Program Synopsis
- The Arduio sketch implemant rows and columns scaning algorithm with synchronous **dual ADC sampling**.
  - COLS = Two 8_Bits shift registers connected directly to the matrix columns.
  - ROWS = One 8_Bits shift register connected to two analog multiplexers that sens the matrix rows.
- The 16x16 Analog sensors values are interpolated into 64x64 with a bilinear algorithm.
- The blob tracking algorithm (connected component labeling) is applyed onto the interpolated matrix.
- Blobs are tracked with persistent ID (this is done with linked list implementation).
- Blobs coordinates, size and pressure are used to control a Polyphonic-Synthesizer.

## Copyright
Except as otherwise noted, all files in the resistiveMatrix project folder

    Copyright (c) 2005-2017 Maurin Donneaud and others.

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the resistiveMatrix project folder.

## Acknowledgements
Thanks to Vincent Roudaut, Hannah Perner Willson, Cedric Honnet, Antoine Meisso, Paul Strohmeier

## TODO
- Optimise interpolation method
  - Retrieval method from Microchip TB3064 white paper (p12)
  - microchip.com/stellent/groups/techpub_sg/documents/devicedoc/en550192.pdf
- $1-Gesture Recognizer
