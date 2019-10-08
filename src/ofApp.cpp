#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);                      // default background to black / LEDs off
    ofDisableAntiAliasing();                    // we need our graphics sharp for the LEDs
    //ofSetVerticalSync(false);
    //ofSetFrameRate(90);
    
    // SYSTEM SETTINGS
    //--------------------------------------
    stripWidth = 44;                            // pixel width of strip
    stripHeight = 3;                            // pixel height of strip
    stripsPerPort = 8;                          // total number of strips per port
    numPorts = 4;                               // total number of teensy ports?
    brightness = 200;                             // LED brightness
    
    // setup our teensys
    teensy.setup(stripWidth, stripHeight, 1, stripsPerPort, numPorts);
    
    /* Configure our teensy boards (portName, xOffset, yOffset, width%, height%, direction) */
    teensy.serialConfigure("TEENSY4", 0, 0, 100, 25, 0);
    teensy.serialConfigure("TEENSY3", 0, 25, 100, 25, 0);
    teensy.serialConfigure("TEENSY1", 0, 50, 100, 25, 0);
    teensy.serialConfigure("TEENSY2", 0, 75, 100, 25, 0);
//    teensy.serialConfigure("ttyACM3", 0, 0, 100, 25, 0);
//    teensy.serialConfigure("ttyACM4", 0, 25, 100, 25, 0);
//    teensy.serialConfigure("ttyACM2", 0, 50, 100, 25, 0);
//    teensy.serialConfigure("ttyACM1", 0, 75, 100, 25, 0);
    
    // allocate our pixels, fbo, and texture
    fbo.allocate(stripWidth, stripHeight*stripsPerPort*numPorts, GL_RGB);
    
    //set up ripple stuff
    shader.load("shader");
    texture1.allocate(ofGetWidth(), ofGetHeight());
    texture2.allocate(ofGetWidth(), ofGetHeight());
    texture3.allocate(ofGetWidth(), ofGetHeight());
    texture1.begin();
    ofClear(0, 0, 0, 0);
    texture1.end();
    texture2.begin();
    ofClear(0,0,0,0);
    texture2.end();
    texture3.begin();
    ofClear(0,0,0,0);
    texture3.end();
    ofEnableAlphaBlending();
    damping = 0.83;
    even = true;
    
    //set up game stuff
    mode = 0;

    //osc stuff
    receiver.setup(PORT);
    sender.setup(HOST, OUTPORT);
    
    //physical table stuff
    tableWidth = 1.584;
    tableLength = 3.24;
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
    
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // check for mouse moved message
        if(m.getAddress() == "/nearside/bonk"){
            nearSideBonk = true;
            cout << "got near side bonk" << endl;
        } else if(m.getAddress() == "/nearside/location") {
            lastNearSideLocation.set(m.getArgAsFloat(0), m.getArgAsFloat(1));
            gotNearSideLocation = true;
            cout << "got near side location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        } else if(m.getAddress() == "/farside/bonk") {
            farSideBonk = true;
            cout << "got far side bonk" << endl;
        } else if(m.getAddress() == "/farside/location") {
            lastFarSideLocation.set(m.getArgAsFloat(0), m.getArgAsFloat(1));
            gotFarSideLocation = true;
            cout << "got far side location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        }  else if(m.getAddress() == "/location") {
            lastLocation.set(m.getArgAsFloat(0), m.getArgAsFloat(1));
            gotLocation = true;
            cout << "got location x = " << m.getArgAsFloat(0) << " y = " << m.getArgAsFloat(1) << endl;
        }
    }

    switch(mode) {
    case 0:
        if(even) //draw peturbation into texture 1, use texture 2 as backbuffer, render
        {
            ofPushStyle();
            ofPushMatrix();
            texture1.begin();
            ofFill();
            ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
            //DID WE GET LOCATION
            if(gotLocation) {
                //map ball hit location to x y pixel location
                float x = ofMap(lastLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofEllipse(x,y,10,10);
                
                //mark Location event as handled
                gotLocation = false;
            }
            ofEllipse(mouseX,mouseY, 10,10);
            //DID WE GET BONK (GUESS LOCATION)
            texture1.end();
            ofPopMatrix();
            ofPopStyle();
            
            texture3.begin();
            shader.begin();
            shader.setUniformTexture("backbuffer", texture2.getTextureReference(), 0);
            shader.setUniformTexture("tex0", texture1.getTextureReference(), 1);
            shader.setUniform1f("damping", (float)damping );
            shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
            //render frame
            ofSetColor(255, 255);
            ofRect(0, 0, ofGetWidth(), ofGetHeight() );
            shader.end();
            texture3.end();
            
            texture2.begin();
            texture3.draw(0,0);
            texture2.end();
            //
            even = false;
        } else
        {
            ofPushStyle();
            ofPushMatrix();
            texture2.begin();
            ofFill();
            ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
            //DID WE GET LOCATION
            if(gotLocation) {
                //map ball hit location to x y pixel location
                float x = ofMap(lastLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofEllipse(x,y,10,10);
                
                //mark Location event as handled
                gotLocation = false;
            }
            //ofEllipse(mouseX,mouseY, 10,10);
            //DID WE GET BONK (GUESS LOCATION)
            ofEllipse(mouseX,mouseY, 10,10);
            texture2.end();
            ofPopMatrix();
            ofPopStyle();
            
            texture3.begin();
            //texture3.bind();
            shader.begin();
            shader.setUniformTexture("backbuffer", texture1.getTextureReference(), 0);
            shader.setUniformTexture("tex0", texture2.getTextureReference(), 1);
            shader.setUniform1f("damping", (float)damping );
            shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
            //render frame
            ofSetColor(255, 255);
            ofRect(0, 0, ofGetWidth(), ofGetHeight());
            shader.end();
            texture3.end();
            
            texture1.begin();
            texture3.draw(0,0);
            texture1.end();
            //
            even = true;
        }
        break;
    case 1:
        break;
    }
    
    updateFbo();                                // update our Fbo functions
    teensy.update();                            // update our serial to teensy stuff

}

//--------------------------------------------------------------
void ofApp::updateFbo(){
    fbo.begin();                                // begins the fbo
    ofClear(0,0,0);                             // refreshes fbo, removes artifacts
    ofSetColor(brightness);
    ofPushStyle();
    switch (mode) {
        case 0:
            ofEnableAlphaBlending();
            if ( even ) {
                texture2.draw(0,0);
                
            } else
            {
                texture1.draw(0,0);
                
            }
            break;
            
        case 1:
            break;
    }
    
    ofPopStyle();
    fbo.end();
    fbo.readToPixels(teensy.pixels1);
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
