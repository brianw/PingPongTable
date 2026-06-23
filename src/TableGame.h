#pragma once

#include "ofMain.h"

class TableGame {
public:
    virtual ~TableGame() = default;

    virtual std::string name() const = 0;
    virtual void setup(ofVec2f canvasSize) = 0;
    virtual void activate() {}
    virtual void deactivate() {}
    virtual void update() = 0;
    virtual void draw(ofFbo& output, int brightness) = 0;
    virtual void handleStrike(ofVec2f location, bool isNearSide) = 0;
};
