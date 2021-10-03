# eTextile-Synthesizer / Software / Demo
### Exploring music through textile

The eTextile-Synthesizer is designed to be used as **Standalone Audio Synthesizer** 
but it can as well be used as **MIDI controler** for Audio applications such as **Ableton Live**, **MaxMsp**, **Puredata**, etc.
The blobs values are transmited to the computer over USB-MIDI.
The below specifications descrybe how the blobs values are encoded over the MIDI standard comunication protocol.
 
## Blobs MIDI Specifications
| **Feature**             | **MIDI message**                       |
| :---------------------- | :------------------------------------- |
| Blob State ON           | noteOn(BI[1:8], BS[1], 1);             |
| Blob X value            | controlChange(BX[3], [0:127], BI[1:8]) |
| Blob Y value            | controlChange(BY[4], [0:127], BI[1:8]) |
| Blob Z value            | controlChange(BZ[5], [0:127], BI[1:8]) |
| Blob W value            | controlChange(BW[6], [0:127], BI[1:8]) |
| Blob H value            | controlChange(BH[7], [0:127], BI[1:8]) |
| Blob State OFF          | noteOff(BI[1:8], BS[0], 1);                |

| **Blob code** |  **Description**  |
| :------------ | :---------------- |
|     BI[1:8]   | Blob UID          |
|     BS[0:1]   | Blob state        |
|     BX[3]     | Blob X centroid   |
|     BY[4]     | Blob Y centroid   |
|     BZ[5]     | Blob Depth        |
|     BW[6]     | Blob width        |
|     BD[7]     | Blob Height       |


## Software DEMOS
This folder is containing some demos that can be used as starting point for your own creations.

- [Ableton Live](https://www.ableton.com/en/live/ "Ableton-Live")
  - **SYNTH_MIDI.als**
  - **SYNTH_MPE.als** (MIDI Polyphonic Expression). This way of using MIDI allows MPE-capable devices to control multiple parameters of every note in real time for more expressive instrumental performances. TODO

- [MaxMsp](http://msp.ucsd.edu/ "Miller Puckette")
  - **SYNTH_SLIP-OSC.maxpat** TODO
  - **SYNTH_MIDI.maxpat**
  - **SYNTH_MPE.maxpat** TODO

- [Pure Data](http://msp.ucsd.edu/ "Miller Puckette")
  - **SYNTH_SLIP-OSC.pd** TODO
    - This patch depends on several third party externals available through **deken plugin** (Help -> Find externals):
	- **comport** : 
	- **mrpeach** : OSC slipenc and slipdec
	- **cyclone** : 
  - **SYNTH_MIDI.pd**
  - **SYNTH_MPE.pd** TODO
