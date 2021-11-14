# eTextile-Synthesizer
## Exploring music through textile

### Requirements
- **eTextile-Synthesizer PCB** & **Teensy 4.0**
- **Arduino IDE**: Arduino 1.8.16 [DOWNLOAD](https://www.arduino.cc/en/Main/Software)
- **Visual Studio Code**: [DOWNLOAD](https://visualstudio.microsoft.com/fr/)
- **platform.io extension**: [DOWNLOAD](https://platform.io/)
- **platform.io additional board**: Teensy 4.0

### External Libraries
All Library dependencies are automatically installed using platformio.ini (lib_deps)
    Path of the lib_deps installation : /Synth/Firmware/.pio/libdeps/teensy40/...

### Firmware main file
    /Synth/Firmware/src/main.cpp
 
## USB_MTP_MIDI - prerequisite
### Update Platformio with the last teensyduino framework

    cd ~/.platformio/packages/framework-arduinoteensy
    rm -r cores
    git clone https://github.com/PaulStoffregen/cores.git

### Create the USB_MTPDISK_MIDI inerface
    ~/.platformio/packages/framework-arduinoteensy/cores/teensy4/usb_desc.h > ADD this deffinition

    #elif defined(USB_MTPDISK_MIDI)
      #define VENDOR_ID		            0x16E0
      #define PRODUCT_ID		          0x03D1
      #define MANUFACTURER_NAME	      {'T','e','e','n','s','y','d','u','i','n','o'}
      #define MANUFACTURER_NAME_LEN	  11
      #define PRODUCT_NAME		        {'T','e','e','n','s','y',' ','M','T','P',' ','M','I','D','I'}
      #define PRODUCT_NAME_LEN	      15
      #define EP0_SIZE		            64
      #define NUM_ENDPOINTS           5
      #define NUM_INTERFACE           3
      #define MTP_INTERFACE		        3   /* MTPDISK */
      #define MTP_TX_ENDPOINT	        4
      #define MTP_TX_SIZE_12	        64
      #define MTP_TX_SIZE_480	        512
      #define MTP_RX_ENDPOINT	        4
      #define MTP_RX_SIZE_12	        64
      #define MTP_RX_SIZE_480	        512
      #define MTP_EVENT_ENDPOINT	    5
      #define MTP_EVENT_SIZE	        32
      #define MTP_EVENT_INTERVAL_12	  10
      #define MTP_EVENT_INTERVAL_480  7
      #define SEREMU_INTERFACE        2	  /* SERIAL_EMULATION */
      #define SEREMU_TX_ENDPOINT      2
      #define SEREMU_TX_SIZE          64
      #define SEREMU_TX_INTERVAL      1
      #define SEREMU_RX_ENDPOINT      2
      #define SEREMU_RX_SIZE          32
      #define SEREMU_RX_INTERVAL      2
      #define MIDI_INTERFACE	        1	  /* MIDI */
      #define MIDI_NUM_CABLES	        1
      #define MIDI_TX_ENDPOINT	      3
      #define MIDI_TX_SIZE_12	        64
      #define MIDI_TX_SIZE_480	      512
      #define MIDI_RX_ENDPOINT	      3
      #define MIDI_RX_SIZE_12	        64
      #define MIDI_RX_SIZE_480	      512
      #define ENDPOINT2_CONFIG	      ENDPOINT_RECEIVE_INTERRUPT + ENDPOINT_TRANSMIT_INTERRUPT
      #define ENDPOINT3_CONFIG	      ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_BULK
      #define ENDPOINT4_CONFIG	      ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_BULK
      #define ENDPOINT5_CONFIG	      ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_INTERRUPT

    ~/.platformio/platforms/teensy/builder/frameworks/arduino.py > ADD:
    line 59: "USB_MTPDISK_MIDI",

    ~/.platformio/packages/framework-arduinoteensy/cores/teensy4/usb.c > ADD:
    ligne 1031: #if defined(USB_MTPDISK) || defined(USB_MTPDISK_SERIAL) || (USB_MTPDISK_MIDI)

    ~/.pio/libdeps/teensy40/MTP_Teensy/src/MTP_Teensy.cpp > ADD:
    ligne 28: #if defined(USB_MTPDISK) || defined(USB_MTPDISK_SERIAL) || defined(USB_MTPDISK_MIDI)

    ~/.pio/libdeps/teensy40/MTP_Teensy/src/MTP_Teensy.h > ADD:
    line 32: #if !defined(USB_MTPDISK) && !defined(USB_MTPDISK_SERIAL) && !defined(USB_MTPDISK_MIDI)

    ~/.pio/libdeps/teensy40/MTP_Teensy/src/MTP_Teensy.h > COMMENT:
    line 59: #include <MemoryHexDump.h>

### Powering the eTextile-Synthesizer
- The Teensy Micro USB Type B **will not power** the eTextile-Synthesizer
- You must use the included power cable with an **5Volts AC/DC power Plug** or your laptop USB plug

### Program Synopsis
The embedded Software is implementing **image analysis algorithms** on the E256 eTextile-matrix-sensor to design a new tactile interaction vocabulary.
- **Bilinear interpolation** The 16x16 Analog pressure sensor values are interpolated with a bilinear algorithm
- **Blob tracking** The interpolated pressure matrix sensor values are analyzed with a Connected Component Labelling algorithm
- **Blob ID management** each blob is tracked in space and time using single chained linked list
- **Blob shape and movement characterisation**: blobs coordinates, size and pressure

### Benchmark
  - ADC_INPUT : 2500 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION : ...
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING : 550 FPS
  - ADC_INPUT / BILINEAR_INTERPOLATION / BLOB_TRACKING / AUDIO : ...

## Configuring the system
The firmware is using a **platformio.ini** to select the needed fonctionnalites. Uncomment the lines that you need.

### Use the eTextile-Synthesizer as COMPUTER_INTERFACE
- [USB_MIDI]: Transmit touch coordinates via USB_MIDI
- [USB_MTP_MIDI]: Experimental
- [HARDWARE_MIDI]: Transmit touch coordinates via HARDWARE_MIDI
- [USB_OSC]: Transmit touch coordinates via USB_SLIP_OSC

### Use the eTextile-Synthesizer as STANDALONE divice
- [SYNTH_PLAYER] : Set the eTextile-Synthesizer as polyphonic synthesizer
- [GRANULAR_PLAYER] : Set the eTextile-Synthesizer as 
- [FLASH_PLAYER] : Set the eTextile-Synthesizer as 

### Mapping functionality (IN_PROGRESS)
- [MAPPING_LAYOUT]
  - [GRID_LAYOUT_MIDI_IN]
  - [GRID_LAYOUT_DEFAULT]
  - [GRID_LAYOUT_HARMONIC]
  - [GRID_LAYOUT_NOTES]
  - [GRID_LAYOUT_FREQ]

## Copyright
Except as otherwise noted, all files in the eTextile-Synthesizer project folder

    Copyright (c) 20014- Maurin Donneaud

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in the eTextile-Synthesizer project folder.

## TODO
- **Gesture Recognizer** could be a nice feature that fit the eTextile-Synthesizer tactile surface.
- **MIDI MPE** : MIDI in general does not have continuous control of note parameters (pitch and velocity) after the note has been played. Some companies like Roli with MPE have tried to address these limitations, but support is not as wide as standard control changes.
- **Teensy Synthesizer** / Open source
    - [MicroDexed](https://www.parasitstudio.de/)
    - [Dexed - FM Plugin Synth](https://github.com/asb2m10/dexed)