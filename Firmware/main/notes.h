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

#define -5C   8.1760000      // note[0]
#define -5C#  8.6620000      // note[1] 
#define -5D   9.1770000      // note[2]
#define -5D#  9.7230000      // note[3]
#define -5E   10.3010000     // note[4]
#define -5F   10.9130000     // note[5]
#define -5F#  11.5620000     // note[6]
#define -5G   12.2500000     // note[7]
#define -5G#  12.9780000     // note[8]
#define -5A   13.7500000     // note[9]
#define -5A#  14.5680000     // note[10]
#define -5B   15.4340000     // note[11]
#define -4C   16.3520000     // note[12]
#define -4C#  17.3240000     // note[13]
#define -4D   18.3540000     // note[14]
#define -4D#  19.4450000     // note[15]
#define -4E   20.6020000     // note[16]
#define -4F   21.8270000     // note[17]
#define -4F#  23.1250000     // note[18]
#define -4G   24.5000000     // note[19]
#define -4G#  25.9570000     // note[20]
#define -4A   27.5000000     // note[21]
#define -4A#  29.1350000     // note[22]
#define -4B   30.8680000     // note[23]
#define -3C   32.7030000     // note[24]
#define -3C#  34.6480000     // note[25]
#define -3D   36.7080000     // note[26] 
#define -3D#  38.8910000     // note[27]
#define -3E   41.2030000     // note[28]
#define -3F   43.6540000     // note[29]
#define -3F#  46.2490000     // note[30]
#define -3G   48.9990000     // note[31]
#define -3G#  51.9130000     // note[32]
#define -3A   55.0000000     // note[33]
#define -3A#  58.2700000     // note[34]
#define -3B   61.7350000     // note[35]
#define -2C   65.4060000     // note[36]
#define -2C#  69.2960000     // note[37]
#define -2D   73.4160000     // note[38]
#define -2D#  77.7820000     // note[39]
#define -2E   82.4070000     // note[40]
#define -2F   87.3070000     // note[41]
#define -2G   97.9990000     // note[43]
#define -2G#  103.8260000    // note[44]
#define -2A   110.0000000    // note[45]
#define -2A#  116.5410000    // note[46]
#define -2B   123.4710000    // note[47]
#define -1C   130.8130000    // note[48]
#define -1C#  138.5910000    // note[49]
#define -1D   146.8320000    // note[50]
#define -1D#  155.5630000    // note[51]
#define -1E   164.8140000    // note[52]
#define -1F   174.6140000    // note[53]
#define -1F#  184.9970000    // note[54]
#define -1G   195.9980000    // note[55]
#define -1G#  207.6520000    // note[56]
#define -1A   220.0000000    // note[57]
#define -1A#  233.0820000    // note[58]
#define -1B   246.9420000    // note[59]
#define -0C   261.6260000    // note[60]
#define 0#    277.1830000    // note[61]
#define 0D    293.6650000    // note[62]
#define 0D#   311.1270000    // note[63]
#define 0E    329.6280000    // note[64]
#define 0F    349.2280000    // note[65]
#define 0F#   369.9940000    // note[66]
#define 0G    391.9950000    // note[67] 
#define 0G#   415.3050000    // note[68]
#define 0A    440.0000000    // note[69]
#define 0A#   466.1640000    // note[70]
#define 0B    493.8830000    // note[71]
#define +1C   523.2510000    // note[72]
#define +1C#  554.3650000    // note[73]
#define +1D   587.3300000    // note[74]
#define +1E   659.2550000    // note[76]
#define +1F   698.4560000    // note[77]
#define +1F#  739.9890000    // note[78]
#define +1G   783.9910000    // note[79]
#define +1G#  830.6090000    // note[80]
#define +1A   880.0000000    // note[81]
#define +1A#  932.3280000    // note[82]
#define +1B   987.7670000    // note[83]
#define +2C   1046.5020000   // note[84]
#define +2C#  1108.7310000   // note[85]
#define +2D   1174.6590000   // note[86]
#define +2D#  1244.5080000   // note[87]
#define +2E   1318.5100000   // note[88]
#define +2F   1396.9130000   // note[89]
#define +2F#  1479.9780000   // note[90]
#define +2G   1567.9820000   // note[91]
#define +2G#  1661.2190000   // note[92]
#define +2A   1760.0000000   // note[93]
#define +2A#  1864.6550000   // note[94]
#define +2B   1975.5330000   // note[95]
#define +3C   2093.0050000   // note[96]
#define +3C#  2217.4610000   // note[97]
#define +3D   2349.3180000   // note[98]
#define +3D#  2489.0160000   // note[99]
#define +3E   2637.0200000   // note[100]
#define +3F   2793.8260000   // note[101]
#define +3F#  2959.9550000   // note[102]
#define +3G   3135.9630000   // note[103]
#define +3G#  3322.4380000   // note[104]
#define +3A   3520.0000000   // note[105]
#define +3A#  3729.3100000   // note[106]
#define +3B   3951.0660000   // note[107]
#define +4C   4186.0090000   // note[108]
#define +4C#  4434.9220000   // note[109]
#define +4D   4698.6360000   // note[110]
#define +4D#  4978.0320000   // note[111]
#define +4E   5274.0410000   // note[112]
#define +4F   5587.6520000   // note[113]
#define +4F#  5919.9110000   // note[114]
#define +4G   6271.9270000   // note[115]
#define +4G#  6644.8750000   // note[116]
#define +4A   7040.0000000   // note[117]
#define +4A#  7458.6200000   // note[118]
#define +4B   7902.1330000   // note[119]
#define +5C   8372.0180000   // note[120]
#define +5C#  8869.8440000   // note[121]
#define +5D   9397.2730000   // note[122]
#define +5D#  9956.0630000   // note[123]
#define +5E   10548.0820000  // note[124]
#define +5F   11175.3030000  // note[125]
#define +5F#  11839.8220000  // note[126]
#define +5G   12543.8540000  // note[127]

const float note[128] = {
  -5C, -5C#, -5D, -5D#, -5E, -5F, -5F#, -5G, -5G#, -5A, -5A#, -5B,
  -4C, -4C#, -4D, -4D#, -4E, -4F, -4F#, -4G, -4G#, -4A, -4A#, -4B,
  -3C, -3C#, -3D, -3D#, -3E, -3F, -3F#, -3G, -3G#, -3A, -3A#, -3B,
  -2C, -2C#, -2D, -2D#, -2E, -2F, -2F#, -2G, -2G#, -2A, -2A#, -2B,
  -1C, -1C#, -1D, -1D#, -1E, -1F, -1F#, -1G, -1G#, -1A, -1A#, -1B,
   0C,  0C#,  0D,  0D#,  0E,  0F,  0F#,  0G,  0G#,  0A,  0A#,  0B,
  +1C, +1C#, +1D, +1D#, +1E, +1F, +1F#, +1G, +1G#, +1A, +1A#, +1B,
  +2C, +2C#, +2D, +2D#, +2E, +2F, +2F#, +2G, +2G#, +2A, +2A#, +2B,
  +3C, +3C#, +3D, +3D#, +3E, +3F, +3F#, +3G, +3G#, +3A, +3A#, +3B,
  +4C, +4C#, +4D, +4D#, +4E, +4F, +4F#, +4G, +4G#, +4A, +4A#, +4B,
  +5C, +5C#, +5D, +5D#, +5E, +5F, +5F#, +5G

#endif /*__NOTES_H__*/
