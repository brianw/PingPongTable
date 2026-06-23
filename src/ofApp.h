#pragma once

#include "ofxTeensyOcto.h"
#include "ofxOsc.h"
#include "ofMain.h"
#include "TableGame.h"

#include <memory>

#define PORT 6666
#define PORT2 7778
#define OUTPORT 7777
#define HOST "127.0.0.1"
#define TIME_BEFORE_RAINBOW 60*1000

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void updateFbo();
        void updateSensorSerial();
        void updateRainbow();
        void handleLocation(ofVec2f location, bool isNear);
        ofRectangle getPreviewBounds();
        TableGame* activeGame();
        void setActiveGame(int index);
        void nextGame();
        bool isIdle() const;

        // void onCharacterReceived(KeyListenerEventData& e);
        // TerminalListener consoleListener;
    
    // LED - Teensy stuff
    //-----------------------------
    ofxTeensyOcto teensy;
    int stripWidth;
    int stripHeight;
    int stripsPerPort;
    int numPorts;
    int brightness;
    
    // FBO stuff
    //-----------------------------
    ofFbo fbo;
    
    //game stuff
    std::vector<std::unique_ptr<TableGame>> games;
    int activeGameIndex;
    
    //osc stuff
    ofxOscReceiver receiver;
    ofxOscReceiver receiver2;
    ofxOscSender sender;

    //sensor serial
    ofSerial sensorSerial;
    bool sensorSerialAvailable;
    std::string sensorSerialLine;
    
    //rainbow
    ofFbo texture5;
    uint64_t timeLastBonk;
};
