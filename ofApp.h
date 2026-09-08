#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "PhaseReceiver.h"

// ofApp
// -----
// The screen and the interaction. Receives SC's phase, subdivides it into a
// step grid, and lets you click a cell to arm it. Arming sends /beat to SC;
// SC decides when that armed cell actually sounds, and echoes /beat back,
// which is what triggers the on-screen flash.
//
//   SC ──/phase──► ofApp        (drives the playhead)
//   ofApp ──/beat──► SC          (arm/disarm a step, on click)
//   ofApp ──/tempo──► SC         (arrow keys)
//   SC ──/beat──► ofApp          (echo of a sounded step, drives the flash)

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;

    void keyPressed(int key) override;
    void mousePressed(int x, int y, int button) override;
    void windowResized(int w, int h) override;

    // no-ops kept for the ofBaseApp interface
    void keyReleased(int key) override {}
    void mouseMoved(int x, int y) override {}
    void mouseDragged(int x, int y, int button) override {}
    void mouseReleased(int x, int y, int button) override {}
    void mouseScrolled(int x, int y, float sx, float sy) override {}
    void mouseEntered(int x, int y) override {}
    void mouseExited(int x, int y) override {}
    void dragEvent(ofDragInfo d) override {}
    void gotMessage(ofMessage m) override {}

private:

    static const int STEPS = 16;   // must match ~steps in bridge.scd

    PhaseReceiver   phaseReceiver;
    ofxOscSender    sender;

    float phase = 0.0f;

    bool  armed[STEPS];            // which steps are active (mirror of SC)
    float flash[STEPS];            // 0..1 flash brightness per step, decays each frame

    float barDur = 2.0f;           // local mirror, only for the /tempo message

    // geometry, recomputed on resize
    float trackX = 0, trackY = 0, trackW = 0, cellW = 0;
    float trackH = 0;
    void  layout();

    int   cellAt(int x, int y) const;   // which step a pixel falls in, or -1
};
