# eTextile-Synthetizer 
### Exploring music through textile

![eTextile Synthetizer](https://live.staticflickr.com/65535/48916850368_dd34d00418_c_d.jpg)

## About The project
After setting up the **E256 eTextile matrix sensor** it is now time to use it for live electronic music performances. **eTextile-Synthesizers** are handcrafted multi-touch digital interfaces based on smart textile processes and custom electronic solutions. These eTextile-synthesizers open up new possibilities for electronic music playing. The project features a fully embedded **multitouch eTextile-Synthesizer** that combines the powerful **Teensy 4.0** MCU and its audio library with the E256 eTextile matrix sensor technology. One of the main purpose of this project is to give priority to the tactility of this textile interface. To promote an intuitive music interpretation and deep listening, the eTextile-Synthesizer offers different textures that guide the touch and form tactile cues.

### Development tools & DOCs
* **Web page:** [https://synth.eTextile.org/](https://synth.eTextile.org "Project web page")
* **Repository:** [https://github.com/eTextile/synth/tree/master/](https://github.com/eTextile/synth/tree/master/ "Project repository")
* **License:** [CC-BY-SA](https://github.com/eTextile/synth/tree/master/LICENSE.txt "Project License")
* **Mail:** [contact@etextile.org](mailto:contact@etextile.org "eMail us")
* **Pictures:** [flickr.com](https://www.flickr.com/photos/maurin/albums/72157673740361510/ "Share your pictures with us")

## Project content
* **Hardware_textile:** eTextile-Synthetizer tutorial
* **Hardware_electronic:** Open Hardware electronic parts (Made with KiCad) 
* **Firmware:** eTextile-Synthetizer firmware (Compile with Arduino)
* **Softwear:** demos applications (PureData, MaxMSP, Max4Live, etc.)

## Specifications
| Feature                 | Description                            |
| :---------------------- | :------------------------------------- |
| Power-IN                | DC-5V                                  |
| Hardware resolution     | 16x16 FSRs with hardware interpolation |
| Software X-Y resolution | [0:64] floating point unit             |
| Software Z resolution   | [0:127] integer unit                   |
| Multi-Touch capability  | Max points - User defined              |
| Points characteristics  | PosX, PosY, sizeX, sizeY, ZPressure    |
| Latency                 | 950 FPS (xms)                          |
| Audio IN                | ADC 16bits (mini-jack)                 |
| Audio OUT               | DAC 16bits (mini-jack)                 |
| Audio OUT               | Headphone (mini-jack)                  |
| Connectivity            | MIDI I/O (mini-jack)                   |
| Connectivity            | USB_SLIP-OSC (FIXME)                   |
| Connectivity            | USB_MIDI                               |
| Connectivity            | USB_MPE (TODO)                         |

## Publications
- [NIME17](http://www.nime2017.org/) - Designing a Multi-Touch eTextile for Music Performances / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/NIME17-eTextile.pdf) - [Video](https://vimeo.com/217690743)
- [MOCO17](http://moco17.movementcomputing.org/) - Music Skin: Fabric Interface for Expressive Music Control / [Paper](https://github.com/eTextile/Matrix/blob/teensy_matrix/docs/publications/MOCO17-MusicSkin.pdf)

## Copyright
Except as otherwise noted, all files in the eTextile-Synthetizer project folder

    Copyright (c) 2014-2018 Maurin Donneaud and others.

For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see LICENSE.txt included in this folder.

## Docs
- [Video made at Schmiede 2014](http://www.kobakant.at/DIY/?p=4305/)
