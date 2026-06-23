// ════════════════════════════════════════════════════════════════════════════
// eTextile-Synthesizer — Live Granular + Filtre + Reverb (Bela)
// SoundIn → PitchShift → RLPF → FreeVerb → out
//
// MAPPING 2 TOUCHES — configurer le firmware pour envoyer :
//
//   Touch 1  X → CC 1   filter cutoff    200 – 8000 Hz (exp)  gauche=sombre droite=clair
//   Touch 1  Y → CC 2   pitch ratio      0.5 – 2.0 ×  (exp)  bas=grave  haut=aigu
//   Touch 1  Z → CC 3   amplitude        0 – 100 %    (lin)  pression = volume
//
//   Touch 2  X → CC 4   reverb room      0 – 100 %    (lin)  gauche=sec  droite=hall
//   Touch 2  Y → CC 5   reverb mix       0 – 100 %    (lin)  bas=sec    haut=reverb
//   Touch 2  Z → CC 6   dry / wet        0 – 100 %    (lin)  pression = engage effet
//
//   Fixe : grain window 0.1s · pitch disp 0 · time disp 0 · damping 0.5 · pan 0
// ════════════════════════════════════════════════════════════════════════════

s = Server.default;

s.options.numAnalogInChannels  = 2;
s.options.numAnalogOutChannels = 2;
s.options.pgaGainLeft          = 40;
s.options.pgaGainRight         = 40;
s.options.numDigitalChannels   = 0;
s.options.blockSize            = 16;
s.options.numInputBusChannels  = 2;
s.options.numOutputBusChannels = 2;
s.options.numWireBufs          = 256;
s.options.memSize              = 16384;

// ── MIDI — init avant le boot serveur ────────────────────────────────────────
~handshake_done = false;
~amidi_port     = nil;
~midiOut        = nil;
MIDIClient.init;

MIDIdef.sysex(\e256_sysex, { |msg|
    var b = msg.asArray;
    if (b.size >= 5 && (b[0] & 0xFF) == 0xF0 && (b[1] & 0xFF) == 0x7D && (b[2] & 0xFF) == 0x02) {
        if (b[3] == 18 && ~handshake_done.not) {
            ~handshake_done = true;
            "── USB_INTERFACE_MODE confirmed (SysEx ACK) ──".postln;
        };
    };
});

~amidi_sysex = { |hex|
    if (~amidi_port.notNil) {
        ("amidi -p " ++ ~amidi_port ++ " -S '" ++ hex ++ "'").unixCmd;
    };
};

~find_amidi_port = {
    var pipe = Pipe.new("amidi -l 2>&1 | grep ETEXTILE | awk '{print $2}'", "r");
    var port = pipe.getLine;
    pipe.close;
    port
};

~send_handshake = {
    if (~handshake_done.not) {
        "── SYNC → USB_INTERFACE_MODE ──".postln;
        ~amidi_sysex.value("F0 7D 01 01 F7");
        0.5.wait;
        if (~handshake_done.not) {
            ~amidi_sysex.value("F0 7D 01 10 F7");
        };
        0.5.wait;
    };
};

~midi_connect = {
    ~midiSrc = MIDIClient.sources.detect      { |e| e.device == "ETEXTILE_SYNTH" };
    ~midiDst = MIDIClient.destinations.detect { |e| e.device == "ETEXTILE_SYNTH" };
    if (~midiSrc.notNil && ~midiDst.notNil) {
        MIDIIn.connect(0, ~midiSrc);
        ~midiOut = MIDIOut.new(0, ~midiDst.uid);
        ~amidi_port = ~find_amidi_port.value;
        ("amidi port: " ++ (~amidi_port ?? "non trouvé")).postln;
        {
            0.5.wait;
            5.do { if (~handshake_done.not) { ~send_handshake.value } };
            while { ~handshake_done.not && ~midiOut.notNil } {
                "── pas de réponse — rebrancher le Teensy ou appuyer BUTTON_R long ──".postln;
                ~send_handshake.value;
                5.0.wait;
            };
        }.fork;
        "── ETEXTILE_SYNTH trouvé ──".postln;
        true
    } {
        MIDIClient.init;
        false
    }
};

~midi_poll = Routine({
    while { ~midiOut.isNil } {
        if (~midi_connect.value.not) { 1.0.wait };
    };
}).play;

// ── Boot serveur audio ────────────────────────────────────────────────────────
s.waitForBoot {

    // CC 1-6 contrôlés par les 2 touches · reste fixe
    ~buses = IdentityDictionary[
        1 -> Bus.control(s),   // T1 X: filter cutoff
        2 -> Bus.control(s),   // T1 Y: pitch ratio
        3 -> Bus.control(s),   // T1 Z: amplitude
        4 -> Bus.control(s),   // T2 X: reverb room
        5 -> Bus.control(s),   // T2 Y: reverb mix
        6 -> Bus.control(s)    // T2 Z: dry/wet
    ];

    ~specs = IdentityDictionary[
        1 -> [ControlSpec(200,  8000, \exp),  4000.0],  // T1 X: cutoff
        2 -> [ControlSpec(0.5,  2.0,  \exp),  1.0   ],  // T1 Y: pitch
        3 -> [ControlSpec(0.0,  1.0,  \lin),  0.7   ],  // T1 Z: amp
        4 -> [ControlSpec(0.0,  1.0,  \lin),  0.5   ],  // T2 X: room
        5 -> [ControlSpec(0.0,  1.0,  \lin),  0.3   ],  // T2 Y: revMix
        6 -> [ControlSpec(0.0,  1.0,  \lin),  1.0   ]   // T2 Z: dry/wet
    ];

    ~specs.keysValuesDo { |cc, pair| ~buses[cc].set(pair[1]) };

    SynthDef(\e256_live, {
        |cutoffBus, pitchBus, ampBus, roomBus, reverbMixBus, mixBus|

        var in       = SoundIn.ar(0);
        // paramètres fixes (pas de capteur)
        var winSize  = 0.1;
        var damp     = 0.5;
        // paramètres capteur
        var cutoff   = Lag.kr(In.kr(cutoffBus),    0.05).clip(20, 20000);
        var pitch    = Lag.kr(In.kr(pitchBus),     0.05);
        var amp      = Lag.kr(In.kr(ampBus),       0.05);
        var room     = Lag.kr(In.kr(roomBus),      0.1);
        var revMix   = Lag.kr(In.kr(reverbMixBus), 0.1);
        var mix      = Lag.kr(In.kr(mixBus),       0.05);

        var shifted  = PitchShift.ar(in, winSize, pitch, 0, 0);
        var filtered = RLPF.ar(shifted, cutoff, 1.0);
        var reverbed = FreeVerb.ar(filtered, revMix, room, damp);
        var wet      = Pan2.ar(reverbed * amp);
        var dry      = Pan2.ar(in);

        Out.ar(0, XFade2.ar(dry, wet, mix * 2 - 1));
    }).add;

    s.sync;

    ~live = Synth(\e256_live, [
        \cutoffBus,    ~buses[1].index,
        \pitchBus,     ~buses[2].index,
        \ampBus,       ~buses[3].index,
        \roomBus,      ~buses[4].index,
        \reverbMixBus, ~buses[5].index,
        \mixBus,       ~buses[6].index
    ]);

    MIDIdef.cc(\e256_live, { |val, num|
        var pair = ~specs[num];
        if (~handshake_done.not) {
            ~handshake_done = true;
            "── USB_INTERFACE_MODE confirmed (CC reçu) ──".postln;
        };
        if (pair.notNil) { ~buses[num].set(pair[0].map(val / 127.0)) };
    });

    CmdPeriod.add({
        ~handshake_done = false;
        ~amidi_port     = nil;
        ~midi_poll.stop;
        ~live.free;
        ~buses.do { |b| b.free };
        MIDIdef(\e256_live).free;
        MIDIdef(\e256_sysex).free;
        if (~midiOut.notNil) { ~midiOut.free; ~midiOut = nil };
    });

    "── e256 live effect ready ──".postln;
};

ServerQuit.add({ 0.exit });
