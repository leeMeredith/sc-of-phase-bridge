#include "PhaseReceiver.h"

//--------------------------------------------------------------
void PhaseReceiver::setup(int port_) {

    port = port_;
    receiver.setup(port);

    phase    = 0.0f;
    beatStep = -1;

    ofLogNotice() << "[PhaseReceiver] listening on :" << port;
}

//--------------------------------------------------------------
void PhaseReceiver::update() {

    while (receiver.hasWaitingMessages()) {

        ofxOscMessage msg;
        receiver.getNextMessage(msg);

        // /phase <float> — where SC is in the bar. This is the only clock.
        if (msg.getAddress() == "/phase" &&
            msg.getNumArgs() > 0 &&
            msg.getArgType(0) == OFXOSC_TYPE_FLOAT) {

            phase = msg.getArgAsFloat(0);
            continue;
        }

        // /beat <int step> — SC just sounded this step; remember it so the
        // app can flash the cell. Overwrites any unconsumed beat, which is
        // fine: at sane tempos the app polls faster than beats arrive.
        if (msg.getAddress() == "/beat" &&
            msg.getNumArgs() > 0 &&
            msg.getArgType(0) == OFXOSC_TYPE_INT32) {

            beatStep = msg.getArgAsInt32(0);
            continue;
        }
    }
}

//--------------------------------------------------------------
int PhaseReceiver::consumeBeat() {

    int s = beatStep;
    beatStep = -1;
    return s;
}
