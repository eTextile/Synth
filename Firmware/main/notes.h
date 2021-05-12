/*
  This file is part of the eTextile-Synthesizer project - http://synth.eTextile.org
  Copyright (c) 2014- Maurin Donneaud <maurin@etextile.org>
  This work is licensed under Creative Commons Attribution-ShareAlike 4.0 International license, see the LICENSE file for details.
*/

#ifndef __NOTES_H__
#define __NOTES_H__

#include "config.h"

// TODO
const char harmonicKeyboard[128] = {
  'G', 'B', 'G#', 'C', 'A', 'C#', 'A#', 'D', 'B', 'D#', 'C', 'E', 'C#', 'F',
  'C', 'E', 'C#', 'F', 'D', 'F#', 'D#', 'G', 'E', 'G#', 'F', 'A', 'F#', 'A#',
};

#define  N5C   8.1760000      // note[0]
#define  N5C#  8.6620000      // note[1] 
#define  N5D   9.1770000      // note[2]
#define  N5D#  9.7230000      // note[3]
#define  N5E   10.3010000     // note[4]
#define  N5F   10.9130000     // note[5]
#define  N5F#  11.5620000     // note[6]
#define  N5G   12.2500000     // note[7]
#define  N5G#  12.9780000     // note[8]
#define  N5A   13.7500000     // note[9]
#define  N5A#  14.5680000     // note[10]
#define  N5B   15.4340000     // note[11]
#define  N4C   16.3520000     // note[12]
#define  N4C#  17.3240000     // note[13]
#define  N4D   18.3540000     // note[14]
#define  N4D#  19.4450000     // note[15]
#define  N4E   20.6020000     // note[16]
#define  N4F   21.8270000     // note[17]
#define  N4F#  23.1250000     // note[18]
#define  N4G   24.5000000     // note[19]
#define  N4G#  25.9570000     // note[20]
#define  N4A   27.5000000     // note[21]
#define  N4A#  29.1350000     // note[22]
#define  N4B   30.8680000     // note[23]
#define  N3C   32.7030000     // note[24]
#define  N3C#  34.6480000     // note[25]
#define  N3D   36.7080000     // note[26] 
#define  N3D#  38.8910000     // note[27]
#define  N3E   41.2030000     // note[28]
#define  N3F   43.6540000     // note[29]
#define  N3F#  46.2490000     // note[30]
#define  N3G   48.9990000     // note[31]
#define  N3G#  51.9130000     // note[32]
#define  N3A   55.0000000     // note[33]
#define  N3A#  58.2700000     // note[34]
#define  N3B   61.7350000     // note[35]
#define  N2C   65.4060000     // note[36]
#define  N2C#  69.2960000     // note[37]
#define  N2D   73.4160000     // note[38]
#define  N2D#  77.7820000     // note[39]
#define  N2E   82.4070000     // note[40]
#define  N2F   87.3070000     // note[41]
#define  N2G   97.9990000     // note[43]
#define  N2G#  103.8260000    // note[44]
#define  N2A   110.0000000    // note[45]
#define  N2A#  116.5410000    // note[46]
#define  N2B   123.4710000    // note[47]
#define  N1C   130.8130000    // note[48]
#define  N1C#  138.5910000    // note[49]
#define  N1D   146.8320000    // note[50]
#define  N1D#  155.5630000    // note[51]
#define  N1E   164.8140000    // note[52]
#define  N1F   174.6140000    // note[53]
#define  N1F#  184.9970000    // note[54]
#define  N1G   195.9980000    // note[55]
#define  N1G#  207.6520000    // note[56]
#define  N1A   220.0000000    // note[57]
#define  N1A#  233.0820000    // note[58]
#define  N1B   246.9420000    // note[59]
#define  C     261.6260000    // note[60]
#define  C#    277.1830000    // note[61]
#define  D     293.6650000    // note[62]
#define  D#    311.1270000    // note[63]
#define  E     329.6280000    // note[64]
#define  F     349.2280000    // note[65]
#define  F#    369.9940000    // note[66]
#define  G     391.9950000    // note[67] 
#define  G#    415.3050000    // note[68]
#define  A     440.0000000    // note[69]
#define  A#    466.1640000    // note[70]
#define  B     493.8830000    // note[71]
#define  P1C    523.2510000    // note[72]
#define  P1C#  554.3650000    // note[73]
#define  P1D   587.3300000    // note[74]
#define  P1E   659.2550000    // note[76]
#define  P1F   698.4560000    // note[77]
#define  P1F#  739.9890000    // note[78]
#define  P1G   783.9910000    // note[79]
#define  P1G#  830.6090000    // note[80]
#define  P1A   880.0000000    // note[81]
#define  P1A#  932.3280000    // note[82]
#define  P1B   987.7670000    // note[83]
#define  P2C   1046.5020000   // note[84]
#define  P2C#  1108.7310000   // note[85]
#define  P2D   1174.6590000   // note[86]
#define  P2D#  1244.5080000   // note[87]
#define  P2E   1318.5100000   // note[88]
#define  P2F   1396.9130000   // note[89]
#define  P2F#  1479.9780000   // note[90]
#define  P2G   1567.9820000   // note[91]
#define  P2G#  1661.2190000   // note[92]
#define  P2A   1760.0000000   // note[93]
#define  P2A#  1864.6550000   // note[94]
#define  P2B   1975.5330000   // note[95]
#define  P3C   2093.0050000   // note[96]
#define  P3C#  2217.4610000   // note[97]
#define  P3D   2349.3180000   // note[98]
#define  P3D#  2489.0160000   // note[99]
#define  P3E   2637.0200000   // note[100]
#define  P3F   2793.8260000   // note[101]
#define  P3F#  2959.9550000   // note[102]
#define  P3G   3135.9630000   // note[103]
#define  P3G#  3322.4380000   // note[104]
#define  P3A   3520.0000000   // note[105]
#define  P3A#  3729.3100000   // note[106]
#define  P3B   3951.0660000   // note[107]
#define  P4C   4186.0090000   // note[108]
#define  P4C#  4434.9220000   // note[109]
#define  P4D   4698.6360000   // note[110]
#define  P4D#  4978.0320000   // note[111]
#define  P4E   5274.0410000   // note[112]
#define  P4F   5587.6520000   // note[113]
#define  P4F#  5919.9110000   // note[114]
#define  P4G   6271.9270000   // note[115]
#define  P4G#  6644.8750000   // note[116]
#define  P4A   7040.0000000   // note[117]
#define  P4A#  7458.6200000   // note[118]
#define  P4B   7902.1330000   // note[119]
#define  P5C   8372.0180000   // note[120]
#define  P5C#  8869.8440000   // note[121]
#define  P5D   9397.2730000   // note[122]
#define  P5D#  9956.0630000   // note[123]
#define  P5E   10548.0820000  // note[124]
#define  P5F   11175.3030000  // note[125]
#define  P5F#  11839.8220000  // note[126]
#define  P5G   12543.8540000  // note[127]

const float note[128] = {
  N5C,  N5C#,  N5D,  N5D#,  N5E,  N5F,  N5F#,  N5G,  N5G#,  N5A,  N5A#,  N5B,
  N4C,  N4C#,  N4D,  N4D#,  N4E,  N4F,  N4F#,  N4G,  N4G#,  N4A,  N4A#,  N4B,
  N3C,  N3C#,  N3D,  N3D#,  N3E,  N3F,  N3F#,  N3G,  N3G#,  N3A,  N3A#,  N3B,
  N2C,  N2C#,  N2D,  N2D#,  N2E,  N2F,  N2F#,  N2G,  N2G#,  N2A,  N2A#,  N2B,
  N1C,  N1C#,  N1D,  N1D#,  N1E,  N1F,  N1F#,  N1G,  N1G#,  N1A,  N1A#,  N1B,
  0C,  0C#,  0D,  0D#,  0E,  0F,  0F#,  0G,  0G#,  0A,  0A#,  0B,
   P1C,  P1C#,  P1D,  P1D#,  P1E,  P1F,  P1F#,  P1G,  P1G#,  P1A,  P1A#,  P1B,
   P2C,  P2C#,  P2D,  P2D#,  P2E,  P2F,  P2F#,  P2G,  P2G#,  P2A,  P2A#,  P2B,
   P3C,  P3C#,  P3D,  P3D#,  P3E,  P3F,  P3F#,  P3G,  P3G#,  P3A,  P3A#,  P3B,
   P4C,  P4C#,  P4D,  P4D#,  P4E,  P4F,  P4F#,  P4G,  P4G#,  P4A,  P4A#,  P4B,
   P5C,  P5C#,  P5D,  P5D#,  P5E,  P5F,  P5F#,  P5G

#endif /*__NOTES_H__*/
