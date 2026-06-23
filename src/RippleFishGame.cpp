#include "RippleFishGame.h"

std::string RippleFishGame::name() const {
    return "Ripple Fish";
}

void RippleFishGame::setup(ofVec2f size) {
    canvasSize = size;
    lastStrikeLocation = canvasSize / 2.0;

    // Ripple shader ping-pongs between texture1 and texture2.
#ifdef TARGET_OSX
    shader.setupShaderFromFile(GL_VERTEX_SHADER, "shader.vert");
    shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shader.frag");
    shader.bindDefaults();
    shader.linkProgram();
#else
    shader.load("shader");
#endif

    texture1.allocate(canvasSize.x, canvasSize.y);
    texture2.allocate(canvasSize.x, canvasSize.y);
    texture3.allocate(canvasSize.x, canvasSize.y);
    fishTexture.allocate(canvasSize.x, canvasSize.y);

    texture1.begin();
    ofClear(0, 0, 0, 0);
    texture1.end();
    texture2.begin();
    ofClear(0, 0, 0, 0);
    texture2.end();
    texture3.begin();
    ofClear(0, 0, 0, 0);
    texture3.end();
    fishTexture.begin();
    ofClear(0, 0, 0, 0);
    fishTexture.end();

    damping = 0.83;
    even = true;

    splashDir.listDir("sounds/splashes/");
    splashDir.allowExt("wav");
    splashDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    for (int i = 0; i < (int)splashDir.size(); i++) {
        ofSoundPlayer sound = ofSoundPlayer();
        sound.load(splashDir.getPath(i));
        splashes.push_back(sound);
        cout << "loaded sound " << splashDir.getPath(i) << endl;
    }

    fish1.setup(lastStrikeLocation, ofColor(0, 255, 0), 0.05);
    fish2.setup(lastStrikeLocation, ofColor(0, 0, 255), 0.05);
}

void RippleFishGame::update() {
    if (even) {
        drawStrikes(texture1);

        texture3.begin();
        shader.begin();
        shader.setUniformTexture("backbuffer", texture2.getTextureReference(), 0);
        shader.setUniformTexture("tex0", texture1.getTextureReference(), 1);
        shader.setUniform1f("damping", (float)damping);
        shader.setUniform2f("resolution", canvasSize.x, canvasSize.y);
        ofSetColor(255, 255);
        ofRect(0, 0, canvasSize.x, canvasSize.y);
        shader.end();
        texture3.end();

        texture2.begin();
        texture3.draw(0, 0);
        texture2.end();
        even = false;
    } else {
        drawStrikes(texture2);

        texture3.begin();
        shader.begin();
        shader.setUniformTexture("backbuffer", texture1.getTextureReference(), 0);
        shader.setUniformTexture("tex0", texture2.getTextureReference(), 1);
        shader.setUniform1f("damping", (float)damping);
        shader.setUniform2f("resolution", canvasSize.x, canvasSize.y);
        ofSetColor(255, 255);
        ofRect(0, 0, canvasSize.x, canvasSize.y);
        shader.end();
        texture3.end();

        texture1.begin();
        texture3.draw(0, 0);
        texture1.end();
        even = true;
    }

    strikes.clear();

    fish1.update(&fish2);
    fish2.update(&fish1);
    fishTexture.begin();
    ofClear(0, 0, 0, 0);
    fish1.draw();
    fish2.draw();
    fishTexture.end();
}

void RippleFishGame::draw(ofFbo& output, int brightness) {
    output.begin();
    ofClear(0, 0, 0);
    ofSetColor(brightness);
    ofPushStyle();
    ofEnableAlphaBlending();
    if (even) {
        texture1.draw(0, 0);
    } else {
        texture2.draw(0, 0);
    }
    fishTexture.draw(0, 0);
    ofPopStyle();
    output.end();
}

void RippleFishGame::handleStrike(ofVec2f location, bool isNearSide) {
    lastStrikeLocation = location;
    strikes.push_back({location, isNearSide});
}

void RippleFishGame::drawStrikes(ofFbo& texture) {
    ofPushStyle();
    ofPushMatrix();
    texture.begin();

    for (int i = 0; i < (int)strikes.size(); i++) {
        const Strike& strike = strikes[i];
        // Near and far side strikes use the original blue/green color coding.
        ofSetColor(ofNoise(ofGetFrameNum()) * 255 * 5, 255);
        ofFill();
        ofDrawEllipse(strike.location.x, strike.location.y, 12, 12);
        if (strike.isNearSide) {
            ofSetColor(0, 0, 255, 255);
        } else {
            ofSetColor(0, 255, 0, 255);
        }
        ofFill();
        ofDrawEllipse(strike.location.x, strike.location.y, 10, 10);
        playSplash(strike.location);
    }

    texture.end();
    ofPopMatrix();
    ofPopStyle();
}

void RippleFishGame::playSplash(ofVec2f location) {
    int i = ofRandom(0, splashes.size() - 1);
    cout << i << endl;
    splashes[i].setPan(ofMap(location.y, 0.0, canvasSize.y, -1.0, 1.0, true));
    splashes[i].setVolume(1.0);
    splashes[i].play();
}
