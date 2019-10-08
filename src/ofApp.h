#pragma once

#include "ofxTeensyOcto.h"
#include "ofxOsc.h"
#include "ofMain.h"

#define PORT 6666
#define OUTPORT 7777
#define HOST "127.0.0.1"

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
    bool even;
    float damping;
    
    //game stuff
    int mode;
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
    ofxOscSender sender;
    
    //physical table stuff
    float tableWidth;
    float tableLength;
};
