#pragma once

#include "ofxTeensyOcto.h"
#include "ofxOsc.h"
#include "ofMain.h"
#include "ofxTiming.h"
#include "fish.h"

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
        void playSplash();
        void handleLocation(ofVec2f location, bool isNear);
        ofRectangle getPreviewBounds();

        void dubStepEvent(ofVec2f loc, bool isNear);

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
    
    // Ripple Shader stuff
    ofShader shader;
    ofFbo texture1;
    ofFbo texture2;
    ofFbo texture3;
    ofFbo fishTexture;
    bool even;
    float damping;
    
    //game stuff
    int mode;
    int game;
    ofVec2f lastNearSideLocation;
    bool gotNearSideLocation;
    ofVec2f lastFarSideLocation;
    bool gotFarSideLocation;
    ofVec2f lastLocation;
    bool gotLocation;
    bool nearSideBonk;
    bool farSideBonk;
    
    //osc stuff
    ofxOscReceiver receiver;
    ofxOscReceiver receiver2;
    ofxOscSender sender;

    //sensor serial
    ofSerial sensorSerial;
    bool sensorSerialAvailable;
    std::string sensorSerialLine;
    
    //physical table stuff
    float tableWidth;
    float tableLength;

    //sound stuff
    ofDirectory dir;
    vector<ofSoundPlayer> splashes;

    //fish stuff
    Fish fish1;
    Fish fish2;

    //dub step mode stuff
    ofFbo texture4;

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
    vector<ofSoundPlayer> beeps;

    ofDirectory boopDir;
    vector<ofSoundPlayer> boops;

    ofDirectory swishBeepDir;
    vector<ofSoundPlayer> swishBeeps;

    ofDirectory swishBoopDir;
    vector<ofSoundPlayer> swishBoops;

    //rainbow
    ofFbo texture5;
    uint64_t timeLastBonk;
};
