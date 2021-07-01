# eTextile-Synthesizer
## Exploring music through textile

## Arduino Firmware
    /Synth-master/Firmware/main/main.ino
 
### Requirements
- **eTextile-Synthesizer PCB** & **Teensy 4.0**
- **Arduino IDE** : Arduino 1.8.14 [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Arduino IDE additional board** : Teensyduino 1.54 [DOWNLOAD](https://www.pjrc.com/teensy/td_download.html)
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
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : ...
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Configuring the system
The Synth firmware is offering an 'config.h' to select the needed fonctionnalites. 
 
### Low level config 
- **BLOBS_POLAR_COORD** [0:1]
- **BLOBS_VELOCITY** [0:1]

### Mapping functionality (IN PROGRESS)
- **MAPPING_LAYAOUT** [0:1] 

### Transmit touch coordinates via [USB_MIDI / HARDWARE_MIDI / USB_SLIP_OSC]
- **USB_MIDI_TRANSMIT** [0:1] **DO NOT FORGET:** Arduino>Touls>USB_Type>MIDI
- **USB_SLIP_OSC_TRANSMIT** [0:1] **DO NOT FORGET:** Arduino>Touls>USB_Type>Serial
- **HARDWARE_MIDI_TRANSMIT** [0:1] **DO NOT FORGET:** Arduino>Touls>USB_Type>Serial

### Use the eTextile-Synthesizer as standalone polyphonic synthesizer
- **SYNTH_PLAYER** [0:1] Set the eTextile-Synthesizer as STANDALONE divice
- **GRANULAR_PLAYER** [0:1] Set the eTextile-Synthesizer as STANDALONE divice
- **FLASH_PLAYER** [0:1] Set the eTextile-Synthesizer as STANDALONE divice

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
- **Gesture Recognizer** could be a nice feature that fitt the eTextile-Synthesizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)
