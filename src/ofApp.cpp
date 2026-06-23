#include "ofApp.h"
#include "DubStepGame.h"
#include "RippleFishGame.h"
#include "multilateration.h"

namespace {
    const float TABLE_WIDTH = 1.584f;
    const float TABLE_LENGTH = 3.24f;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);                      // default background to black / LEDs off
    ofDisableAntiAliasing();                    // we need our graphics sharp for the LEDs
#ifdef TARGET_OSX
    ofDisableArbTex();
#endif
    //ofSetVerticalSync(false);
    //ofSetFrameRate(90);
    
    // SYSTEM SETTINGS
    //--------------------------------------
    stripWidth = 44;                            // pixel width of strip
    stripHeight = 3;                            // pixel height of strip
    stripsPerPort = 8;                          // total number of strips per port
    numPorts = 4;                               // total number of teensy ports?
    brightness = 170;                           // LED brightness

    // setup our teensys
    teensy.setup(stripWidth, stripHeight, 1, stripsPerPort, numPorts);

    /* Configure our teensy boards (portName, xOffset, yOffset, width%, height%, direction) */
    teensy.serialConfigure("TEENSY4", 0, 0, 100, 25, 0);
    teensy.serialConfigure("TEENSY3", 0, 25, 100, 25, 0);
    teensy.serialConfigure("TEENSY1", 0, 50, 100, 25, 0);
    teensy.serialConfigure("TEENSY2", 0, 75, 100, 25, 0);
    sensorSerialAvailable = sensorSerial.setup("/dev/TEENSY5", 38400);
    
    // allocate the LED matrix frame buffer
    fbo.allocate(stripWidth, stripHeight * stripsPerPort * numPorts, GL_RGB);
    ofEnableAlphaBlending();

    // OSC input: receiver gets strike data, receiver2 gets external game selection
    receiver.setup(PORT);
    receiver2.setup(PORT2);
    sender.setup(HOST, OUTPORT);

    // Games render in LED matrix coordinates.
    activeGameIndex = 0;
    games.push_back(std::unique_ptr<TableGame>(new RippleFishGame()));
    games.push_back(std::unique_ptr<TableGame>(new DubStepGame()));

    for (int i = 0; i < (int)games.size(); i++) {
        games[i]->setup(ofVec2f(fbo.getWidth(), fbo.getHeight()));
    }
    activeGame()->activate();

    // Rainbow is the idle/saver mode, not part of the game cycle.
    texture5.allocate(fbo.getWidth(), fbo.getHeight());
    texture5.begin();
    ofClear(0, 0, 0, 0);
    texture5.end();
    timeLastBonk = 0;
}

void ofApp::exit(){
    /* turn all leds to black */
    fbo.begin();
    ofClear(0,0,0);
    fbo.end();
    fbo.readToPixels(teensy.pixels1);
    teensy.update();
}

//--------------------------------------------------------------
void ofApp::update(){
    updateSensorSerial();
    
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if(m.getAddress() == "/nearside/bonk"){
            cout << "got near side bonk" << endl;
        } else if(m.getAddress() == "/nearside/location") {
            handleLocation(ofVec2f(m.getArgAsFloat(0), m.getArgAsFloat(1)), true);
            cout << "got near side location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        } else if(m.getAddress() == "/farside/bonk") {
            cout << "got far side bonk" << endl;
        } else if(m.getAddress() == "/farside/location") {
            handleLocation(ofVec2f(m.getArgAsFloat(0), m.getArgAsFloat(1)), false);
            cout << "got far side location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        }  else if(m.getAddress() == "/location") {
            ofVec2f location(m.getArgAsFloat(0), m.getArgAsFloat(1));
            handleLocation(location, location.y >= TABLE_LENGTH / 2.0f);
            cout << "got location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        }
    }

    while(receiver2.hasWaitingMessages()){
        ofxOscMessage m;
        receiver2.getNextMessage(m);
        if(m.getAddress() == "/video") {
            setActiveGame(m.getArgAsInt(0));
            cout << "got /video = " << m.getArgAsInt(0) << endl;
        }
    }

    if (isIdle()) {
        updateRainbow();
    } else {
        activeGame()->update();
    }
    
    updateFbo();
    teensy.update();
}

//--------------------------------------------------------------
void ofApp::updateFbo(){
    if (isIdle()) {
        fbo.begin();
        ofClear(0,0,0);
        ofSetColor(brightness);
        ofPushStyle();
        texture5.draw(0,0);
        ofPopStyle();
        fbo.end();
    } else {
        activeGame()->draw(fbo, brightness);
    }
    fbo.readToPixels(teensy.pixels1);
}

void ofApp::updateRainbow(){
    ofPushStyle();
    ofPushMatrix();
    texture5.begin();
    for (int i = 0; i < 10; i++) {
        ofColor c = ofColor::fromHsb(static_cast<int>(ofGetFrameNum() + ofMap(i, 0, 10, 0, 255)) % 255, 255, 255);
        ofSetColor(c);
        ofFill();
        ofDrawRectangle(0.0, ((i * fbo.getHeight() / 10)), fbo.getWidth(), (fbo.getHeight()/10)+1);
    }
    texture5.end();
    ofPopMatrix();
    ofPopStyle();
}

void ofApp::updateSensorSerial(){
    if (!sensorSerialAvailable) {
        return;
    }

    while (sensorSerial.available() > 0) {
        int byte = sensorSerial.readByte();
        if (byte == OF_SERIAL_ERROR) {
            ofLogError("sensor serial") << "failed to read sensor serial port";
            sensorSerialAvailable = false;
            return;
        }
        if (byte == OF_SERIAL_NO_DATA) {
            return;
        }
        if (byte == '\n') {
            std::string line = sensorSerialLine;
            sensorSerialLine.clear();
            try {
                MultilaterationResult result = calculateSerialLine(line);
                handleLocation(ofVec2f(result.x, result.y), result.side == TableSide::Near);
                cout << "got serial location x = " << result.x << " y = " << result.y
                     << " valid = " << result.valid << endl;
            } catch (const std::exception& error) {
                ofLogWarning("sensor serial") << "discarding line '" << line << "': " << error.what();
            }
        } else if (byte != '\r') {
            sensorSerialLine += static_cast<char>(byte);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
#ifdef TARGET_OSX
    ofSetColor(255);
    ofRectangle previewBounds = getPreviewBounds();
    fbo.draw(previewBounds);
#endif
}

ofRectangle ofApp::getPreviewBounds(){
    float scale = std::min(ofGetWidth() / fbo.getWidth(), ofGetHeight() / fbo.getHeight());
    float width = fbo.getWidth() * scale;
    float height = fbo.getHeight() * scale;
    return ofRectangle((ofGetWidth() - width) / 2, (ofGetHeight() - height) / 2, width, height);
}

void ofApp::handleLocation(ofVec2f location, bool isNear){
    // Sensor locations arrive in physical table coordinates.
    ofVec2f matrixLocation(
        ofMap(location.x, 0.0, TABLE_WIDTH, 0.0, fbo.getWidth()),
        ofMap(location.y, 0.0, TABLE_LENGTH, 0.0, fbo.getHeight())
    );
    timeLastBonk = ofGetSystemTimeMillis();
    activeGame()->handleStrike(matrixLocation, isNear);
}

TableGame* ofApp::activeGame(){
    return games[activeGameIndex].get();
}

void ofApp::setActiveGame(int index){
    int gameCount = games.size();
    index = ((index % gameCount) + gameCount) % gameCount;
    if (index == activeGameIndex) {
        return;
    }
    activeGame()->deactivate();
    activeGameIndex = index;
    activeGame()->activate();
    cout << "setting game = " << activeGameIndex << " (" << activeGame()->name() << ")" << endl;
}

void ofApp::nextGame(){
    setActiveGame(activeGameIndex + 1);
}

bool ofApp::isIdle() const {
    return ofGetSystemTimeMillis() > timeLastBonk + TIME_BEFORE_RAINBOW;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    ofLogVerbose(__func__) << "key: " << key;
    switch (key) 
    {
        case 'g':
        {
            nextGame();
            break;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
#ifdef TARGET_OSX
    ofRectangle previewBounds = getPreviewBounds();
    if (!previewBounds.inside(x, y)) {
        return;
    }
    ofVec2f location(
        ofMap(x, previewBounds.getLeft(), previewBounds.getRight(), 0, fbo.getWidth(), true),
        ofMap(y, previewBounds.getTop(), previewBounds.getBottom(), 0, fbo.getHeight(), true)
    );
    timeLastBonk = ofGetSystemTimeMillis();
    activeGame()->handleStrike(location, location.y >= fbo.getHeight() / 2.0);
#endif
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
