#pragma once

#include "TableGame.h"
#include "fish.h"

class RippleFishGame : public TableGame {
public:
    std::string name() const override;
    void setup(ofVec2f canvasSize) override;
    void update() override;
    void draw(ofFbo& output, int brightness) override;
    void handleStrike(ofVec2f location, bool isNearSide) override;

private:
    struct Strike {
        ofVec2f location;
        bool isNearSide;
    };

    void drawStrikes(ofFbo& texture);
    void playSplash(ofVec2f location);

    ofVec2f canvasSize;
    ofVec2f lastStrikeLocation;
    std::vector<Strike> strikes;

    ofShader shader;
    ofFbo texture1;
    ofFbo texture2;
    ofFbo texture3;
    ofFbo fishTexture;
    bool even;
    float damping;

    ofDirectory splashDir;
    std::vector<ofSoundPlayer> splashes;

    Fish fish1;
    Fish fish2;
};
