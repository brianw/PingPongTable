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
    brightness = 170;                             // LED brightness
    
    // setup our teensys
    teensy.setup(stripWidth, stripHeight, 1, stripsPerPort, numPorts);
    
    /* Configure our teensy boards (portName, xOffset, yOffset, width%, height%, direction) */
    teensy.serialConfigure("TEENSY4", 0, 0, 100, 25, 0);
    teensy.serialConfigure("TEENSY3", 0, 25, 100, 25, 0);
    teensy.serialConfigure("TEENSY1", 0, 50, 100, 25, 0);
    teensy.serialConfigure("TEENSY2", 0, 75, 100, 25, 0);
    
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
    receiver2.setup(PORT2);
    sender.setup(HOST, OUTPORT);
    
    //physical table stuff
    tableWidth = 1.584;
    tableLength = 3.24;

    //sound stuff
    dir.listDir("sounds/splashes/");
    dir.allowExt("wav");
    dir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order

    // //allocate the vector to have as many soundplayers as files
    // if( dir.size() ){
    //     splashes.assign(dir.size(), ofSoundPlayer());
    // }

    // you can now iterate through the files and load them into the ofImage vector
    for(int i = 0; i < (int)dir.size(); i++){
        ofSoundPlayer s = ofSoundPlayer();
        s.load(dir.getPath(i));
        splashes.push_back(s);
        //splashes[i].load(dir.getPath(i));
        cout << "loaded sound " << dir.getPath(i) << endl;
    }

    //set up dubstep stuff
    texture4.allocate(ofGetWidth(), ofGetHeight());
    texture4.begin();
    ofClear(0, 0, 0, 0);
    texture4.end();

    backgroundCol = ofColor(ofRandom(0, 255),
                            ofRandom(0, 255),
                            ofRandom(0, 255)
                            );
    newCol = ofColor(ofRandom(0, 255),
                            ofRandom(0, 255),
                            ofRandom(0, 255)
                            );
    newNewCol = ofColor(ofRandom(0, 255),
                            ofRandom(0, 255),
                            ofRandom(0, 255)
                            );
    loc1 = ofVec2f(ofGetWidth()/2.0, ofGetHeight()/8);
    loc2 = ofVec2f(ofGetWidth()/2.0, 7.0 * ofGetHeight()/8);
    loc3 = ofVec2f(0,0);
    
    lerpTimer1.setDuration(0.2);
    lerpTimer2.setDuration(0.2);

    //dubstep sounds

    beepDir.listDir("sounds/beeps/");
    beepDir.allowExt("wav");
    beepDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order


    for(int i = 0; i < (int)beepDir.size(); i++){
        ofSoundPlayer s = ofSoundPlayer();
        s.load(beepDir.getPath(i));
        beeps.push_back(s);
        //splashes[i].load(dir.getPath(i));
        cout << "loaded sound " << beepDir.getPath(i) << endl;
    }

    boopDir.listDir("sounds/boops/");
    boopDir.allowExt("wav");
    boopDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order


    for(int i = 0; i < (int)boopDir.size(); i++){
        ofSoundPlayer s = ofSoundPlayer();
        s.load(boopDir.getPath(i));
        boops.push_back(s);
        //splashes[i].load(dir.getPath(i));
        cout << "loaded sound " << boopDir.getPath(i) << endl;
    }

    swishBeepDir.listDir("sounds/swish-beeps/");
    swishBeepDir.allowExt("wav");
    swishBeepDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order


    for(int i = 0; i < (int)swishBeepDir.size(); i++){
        ofSoundPlayer s = ofSoundPlayer();
        s.load(swishBeepDir.getPath(i));
        swishBeeps.push_back(s);
        //splashes[i].load(dir.getPath(i));
        cout << "loaded sound " << swishBeepDir.getPath(i) << endl;
    }

    swishBoopDir.listDir("sounds/swish-boops/");
    swishBoopDir.allowExt("wav");
    swishBoopDir.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order


    for(int i = 0; i < (int)swishBoopDir.size(); i++){
        ofSoundPlayer s = ofSoundPlayer();
        s.load(swishBoopDir.getPath(i));
        swishBoops.push_back(s);
        //splashes[i].load(dir.getPath(i));
        cout << "loaded sound " << swishBoopDir.getPath(i) << endl;
    }

    mode = 1;
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

    while(receiver2.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver2.getNextMessage(m);
        if(m.getAddress() == "/video") {
            mode = m.getArgAsInt(0) % 2;
            cout << "got /video = " << m.getArgAsInt(0) << endl;
            cout << "setting mode = " << mode << endl;
        }
    }

    switch(mode) {
    case 0:
        if(even) //draw peturbation into texture 1, use texture 2 as backbuffer, render
        {
            ofPushStyle();
            ofPushMatrix();
            texture1.begin();
            //DID WE GET LOCATION
            if (gotNearSideLocation) {
                //map ball hit location to x y pixel location
                float x = ofMap(lastNearSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastNearSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
                //ofSetColor(255,255);
                ofFill();
                ofDrawEllipse(x,y,12,12 );
                ofSetColor(0,0,255,255);
                ofFill();
                ofDrawEllipse(x,y,10,10);
                gotNearSideLocation = false;
            }
            if (gotFarSideLocation) {
                float x = ofMap(lastFarSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastFarSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
                //ofSetColor(255,255);
                ofFill();
                ofDrawEllipse(x,y,12,12);
                ofSetColor(0,255,0,255);
                ofFill();
                ofDrawEllipse(x,y,10,10);
                gotFarSideLocation = false;
            }
            if(gotLocation) {
                
                playSplash();

                //mark Location event as handled
                gotLocation = false;
            }
            //ofEllipse(mouseX,mouseY, 10,10);
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
            if (gotNearSideLocation) {
                //map ball hit location to x y pixel location
                float x = ofMap(lastNearSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastNearSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
                //ofSetColor(255,255);
                ofFill();
                ofDrawEllipse(x,y,12,12);
                ofSetColor(0,0,255,255);
                ofFill();
                ofDrawEllipse(x,y,10,10);
                gotNearSideLocation = false;
            }
            if (gotFarSideLocation) {
                float x = ofMap(lastFarSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
                float y = ofMap(lastFarSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
                ofSetColor(ofNoise( ofGetFrameNum() ) * 255 * 5, 255);
                //ofSetColor(255,255);
                ofFill();
                ofEllipse(x,y,12,12);
                ofSetColor(0,255,0,255);
                ofFill();
                ofEllipse(x,y,10,10);
                gotFarSideLocation = false;
            }
            if(gotLocation) {
                
                playSplash();

                //mark Location event as handled
                gotLocation = false;
            }
            //ofEllipse(mouseX,mouseY, 10,10);
            //DID WE GET BONK (GUESS LOCATION)
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
        //handle events
        if (gotFarSideLocation) {
            float x = ofMap(lastFarSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
            float y = ofMap(lastFarSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
            ofVec2f vec = ofVec2f(x,y);
            dubStepEvent(vec, false);

            //play beep

            gotFarSideLocation = false;
        }
        if (gotNearSideLocation) {
            float x = ofMap(lastNearSideLocation.x, 0.0, tableWidth, 0.0, ofGetWidth());
            float y = ofMap(lastNearSideLocation.y, 0.0, tableLength, 0.0, ofGetHeight());
            ofVec2f vec = ofVec2f(x,y);
            dubStepEvent(vec, true);

            //play bloop

            gotNearSideLocation = false;
        }

        //handle timer
        if(lerpTimer3.getDone() && (lerpTimer3.getProgress() == 1.0) && dubStepState == 2) {
            dubStepState = 0;
            cout << "going to 0" << endl;
            lerpTimer1.setToValue(0);
            lerpTimer2.setToValue(0);
            lerpTimer3.setToValue(0);
            loc3 = loc2;
            backgroundCol = newCol;
            newCol = newNewCol;
            newNewCol = ofColor(ofRandom(0, 255),
                                ofRandom(0, 255),
                                ofRandom(0, 255)
                                );
            ofLog() << "backgroundCol = " << backgroundCol;
            ofLog() << "newCol = " << newCol;
            ofLog() << "newNewCol = " << newNewCol;
        }

        //draw stuff
        ofPushStyle();
        ofPushMatrix();
        texture4.begin();
        switch(dubStepState) {
            case 0:
                //ofSetBackgroundColor(backgroundCol);
                ofSetColor(backgroundCol);
                ofSetRectMode(OF_RECTMODE_CORNER);
                ofDrawRectangle(0,0, ofGetWidth(), ofGetHeight());
                
                ofSetColor(newCol);
                ofFill();
                ofDrawEllipse(loc2, 35.025, 35.025);
                break;
            case 1:
                //ofSetBackgroundColor(backgroundCol);
                ofSetColor(backgroundCol);
                ofSetRectMode(OF_RECTMODE_CORNER);
                ofDrawRectangle(0,0, ofGetWidth(), ofGetHeight());
                
                ofSetColor(newCol);
                ofFill();
                ofDrawEllipse(loc2, 35.025, 35.025);
                ofDrawEllipse(loc1, 10.0+25.0*lerpTimer1.getProgress(), 10.0+25.0*lerpTimer1.getProgress());
                break;
            case 2:
                //ofSetBackgroundColor(backgroundCol);
                ofSetColor(backgroundCol);
                ofFill();
                ofSetRectMode(OF_RECTMODE_CORNER);
                ofDrawRectangle(0,0, ofGetWidth(), ofGetHeight());
                
                ofSetColor(newCol);
                ofFill();
                ofDrawEllipse(loc1, 10.0+25.0*lerpTimer1.getProgress(), 10.0+25.0*lerpTimer1.getProgress());
                
                
                ofPushMatrix();
                ofTranslate(loc1);
                ofVec2f a = loc2 - loc1;
                //ofRotate(-PI/2 + a.angle(ofVec2f(1.0,0.0)));
                ofRotate(PI/2 - a.angle(ofVec2f(0.0,1.0)));
                ofSetRectMode(OF_RECTMODE_CENTER);
                ofSetColor(newCol);
                ofFill();
                ofDrawRectangle(0.0, 0.0, ofMap(lerpTimer3.getProgress(), 0.0, 1.0, 0.0, 300.0), 2000);
                ofPopMatrix();
                ofSetColor(newNewCol);
                ofDrawEllipse(loc2, 10.0+25.0*lerpTimer2.getProgress(), 10.0+25.0*lerpTimer2.getProgress());

                break;
        }
        texture4.end();
        ofPopMatrix();
        ofPopStyle();

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
            texture4.draw(0,0);
            break;
    }
    
    ofPopStyle();
    fbo.end();
    fbo.readToPixels(teensy.pixels1);
}

void ofApp::playSplash(){
    //play splash
    int i = ofRandom(0,splashes.size()-1);
    cout << i << endl;
    splashes[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
    splashes[i].setVolume(1.0);
    splashes[i].play();
}

void ofApp::dubStepEvent(ofVec2f loc, bool isNear){
    switch(dubStepState) {
        case 0:
            cout << "got event in state 0" << endl;
            loc1 = loc;
            lerpTimer1.lerpToValue(25);
            dubStepState++;
            cout << "going to 1" << endl;
            if (isNear) {
                int i = ofRandom(0,beeps.size()-1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
            } else {
                int i = ofRandom(0,boops.size()-1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();
            }
            break;
        case 1:
            cout << "got event in state 1" << endl;
            loc2 = loc;
            lerpTimer2.lerpToValue(25);
            lerpTimer3.lerpToValue(75);
            dubStepState++;
            cout << "going to 2" << endl;
            if (isNear) {
                int i = ofRandom(0,beeps.size()-1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
                swishBeeps[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                swishBeeps[i].setVolume(1.0);
                swishBeeps[i].play();
            } else {
                int i = ofRandom(0,boops.size()-1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();
                swishBoops[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                swishBoops[i].setVolume(1.0);
                swishBoops[i].play();
            }
            break;
        case 2:
            cout << "got event in state 2" << endl;
            loc1 = loc;
            loc3 = loc2;
        
            lerpTimer1.setToValue(0.0);
            lerpTimer2.setToValue(0.0);
            lerpTimer3.setToValue(0.0);
            backgroundCol = newCol;
            newCol = newNewCol;
            newNewCol = ofColor(ofRandom(0, 255),
                            ofRandom(0, 255),
                            ofRandom(0, 255)
                            );
            ofLog() << "backgroundCol = " << backgroundCol;
            ofLog() << "newCol = " << newCol;
            ofLog() << "newNewCol = " << newNewCol;
            dubStepState = 1;
            cout << "going to 1" << endl;
            if (isNear) {
                int i = ofRandom(0,beeps.size()-1);
                cout << i << endl;
                beeps[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                beeps[i].setVolume(1.0);
                beeps[i].play();
                
            } else {
                int i = ofRandom(0,boops.size()-1);
                cout << i << endl;
                boops[i].setPan(ofMap(lastLocation.y, 0.0, tableLength, -1.0, 1.0, true));
                boops[i].setVolume(1.0);
                boops[i].play();

            }
            break;
    }
}

// void ofApp::onCharacterReceived(KeyListenerEventData& e)
// {
//     keyPressed((int)e.character);
// }

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    ofLogVerbose(__func__) << "key: " << key;
    switch (key) 
    {
        case 'n':
        {
            mode++;
            if (mode == 1) {
                mode = 0;
            }
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
