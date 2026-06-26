#include "ofMain.h"
#include "ofApp.h"
#if defined(TARGET_LINUX)
#include "ofAppEGLWindow.h"
#endif

//========================================================================
int main( ){
#if defined(TARGET_LINUX)
    auto window = std::make_shared<ofAppEGLWindow>();
    ofAppEGLWindowSettings settings;
    settings.glesVersion = 2;
    settings.eglWindowPreference = OF_APP_WINDOW_NATIVE;
    settings.setSize(44,96);
    settings.windowMode = OF_WINDOW;
    ofInit();
    ofGetMainLoop()->addWindow(window);
    window->setup(settings);
    ofRunApp(window, std::make_shared<ofApp>());
    return ofRunMainLoop();
#elif defined(TARGET_OPENGLES)
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
