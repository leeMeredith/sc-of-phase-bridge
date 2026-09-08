# SC ↔ OF Phase Bridge

A minimal, honest example of getting **SuperCollider** and **openFrameworks**
to share one clock over OSC — no middleman, no glue framework, nothing hidden.

SuperCollider owns time. openFrameworks owns the screen and the interaction.
They agree on exactly two messages. That's the whole thing.

![demo](docs/demo.gif)

## Why this exists

Most people wiring SC to OF end up with one of two messes: an extra program
wedged in the middle as an invisible clock source, or two programs each running
their own clock and slowly drifting apart. Both hide the one question that
matters — *are these two actually in sync?*

This repo answers that question visibly. SuperCollider sends a single phase
ramp (0..1, once per bar). openFrameworks receives it and subdivides it into a
step grid at its own frame rate. You click a step to arm it; from then on it
sounds once per bar, the instant SC's phase sweeps across it. The flash you see
is triggered by the beat coming *back* from SuperCollider — so if the two ever
drift, the light lags the playhead and you can see it. When it's tight, sound
and light land on the same frame.

Nothing on screen moves unless a real message moved it.

## The contract

```
SuperCollider ──/phase <float 0..1>──────────► openFrameworks  :9001
openFrameworks ──/beat  <int step>───────────► SuperCollider   :57120
openFrameworks ──/tempo <float barDur>───────► SuperCollider   :57120   (optional)
openFrameworks ──/clear ─────────────────────► SuperCollider   :57120   (disarm all)
```

- **/phase** — one continuous ramp per bar. SC has no opinion about beats,
  steps, or meter; it just reports where it is in the bar.
- **/beat** — openFrameworks arms or disarms a step. It declares *which* cell
  is active, never *when* it should sound. SC decides when, as the phase crosses.
- **/tempo** — keys in OF nudge the bar duration, but the change is applied in
  SC so the clock never forks.

The design rule behind all of it: **openFrameworks sends facts, not timing
decisions.** Timing authority stays in one place — SuperCollider — which is
what keeps the two in sync without any negotiation.

## Run it

1. Open `supercollider/bridge.scd`, select the top block, and evaluate it.
2. Wait for `READY` in the post window.
3. Build and run the openFrameworks app in `openframeworks/`.
4. Click a step cell to arm it.

Controls in the openFrameworks window:

- **click** a cell — arm / disarm that step
- **up / down** — faster / slower (sends `/tempo`, applied in SC)
- **c** — clear all steps

The step count is `16` in both files — `~steps` in `bridge.scd` and `STEPS`
in `ofApp.h`. Change both together if you want a different grid.

## What this is not

This is a bridge, not an instrument. One click synth, one ramp, one grid.
It's meant to be the thing you paste into a forum thread when someone asks
"how do I get these two talking properly?" — small enough to read in full,
correct enough to build on.

## License

MIT.
