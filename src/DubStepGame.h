#pragma once

#include "TableGame.h"
#include "ofxTiming.h"

class DubStepGame : public TableGame {
public:
    std::string name() const override;
    void setup(ofVec2f canvasSize) override;
    void update() override;
    void draw(ofFbo& output, int brightness) override;
    void handleStrike(ofVec2f location, bool isNearSide) override;

private:
    void dubStepEvent(ofVec2f location, bool isNearSide);

    ofVec2f canvasSize;
    ofVec2f lastStrikeLocation;
    ofFbo texture;

    int dubStepState;
    ofColor backgroundCol;
    ofColor newCol;
    ofColor newNewCol;
    ofVec2f loc1;
    ofVec2f loc2;
    ofVec2f loc3;
    LerpTimer lerpTimer1;
    LerpTimer lerpTimer2;
    LerpTimer lerpTimer3;

    ofDirectory beepDir;
    std::vector<ofSoundPlayer> beeps;

    ofDirectory boopDir;
    std::vector<ofSoundPlayer> boops;

    ofDirectory swishBeepDir;
    std::vector<ofSoundPlayer> swishBeeps;

    ofDirectory swishBoopDir;
    std::vector<ofSoundPlayer> swishBoops;
};
