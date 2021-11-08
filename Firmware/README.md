# eTextile-Synthesizer
## Exploring music through textile

## Arduino Firmware
    /Synth/Firmware/main/main.ino
 
### Requirements
- **eTextile-Synthesizer PCB** & **Teensy 4.0**
- **Arduino IDE** : Arduino 1.8.14 [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Arduino IDE additional board** : Teensyduino 1.54 [DOWNLOAD](https://www.pjrc.com/teensy/td_download.html)
- **Arduino IDE additional library**
  - **Included in Teensyduino**
    - **SPI**: https://github.com/PaulStoffregen/SPI
    - **ADC**: https://github.com/pedvide/ADC
    - **Encoder** : https://github.com/PaulStoffregen/Encoder 
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

### Powering the eTextile-Synthesizer
- The Teensy Micro USB Type B **will not power** the eTextile-Synthesizer
- You must use the included power cable with an **5Volts AC/DC power Plug** or your laptop USB plug

### Program Synopsis
The embedded Software is implementing **image analysis algorithms** on the E256 eTextile-matrix-sensor to design a new tactile interaction vocabulary.
- **Bilinear interpolation** The 16x16 Analog pressure sensor values are interpolated with a bilinear algorithm
- **Blob tracking** The interpolated pressure matrix sensor values are analyzed with a Connected Component Labelling algorithm
- **Blob ID management** each blob is tracked in space and time using single chained linked list
- **Blob shape and movement characterisation** Blobs coordinates, size and pressure are used to play music...

### eTextile-Synthesizer / Benchmark
  - ADC_INPUT : 2500 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ...
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 550 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Configuring the system
The Synth firmware is offering a **config.h** to select the needed fonctionnalites. 

### Use the eTextile-Synthesizer as COMPUTER_INTERFACE
 / HARDWARE_MIDI / USB_SLIP_OSC]
- [0:1] #define MIDI_USB : Transmit touch coordinates via MIDI_USB **DO NOT FORGET: Arduino>Touls>USB_Type>MIDI**
- [0:1] #define MIDI_HARDWARE : Transmit touch coordinates via MIDI_HARDWARE **DO NOT FORGET: Arduino>Touls>USB_Type>Serial**
- [0:1] #define USB_SLIP_OSC : Transmit touch coordinates via USB_SLIP_OSC **DO NOT FORGET: Arduino>Touls>USB_Type>Serial**

### Use the eTextile-Synthesizer as STANDALONE divice
- [0:1] #define SYNTH_PLAYER : Set the eTextile-Synthesizer as polyphonic synthesizer
- [0:1] #define GRANULAR_PLAYER : Set the eTextile-Synthesizer as
- [0:1] #define FLASH_PLAYER : Set the eTextile-Synthesizer as 

### Mapping functionality (IN PROGRESS)
- [0:1] #define MAPPING_LAYOUT
  - [0:1] #define GRID_LAYOUT_MIDI_IN
  - [0:1] #define GRID_LAYOUT_DEFAULT
  - [0:1] #define GRID_LAYOUT_HARMONIC
  - [0:1] #define GRID_LAYOUT_NOTES
  - [0:1] #define GRID_LAYOUT_FREQ

## FLASH_PLAYER - prerequisite

    cd /Synth-master/Firmware/extras/CopyFromSerial/
    **Arduino** : CopyFromSerial.ino 
    
    cd /Synth-master/Firmware/medias/
    **python** rawfile-uploader.py /dev/ttyACM0 RAW/*

## Copyright
Except as otherwise noted, all files in the eTextile-Synthesizer project folder

    Copyright (c) 20014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthesizer project folder.

## TODO
- Optimise interpolation method (it have been already updated with windowing method)
- **Gesture Recognizer** could be a nice feature that fit the eTextile-Synthesizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)
