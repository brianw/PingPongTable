#include "DubStepGame.h"

std::string DubStepGame::name() const {
    return "Dub Step";
}

void DubStepGame::setup(ofVec2f size) {
    canvasSize = size;
    lastStrikeLocation = canvasSize / 2.0;

    texture.allocate(canvasSize.x, canvasSize.y);
    texture.begin();
    ofClear(0, 0, 0, 0);
    texture.end();

    // Dubstep mode starts with random colors and advances on strikes.
    dubStepState = 0;
    backgroundCol = ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
    newCol = ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
    newNewCol = ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
    loc1 = ofVec2f(canvasSize.x / 2.0, canvasSize.y / 8);
    loc2 = ofVec2f(canvasSize.x / 2.0, 7.0 * canvasSize.y / 8);
    loc3 = ofVec2f(0, 0);

    lerpTimer1.setDuration(0.2);
    lerpTimer2.setDuration(0.2);

    beepDir.listDir("sounds/beeps/");
    beepDir.allowExt("wav");
    beepDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    for (int i = 0; i < (int)beepDir.size(); i++) {
        ofSoundPlayer sound = ofSoundPlayer();
        sound.load(beepDir.getPath(i));
        beeps.push_back(sound);
        cout << "loaded sound " << beepDir.getPath(i) << endl;
    }

    boopDir.listDir("sounds/boops/");
    boopDir.allowExt("wav");
    boopDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    for (int i = 0; i < (int)boopDir.size(); i++) {
        ofSoundPlayer sound = ofSoundPlayer();
        sound.load(boopDir.getPath(i));
        boops.push_back(sound);
        cout << "loaded sound " << boopDir.getPath(i) << endl;
    }

    swishBeepDir.listDir("sounds/swish-beeps/");
    swishBeepDir.allowExt("wav");
    swishBeepDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    for (int i = 0; i < (int)swishBeepDir.size(); i++) {
        ofSoundPlayer sound = ofSoundPlayer();
        sound.load(swishBeepDir.getPath(i));
        swishBeeps.push_back(sound);
        cout << "loaded sound " << swishBeepDir.getPath(i) << endl;
    }

    swishBoopDir.listDir("sounds/swish-boops/");
    swishBoopDir.allowExt("wav");
    swishBoopDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    for (int i = 0; i < (int)swishBoopDir.size(); i++) {
        ofSoundPlayer sound = ofSoundPlayer();
        sound.load(swishBoopDir.getPath(i));
        swishBoops.push_back(sound);
        cout << "loaded sound " << swishBoopDir.getPath(i) << endl;
    }
}

void DubStepGame::update() {
    if (lerpTimer3.getDone() && (lerpTimer3.getProgress() == 1.0) && dubStepState == 2) {
        dubStepState = 0;
        cout << "going to 0" << endl;
        lerpTimer1.setToValue(0);
        lerpTimer2.setToValue(0);
        lerpTimer3.setToValue(0);
        loc3 = loc2;
        backgroundCol = newCol;
        newCol = newNewCol;
        newNewCol = ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
        ofLog() << "backgroundCol = " << backgroundCol;
        ofLog() << "newCol = " << newCol;
        ofLog() << "newNewCol = " << newNewCol;
    }

    ofPushStyle();
    ofPushMatrix();
    texture.begin();
    switch (dubStepState) {
        case 0:
            ofSetColor(backgroundCol);
            ofSetRectMode(OF_RECTMODE_CORNER);
            ofDrawRectangle(0, 0, canvasSize.x, canvasSize.y);

            ofSetColor(newCol);
            ofFill();
            ofDrawEllipse(loc2, 35.025, 35.025);
            break;
        case 1:
            ofSetColor(backgroundCol);
            ofSetRectMode(OF_RECTMODE_CORNER);
            ofDrawRectangle(0, 0, canvasSize.x, canvasSize.y);

            ofSetColor(newCol);
            ofFill();
            ofDrawEllipse(loc2, 35.025, 35.025);
            ofDrawEllipse(loc1, 10.0 + 25.0 * lerpTimer1.getProgress(), 10.0 + 25.0 * lerpTimer1.getProgress());
            break;
        case 2:
            ofSetColor(backgroundCol);
            ofFill();
            ofSetRectMode(OF_RECTMODE_CORNER);
            ofDrawRectangle(0, 0, canvasSize.x, canvasSize.y);

            ofSetColor(newCol);
            ofFill();
            ofDrawEllipse(loc1, 10.0 + 25.0 * lerpTimer1.getProgress(), 10.0 + 25.0 * lerpTimer1.getProgress());

            ofPushMatrix();
            ofTranslate(loc1);
            ofVec2f a = loc2 - loc1;
            ofRotate(PI / 2 - a.angle(ofVec2f(0.0, 1.0)));
            ofSetRectMode(OF_RECTMODE_CENTER);
            ofSetColor(newCol);
            ofFill();
            ofDrawRectangle(0.0, 0.0, ofMap(lerpTimer3.getProgress(), 0.0, 1.0, 0.0, 300.0), 2000);
            ofPopMatrix();
            ofSetColor(newNewCol);
            ofDrawEllipse(loc2, 10.0 + 25.0 * lerpTimer2.getProgress(), 10.0 + 25.0 * lerpTimer2.getProgress());

            break;
    }
    texture.end();
    ofPopMatrix();
    ofPopStyle();
}

void DubStepGame::draw(ofFbo& output, int brightness) {
    output.begin();
    ofClear(0, 0, 0);
    ofSetColor(brightness);
    ofPushStyle();
    texture.draw(0, 0);
    ofPopStyle();
    output.end();
}

void DubStepGame::handleStrike(ofVec2f location, bool isNearSide) {
    lastStrikeLocation = location;
    dubStepEvent(location, isNearSide);
}

void DubStepGame::dubStepEvent(ofVec2f location, bool isNearSide) {
    switch (dubStepState) {
        case 0:
            cout << "got event in state 0" << endl;
            loc1 = location;
            lerpTimer1.lerpToValue(25);
            dubStepState++;
            cout << "going to 1" << endl;
            if (isNearSide) {
                int i = ofRandom(0, beeps.size() - 1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
            } else {
                int i = ofRandom(0, boops.size() - 1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();
            }
            break;
        case 1:
            cout << "got event in state 1" << endl;
            loc2 = location;
            lerpTimer2.lerpToValue(25);
            lerpTimer3.lerpToValue(75);
            dubStepState++;
            cout << "going to 2" << endl;
            if (isNearSide) {
                int i = ofRandom(0, beeps.size() - 1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
                swishBeeps[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                swishBeeps[i].setVolume(1.0);
                swishBeeps[i].play();
            } else {
                int i = ofRandom(0, boops.size() - 1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();
                swishBoops[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                swishBoops[i].setVolume(1.0);
                swishBoops[i].play();
            }
            break;
        case 2:
            cout << "got event in state 2" << endl;
            loc1 = location;
            loc3 = loc2;

            lerpTimer1.setToValue(0.0);
            lerpTimer2.setToValue(0.0);
            lerpTimer3.setToValue(0.0);
            backgroundCol = newCol;
            newCol = newNewCol;
            newNewCol = ofColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
            ofLog() << "backgroundCol = " << backgroundCol;
            ofLog() << "newCol = " << newCol;
            ofLog() << "newNewCol = " << newNewCol;
            dubStepState = 1;
            cout << "going to 1" << endl;
            if (isNearSide) {
                int i = ofRandom(0, beeps.size() - 1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
            } else {
                int i = ofRandom(0, boops.size() - 1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastStrikeLocation.y, 0.0, canvasSize.y, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();
            }
            break;
    }
}
