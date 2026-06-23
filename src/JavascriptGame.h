#pragma once

#include "TableGame.h"
#include "../libs/quickjs/quickjs.h"

class JavascriptGame : public TableGame {
public:
    explicit JavascriptGame(std::string codePath);
    ~JavascriptGame() override;

    std::string name() const override;
    void setup(ofVec2f canvasSize) override;
    void update() override;
    void draw(ofFbo& output, int brightness) override;
    void handleStrike(ofVec2f location, bool isNearSide) override;

    void setFillColor(ofColor color);
    void disableFill();
    void setStrokeColor(ofColor color);
    void disableStroke();
    void setStrokeWidth(float width);
    void drawCircle(float x, float y, float radius);
    void drawRect(float x, float y, float width, float height);
    void drawLine(float x1, float y1, float x2, float y2);
    void drawPoint(float x, float y);

private:
    std::string codePath;
    std::string gameName;
    ofVec2f canvasSize;

    bool fillEnabled;
    bool strokeEnabled;
    float currentStrokeWidth;
    ofColor fillColor;
    ofColor strokeColor;

    JSRuntime* runtime;
    JSContext* context;
    JSValue graphicsObject;
};
