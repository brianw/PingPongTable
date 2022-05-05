#ifndef _FISH // if this class hasn't been defined, the program can define it
#define _FISH // by using this if statement you prevent the class to be called more than once which would confuse the compiler
#include "ofMain.h" // we need to include this to have a reference to the openFrameworks framework
class Fish {

    public: // place public functions or variables declarations here

    // methods, equivalent to specific functions of your class objects
    void setup(ofVec2f& _attractor, ofColor _c, float _topSpeed);	// setup method, use this to setup your object's initial state
    void update(Fish* other);  // update method, used to refresh your objects properties
    void draw();    // draw method, this where you'll do the object's drawing

    // variables
    ofVec2f *attractor;
    ofVec2f location;
    ofVec2f velocity;
    ofVec2f acceleration;
    float topSpeed;
    int tailLength;        // size
    ofColor color;  // color using ofColor type

    Fish();  // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
    private: // place private functions or variables declarations here
}; // don't forget the semicolon!
#endif