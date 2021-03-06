#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxDOF.h"
#include "tree.h"
#include "ofxAnimatableFloat.h"
#include "ofxJSON.h"
#include "ofxSpout.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

		Tree tree;
		string treeConfigFile;

		ofxDOF depthOfField;
		ofEasyCam cam;

		ofVec3f camTargetPos;
		ofVec3f camOriginPos;
		ofxAnimatableFloat posAnim;

		string usedPhotosPath, newPhotosPath;
		float cameraRadius;
		float cameraMovementDuration;
		float cameraDelay;

		float visualizationDelay;

		bool autoSave;

		Node *nextTargetNode;

		bool hideGui;

		// SPOUT stuff
		ofFbo spoutTexture;
		ofxSpout::Sender spoutSender;
};

