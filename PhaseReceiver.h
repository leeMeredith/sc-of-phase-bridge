#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

// PhaseReceiver
// -------------
// Listens on one port for the two messages SuperCollider sends:
//
//   /phase <float 0..1>   where SC is in the current bar
//   /beat  <int step>     echo of a step SC just sounded (drives the flash)
//
// It holds no clock of its own. phase is simply the last value SC reported.
// beatStep is set for one poll when an echo arrives, then cleared by
// consumeBeat(), so the app flashes a cell exactly once per round-trip.

class PhaseReceiver {
public:

    void setup(int port);
    void update();

    float getPhase() const { return phase; }

    // Returns the step index of a beat echo since the last call, or -1 if none.
    // Clears the pending beat so each echo is consumed once.
    int consumeBeat();

private:

    ofxOscReceiver receiver;
    int   port     = 0;

    float phase    = 0.0f;
    int   beatStep = -1;   // -1 = nothing pending
};
