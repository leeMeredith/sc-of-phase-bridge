#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

    ofSetWindowTitle("SC <-> OF Phase Bridge");
    ofSetFrameRate(60);
    ofSetBackgroundColor(14, 14, 16);
    ofSetCircleResolution(64);

    phaseReceiver.setup(9001);          // <- SC sends /phase and /beat here
    sender.setup("127.0.0.1", 57120);   // -> SC listens for /beat and /tempo

    for (int i = 0; i < STEPS; i++) { armed[i] = false; flash[i] = 0.0f; }

    layout();
}

//--------------------------------------------------------------
void ofApp::layout() {

    trackW = ofGetWidth() * 0.86f;
    trackX = (ofGetWidth() - trackW) * 0.5f;
    trackY = ofGetHeight() * 0.5f;
    trackH = ofClamp(ofGetHeight() * 0.16f, 40.0f, 160.0f);
    cellW  = trackW / (float)STEPS;
}

//--------------------------------------------------------------
void ofApp::update() {

    phaseReceiver.update();
    phase = phaseReceiver.getPhase();

    // A beat echo from SC lights the cell it sounded. The flash is driven by
    // the round-trip, not our own click, so any drift shows as the flash
    // lagging the playhead.
    int beat = phaseReceiver.consumeBeat();
    if (beat >= 0 && beat < STEPS) flash[beat] = 1.0f;

    // decay flashes
    for (int i = 0; i < STEPS; i++) {
        flash[i] *= 0.86f;
        if (flash[i] < 0.001f) flash[i] = 0.0f;
    }
}

//--------------------------------------------------------------
void ofApp::draw() {

    float accentR = 90,  accentG = 220, accentB = 255;   // one cool accent
    float playX   = trackX + phase * trackW;
    float top     = trackY - trackH * 0.5f;
    float bot     = trackY + trackH * 0.5f;

    // step cells: dim resting, warm when armed, blazing on flash
    for (int i = 0; i < STEPS; i++) {

        float cx = trackX + i * cellW;
        float pad = cellW * 0.08f;

        float base = armed[i] ? 46.0f : 24.0f;
        float f    = flash[i];
        float r = base + f * (accentR - base);
        float g = base + f * (accentG - base);
        float b = base + f * (accentB - base) + (armed[i] ? 14.0f : 0.0f);

        ofSetColor(r, g, b);
        ofDrawRectangle(cx + pad, top, cellW - pad * 2.0f, trackH);

        // expanding ring on a fresh flash — brief, one per round-trip
        if (f > 0.15f) {
            float ringR = (1.0f - f) * cellW * 1.4f;
            ofNoFill();
            ofSetColor(accentR, accentG, accentB, f * 180.0f);
            ofSetLineWidth(2.0f);
            ofDrawCircle(cx + cellW * 0.5f, trackY, ringR);
            ofFill();
            ofSetLineWidth(1.0f);
        }
    }

    // cell dividers, very faint
    ofSetColor(0, 0, 0, 120);
    for (int i = 0; i <= STEPS; i++) {
        float x = trackX + i * cellW;
        ofDrawLine(x, top, x, bot);
    }

    // playhead trail — a short fading tail so sweep speed is legible
    int trailN = 18;
    for (int k = trailN; k > 0; k--) {
        float tp = phase - (k * 0.006f);
        if (tp < 0.0f) continue;
        float tx = trackX + tp * trackW;
        float a  = (1.0f - (k / (float)trailN)) * 90.0f;
        ofSetColor(accentR, accentG, accentB, a);
        ofDrawRectangle(tx - 1.0f, top, 2.0f, trackH);
    }

    // playhead — the clock made visible; its motion IS SC's phase
    ofSetColor(accentR, accentG, accentB);
    ofDrawRectangle(playX - 1.5f, top - 10, 3.0f, trackH + 20);
    ofDrawCircle(playX, top - 16, 4.0f);

    // labels
    ofSetColor(150, 150, 156);
    ofDrawBitmapString("click a step to arm it   |   up/down: tempo   |   c: clear",
                       trackX, bot + 28);
    ofDrawBitmapString("phase " + ofToString(phase, 3) +
                       "     barDur " + ofToString(barDur, 2) + "s",
                       trackX, top - 30);
}

//--------------------------------------------------------------
int ofApp::cellAt(int x, int y) const {

    float top = trackY - trackH * 0.5f;
    float bot = trackY + trackH * 0.5f;
    if (y < top || y > bot)            return -1;
    if (x < trackX || x > trackX + trackW) return -1;

    int i = (int)floor((x - trackX) / cellW);
    if (i < 0 || i >= STEPS) return -1;
    return i;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    int i = cellAt(x, y);
    if (i < 0) return;

    // Toggle our local mirror and tell SC. SC is the source of truth for the
    // arm state and will confirm by echoing beats — but mirroring locally
    // keeps the resting cell colour responsive to the click.
    armed[i] = !armed[i];

    ofxOscMessage m;
    m.setAddress("/beat");
    m.addIntArg(i);
    sender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

    bool tempoChanged = false;

    if (key == OF_KEY_UP)   { barDur = ofClamp(barDur - 0.25f, 0.25f, 30.0f); tempoChanged = true; }
    if (key == OF_KEY_DOWN) { barDur = ofClamp(barDur + 0.25f, 0.25f, 30.0f); tempoChanged = true; }

    if (tempoChanged) {
        ofxOscMessage m;
        m.setAddress("/tempo");
        m.addFloatArg(barDur);
        sender.sendMessage(m, false);   // SC applies it; the clock never forks
    }

    if (key == 'c') {
        for (int i = 0; i < STEPS; i++) armed[i] = false;
        ofxOscMessage m;
        m.setAddress("/clear");
        sender.sendMessage(m, false);   // SC disarms too, both sides agree
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) { layout(); }

//--------------------------------------------------------------
void ofApp::exit() {}
