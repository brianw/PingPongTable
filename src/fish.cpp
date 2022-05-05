#include "fish.h"

Fish::Fish() {
}

void Fish::setup(ofVec2f& _attractor, ofColor _c, float _topSpeed) {
	attractor = &_attractor;
	location = ofVec2f(ofRandom(0, ofGetWidth()), ofRandom(0,ofGetHeight()));
	velocity = ofVec2f(0,0);
	color = _c;
	topSpeed = _topSpeed;
}

void Fish::update(Fish* other) {
	ofVec2f dir = *attractor - location;
	dir.normalize();
	dir *= 0.5;
	ofVec2f rep = other->location - location;
	float dist = location.distance(other->location);
	rep.scale(-0.2/dist);
	acceleration = dir + rep;
	ofVec2f rand = ofVec2f(ofRandom(0, 0.1), ofRandom(0, 0.1));
	acceleration += rand;
	velocity += acceleration;
	velocity.limit(topSpeed);
	location += velocity;

	// ofLog() << " attractor x = " << attractor->x;
	// ofLog() << " attractor y = " << attractor->y;
	// ofLog() << " location x = " << location.x;
	// ofLog() << " location y = " << location.y;
	// ofLog() << " velocity x = " << velocity.x;
	// ofLog() << " velocity y = " << velocity.y;
	// ofLog() << " acceleration x = " << acceleration.x;
	// ofLog() << " acceleration y = " << acceleration.y;
	
}

void Fish::draw() {
	ofPushStyle();
	ofPushMatrix();
	ofSetRectMode(OF_RECTMODE_CENTER);
	float x = ofMap(location.x, 0.0, 1.584, 0.0, ofGetWidth());
 	float y = ofMap(location.y, 0.0, 3.24, 0.0, ofGetHeight());
	ofTranslate(x, y);
	ofTranslate(location);
	ofRotate(PI/2 - velocity.angle(ofVec2f(1.0,0.0)));
	ofSetColor(color);
	ofFill();
	ofDrawRectangle(0.0, 0.0, 5, 1.5);
	ofPopMatrix();
	ofPopStyle();

	// ofPushStyle();
	// ofPushMatrix();
	// ofSetRectMode(OF_RECTMODE_CENTER);
	// ofTranslate();
	// ofSetColor(255);
	// ofDrawRectangle(0.0, 0.0, 10, 2.5);
	// ofPopMatrix();
	// ofPopStyle();

}