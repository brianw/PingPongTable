#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
#ifdef TARGET_OPENGLES
    ofGLESWindowSettings settings;
    settings.glesVersion = 2;
    //settings.width = 44;
    //settings.height = 96;
    settings.setSize(44,96);
    ofCreateWindow(settings);
#else
    ofSetupOpenGL(1024,768,OF_WINDOW);
#endif
    
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    
}
