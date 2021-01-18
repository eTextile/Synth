# eTextile-Synthetizer
### Exploring music through textile

## Requirements
- **eTextile-Synthetizer PCB** & **Teensy 4.0**
- **Arduino IDE** : Arduino 1.8.13 or higher [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Arduino IDE additional board** : Teensyduino (1.53 or higher) [DOWNLOAD](https://www.pjrc.com/teensy/teensyduino.html)
- **Arduino IDE additional library**
  - **Included in Teensyduino**
    - **SPI**: https://github.com/PaulStoffregen/SPI
    - **ADC**: https://github.com/pedvide/ADC
    - **Encoder** : https://github.com/PaulStoffregen/Encoder 
    - **elapsedMillis** : https://github.com/pfeerick/elapsedMillis
    - **Audio** : https://github.com/PaulStoffregen/Audio
    - **MIDI** : https://github.com/PaulStoffregen/MIDI
  - **To install with Arduino library manager**
    - **Bounce2** : https://github.com/thomasfredericks/Bounce2
  - **To install by hand in /Documents/Arduino/library**
    - **OSC** : https://github.com/CNMAT/OSC

### Arduino IDE Settings for Teensy 4.0
- Board:        Teensy 4.0
- USB Type:     MIDI
- CPUSpeed:     600MHz
- Optimize      Faster

### eTextile-Synthetizer / Firmware
- Load /Synth-master/Firmware/main/main.ino
 
### eTextile-Synthetizer / Powering
- The Teensy **Micro USB Type B** will not power the eTextile-Synthetizer
- You must connect the included power cable with a power adapter or your laptop USB plug

### eTextile-Synthetizer / Benchmark
  - ADC_INPUT : 1853 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : 987 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 950 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : 955 FPS

## eTextile-Synthetizer / Program Synopsis
- The Arduio sketch implemant rows and columns scaning algorithm with synchronous **dual ADC sampling**.
  - ROWS : Two 8_Bits shift register connected to the matrix rows
  - COLS : Two analog multiplexers connected to the matrix columns 
- The 16x16 Analog sensors values are interpolated into 64x64 with a bilinear algorithm
- The blob tracking algorithm (connected component labeling) is applyed onto the interpolated matrix
- Blobs are tracked with persistent ID (this is done with linked list implementation)
- Blobs coordinates, size and pressure are used to play music.

## eTextile-Synthetizer / Modes
  - **STANDALONE** : Blobs values are used to control embeded custom polyphonic synthesizers
  - **MIDI_USB** : Blobs values are transmitted via MIDI_USB to control Audio Applications software like Pd, Max...
  - **MIDI** : Blobs values are translated into MIDI-NOTES that are transmitted via MIDI OUT miniJack socket

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
