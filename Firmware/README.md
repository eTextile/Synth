# eTextile-Synthetizer
### Exploring music through textile

## Requirements
- **eTextile-Synthetizer PCB** & **Teensy 4.0**
- **Arduino IDE** : Arduino 1.8.13 or higher [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Arduino IDE additional board** : Teensyduino 1.53 or higher [DOWNLOAD](https://www.pjrc.com/teensy/teensyduino.html)
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
- **Board** :     Teensy 4.0
- **USB Type** :  MIDI
- **CPUSpeed** :  600MHz
- **Optimize** :  Faster

### eTextile-Synthetizer / Firmware
- **Arduino** : /Synth-master/Firmware/main/main.ino
 
### eTextile-Synthetizer / Powering !
- The Teensy **Micro USB Type B** will not power the eTextile-Synthetizer
- You must connect the included power cable with a power adapter or your laptop USB plug

### eTextile-Synthetizer / Benchmark
  - ADC_INPUT : ... FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ... FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : ... FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : 420 FPS

## eTextile-Synthetizer / Program Synopsis
- The Arduio sketch implemant rows and columns scaning algorithm with synchronous **dual ADC sampling**.
  - COLS : Two analog multiplexers connected to the matrix columns
  - ROWS : Two 8_Bits shift register connected to the matrix rows
- The 16x16 Analog sensors values are interpolated into 64x64 with a bilinear algorithm
- The blob tracking algorithm (connected component labeling) is applyed onto the interpolated matrix
- Blobs are tracked with persistent ID (this is done with linked list implementation)
- Blobs coordinates, size and pressure are used to play music.

## eTextile-Synthetizer / Modes
  - **STANDALONE** : Blobs values are used to control embeded custom polyphonic synthesizers
  - **MIDI_USB** : Blobs values are translated into NOTES & CC transmitted via MIDI_USB to control Audio Applications software like Pd, Max...
  - **MIDI_HARDWARE** : Blobs values are translated into NOTES & CC transmitted via **miniJack TRS-A** to control external synthesizers

## eTextile-Synthetizer / Play Audio Files

    cd /Synth-master/Firmware/extras/CopyFromSerial/
    **Arduino** : CopyFromSerial.ino 
    
    cd /Synth-master/Firmware/medias/
    **python** rawfile-uploader.py /dev/ttyACM0 RAW/*

## Copyright
Except as otherwise noted, all files in the eTextile-Synthetizer project folder

    Copyright (c) 20014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthetizer project folder.

## TODO
- Optimise interpolation method (it have been already updated with windowing method)
- **Gesture Recognizer** could be a nice feature that fitt the eTextile-Synthetizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)
