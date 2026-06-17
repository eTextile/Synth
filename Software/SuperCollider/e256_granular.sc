// ════════════════════════════════════════════════════════════════════════════
// eTextile-Synthesizer — Live Granular Synthesis
// Audio input granularisé, tous paramètres adressés via MIDI CC
//
// CC MAP
//   CC  1  grain duration        0.01 – 2.0 s       (exp)
//   CC  2  grain density         1 – 200 grains/s    (exp)
//   CC  3  pitch / playback rate 0.25 – 4.0 ×        (exp)
//   CC  4  buffer position       0 – 100 %           (lin → 0–bufDur s)
//   CC  5  position scatter      0 – 100 %           (lin)
//   CC  6  rate scatter          0 – 100 %           (lin)
//   CC  7  pan spread            0 – ±1              (lin)
//   CC  8  output amplitude      0 – 100 %           (lin)
//   CC 11  dry / wet mix         0 = dry  1 = wet    (lin)
//   CC 12  feedback              0 – 95 %            (lin)
//   CC 64  freeze record         0 = rec  1 = freeze (toggle)
//   CC 71  filter resonance      0 – 95 %            (lin)
//   CC 74  filter cutoff         200 – 8000 Hz       (exp)
// ════════════════════════════════════════════════════════════════════════════

(
s.waitForBoot {

  var bufDur = 8.0;  // secondes de buffer live

  // ── Buffer audio live ────────────────────────────────────────────────────
  ~buf = Buffer.alloc(s, (s.sampleRate * bufDur).round, 1);

  // ── Control buses (un par CC) ────────────────────────────────────────────
  ~buses = IdentityDictionary[
    1  -> Bus.control(s),   // grain duration
    2  -> Bus.control(s),   // grain density
    3  -> Bus.control(s),   // rate / pitch
    4  -> Bus.control(s),   // buffer position
    5  -> Bus.control(s),   // position scatter
    6  -> Bus.control(s),   // rate scatter
    7  -> Bus.control(s),   // pan spread
    8  -> Bus.control(s),   // amplitude
    11 -> Bus.control(s),   // dry / wet
    12 -> Bus.control(s),   // feedback
    64 -> Bus.control(s),   // freeze
    71 -> Bus.control(s),   // filter resonance
    74 -> Bus.control(s)    // filter cutoff
  ];

  // ── Specs CC→paramètre + valeurs par défaut ──────────────────────────────
  ~specs = IdentityDictionary[
    //   cc -> [ControlSpec,                       default ]
    1  -> [ControlSpec(0.01, 2.0,  \exp),          0.1   ],
    2  -> [ControlSpec(1,    200,  \exp),           20.0  ],
    3  -> [ControlSpec(0.25, 4.0,  \exp),           1.0   ],
    4  -> [ControlSpec(0.0,  1.0,  \lin),           0.0   ],
    5  -> [ControlSpec(0.0,  1.0,  \lin),           0.0   ],
    6  -> [ControlSpec(0.0,  1.0,  \lin),           0.0   ],
    7  -> [ControlSpec(0.0,  1.0,  \lin),           0.0   ],
    8  -> [ControlSpec(0.0,  1.0,  \lin),           0.7   ],
    11 -> [ControlSpec(0.0,  1.0,  \lin),           1.0   ],
    12 -> [ControlSpec(0.0,  0.95, \lin),           0.0   ],
    64 -> [ControlSpec(0,    1,    \lin),           0     ],
    71 -> [ControlSpec(0.0,  0.95, \lin),           0.0   ],
    74 -> [ControlSpec(200,  8000, \exp),           4000.0]
  ];

  // Initialisation des buses aux valeurs par défaut
  ~specs.keysValuesDo { |cc, pair| ~buses[cc].set(pair[1]) };

  // ── SynthDef : enregistrement en boucle de l'entrée audio ────────────────
  SynthDef(\e256_recorder, { |buf, freezeBus|
    var freeze = In.kr(freezeBus);
    RecordBuf.ar(SoundIn.ar(0), buf, loop: 1, run: 1 - freeze);
  }).add;

  // ── SynthDef : moteur granulaire ─────────────────────────────────────────
  SynthDef(\e256_granular, {
    |buf, bufDurArg = 8.0,
     durBus, densityBus, rateBus, posBus, posRandBus, rateRandBus,
     panBus, ampBus, mixBus, feedbackBus, cutoffBus, resonBus|

    var grainDur  = In.kr(durBus);
    var density   = In.kr(densityBus);
    var rate      = In.kr(rateBus);
    var pos       = In.kr(posBus)     * bufDurArg;        // 0–1 → 0–bufDur s
    var posRand   = In.kr(posRandBus) * bufDurArg;
    var rateRand  = In.kr(rateRandBus);
    var panSpread = In.kr(panBus);
    var amp       = In.kr(ampBus);
    var mix       = In.kr(mixBus);
    var feedback  = In.kr(feedbackBus);
    var cutoff    = In.kr(cutoffBus).clip(20, 20000);
    var reson     = (1 - In.kr(resonBus)).clip(0.01, 1);

    var trig   = Impulse.ar(density);
    var fbIn   = LocalIn.ar(2);

    var grains = TGrains.ar(2,
      trigger:   trig,
      bufnum:    buf,
      rate:      rate   + LFNoise1.kr(4).bipolar(rateRand * 0.5),
      centerPos: pos    + LFNoise1.kr(3).bipolar(posRand),
      dur:       grainDur,
      pan:       LFNoise1.kr(5).bipolar(panSpread),
      amp:       0.5
    );

    var wet = RLPF.ar(grains + (fbIn * feedback), cutoff, reson);
    LocalOut.ar(wet);

    var dry = Pan2.ar(SoundIn.ar(0));
    Out.ar(0, XFade2.ar(dry, wet * amp, mix * 2 - 1));
  }).add;

  s.sync;

  // ── Lancement des synths ─────────────────────────────────────────────────
  ~rec = Synth(\e256_recorder, [
    \buf,       ~buf,
    \freezeBus, ~buses[64].index
  ]);

  // after : granulaire tourne après le recorder dans le node tree
  ~gran = Synth.after(~rec, \e256_granular, [
    \buf,        ~buf,
    \bufDurArg,  bufDur,
    \durBus,     ~buses[1].index,
    \densityBus, ~buses[2].index,
    \rateBus,    ~buses[3].index,
    \posBus,     ~buses[4].index,
    \posRandBus, ~buses[5].index,
    \rateRandBus,~buses[6].index,
    \panBus,     ~buses[7].index,
    \ampBus,     ~buses[8].index,
    \mixBus,     ~buses[11].index,
    \feedbackBus,~buses[12].index,
    \cutoffBus,  ~buses[74].index,
    \resonBus,   ~buses[71].index
  ]);

  // ── MIDI ─────────────────────────────────────────────────────────────────
  MIDIClient.init;
  MIDIIn.connect(0, MIDIClient.sources.detect { |src| src.device == "ETEXTILE_SYNTH" });
  ~midiOut = MIDIOut.newByName("ETEXTILE_SYNTH", "MIDI 1");
  ~midiOut.sysex(Int8Array[0xF0.asInteger, 0x7D, 0x01, 0x10, 0xF7.asInteger]); // USB_INTERFACE_MODE

  MIDIdef.cc(\e256_gran, { |val, num|
    var pair = ~specs[num];
    if (pair.notNil) {
      ~buses[num].set(pair[0].map(val / 127.0));
    };
  });

  // ── Nettoyage sur Cmd+. ──────────────────────────────────────────────────
  CmdPeriod.add({
    ~rec.free; ~gran.free; ~buf.free;
    ~buses.do(_.free);
    MIDIdef(\e256_gran).free;
    ~midiOut.free;
  });

  "── e256 granular synthesis ready ──".postln;
};
)
