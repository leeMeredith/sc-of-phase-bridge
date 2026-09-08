// bridge.scd
// SC ↔ OF Phase Bridge — SuperCollider side.
//
// SuperCollider owns the clock. It sends one phase ramp (0..1) per bar to
// openFrameworks, and listens for two things back: which steps are armed,
// and (optionally) a new bar duration. It has no opinion about beats or
// meter — it only reports where it is in the bar, and sounds an armed step
// the instant the phase crosses it.
//
// ─────────────────────────────────────────────────────────────────────────────
// CONTRACT
//
//   SC ──/phase <float 0..1>──────────► OF  :9001
//   OF ──/beat  <int step>────────────► SC  :57120   toggle-arm a step
//   OF ──/tempo <float barDur>────────► SC  :57120   optional, from OF keys
//   OF ──/clear ──────────────────────► SC  :57120   disarm all steps
//
// SC echoes each sounded beat back to OF as /beat <step>, so OF can flash the
// cell from the round-trip rather than from its own click. That echo is what
// makes drift visible.
//
// ─────────────────────────────────────────────────────────────────────────────
// HOW TO RUN
//
//   1. Click inside the big block below, cmd-Enter (evaluates as one unit).
//   2. Wait for "READY" in the post window.
//   3. Build and run the openFrameworks app.
//   4. Click a step in OF to arm it. Arrow keys change tempo.
//
// ─────────────────────────────────────────────────────────────────────────────

(
s.waitForBoot({

    // ── config ───────────────────────────────────────────────────────────────

    ~ofAddr    = NetAddr("127.0.0.1", 9001);   // OF's phase receiver
    ~steps     = 16;                           // subdivisions per bar (OF matches this)
    ~barDur    = 2.0;                          // seconds per bar — authority lives here
    ~clockRate = 100;                          // phase messages per second
    ~clockGen  = (~clockGen ? 0) + 1;          // bumps each evaluation, retires old clocks
    ~amp       = 0.4;                           // global level

    // Armed steps. index 0..~steps-1, true = sounds once per bar.
    ~armed     = Array.fill(~steps, false);

    // Which step the phase was in on the previous tick, so we fire exactly
    // once on the crossing into a new cell rather than every frame.
    ~lastStep  = -1;

    // ── voice ────────────────────────────────────────────────────────────────
    // One click. Deliberately plain — this repo is a bridge, not an instrument.

    SynthDef(\click, { |out = 0, freq = 900, amp = 0.4, pan = 0|
        var env = EnvGen.kr(Env.perc(0.001, 0.12), doneAction: 2);
        var sig = SinOsc.ar(freq) + (Ringz.ar(Impulse.ar(0), freq * 2, 0.05) * 0.4);
        Out.ar(out, Pan2.ar(sig * env * amp, pan));
    }).add;

    s.sync;

    // ── receivers ──────────────────────────────────────────────────────────────

    // Toggle-arm a step. OF sends the fact "this cell is active"; it never
    // says when to sound. That decision stays here, on the phase crossing.
    OSCdef(\beat, { |msg|
        var step = msg[1].asInteger;
        if (step >= 0 and: { step < ~steps }) {
            ~armed[step] = ~armed[step].not;
            ("beat" + (if(~armed[step], "arm", "off")) + step).postln;
        };
    }, '/beat');

    // Optional tempo change from OF keys. Applied here so the clock never forks.
    OSCdef(\tempo, { |msg|
        ~barDur = msg[1].asFloat.clip(0.1, 30.0);
        ("tempo barDur" + ~barDur.round(0.01)).postln;
    }, '/tempo');

    // Disarm all steps (OF 'c' key). Keeps both sides in agreement.
    OSCdef(\clear, { |msg| ~clear.value }, '/clear');

    // ── clock ────────────────────────────────────────────────────────────────
    // One continuous phase ramp per bar. On each tick we also check whether the
    // phase has crossed into a new step; if that step is armed, sound it and
    // echo /beat back to OF so the on-screen flash rides the round-trip.

    if (~clock.notNil) { ~clock.stop; ~clock = nil };

    ~clock = Routine({
        var t0  = Main.elapsedTime;
        var gen = ~clockGen;
        loop {
            var phase, step;

            if (gen != ~clockGen) { nil.yield };   // a newer clock exists; retire

            phase = ((Main.elapsedTime - t0) / ~barDur).mod(1.0);
            ~ofAddr.sendMsg("/phase", phase.asFloat);

            step = (phase * ~steps).floor.asInteger.min(~steps - 1);
            if (step != ~lastStep) {
                if (~armed[step]) {
                    Synth(\click, [
                        \freq, 700 + (step * 60),
                        \amp,  ~amp,
                        \pan,  (step / (~steps - 1) * 2.0) - 1.0
                    ]);
                    ~ofAddr.sendMsg("/beat", step);   // echo for the visual flash
                };
                ~lastStep = step;
            };

            (1 / ~clockRate).wait;
        }
    });
    ~clock.play(SystemClock);

    // ── helpers ────────────────────────────────────────────────────────────────

    ~clear = { ~armed = Array.fill(~steps, false); "cleared".postln; };

    "READY — clock running, listening for /beat and /tempo on 57120".postln;
    ("steps:" + ~steps + "   barDur:" + ~barDur + "s").postln;
});
)

// ─────────────────────────────────────────────────────────────────────────────
// LIVE CONTROLS — evaluate one line at a time
// ─────────────────────────────────────────────────────────────────────────────

~barDur = 4.0;        // slower
~barDur = 1.0;        // faster
~amp    = 0.3;
~clear.value;         // disarm all steps

// stop
~clock.stop;
OSCdef(\beat).free;
OSCdef(\tempo).free;
s.freeAll;
