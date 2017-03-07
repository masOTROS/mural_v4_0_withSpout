#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxJSON.h"
#include "ofxGui.h"


#define MAX_PHOTOS_PER_NODE 3
#define TEX_WIDTH 250.

class Node {
public:
	Node(Node *_parent, float photoSize=0, string texPath="");

	Node *parent;
	vector<Node *> children;
	unsigned int level;

	//Node rendering
	ofPoint targetPos;
	ofPoint originPos;
	ofPoint currentPos;
	
	float targetSize;
	float originSize;
	float currentSize;

	ofTexture tex;
	string texPath;

	ofxAnimatableFloat growth;
	bool hasFixedPos;

	/*
	bool isBeingDeleted() {
		return (growth.getTargetValue() == 0.);
	};
	*/
	bool isBeingDeleted;
	bool isReadyToDelete() {
		return (isBeingDeleted) && !growth.isOrWillBeAnimating();
	}	

	bool needsToUpdatePos;
};

class Tree{

	public:
		Tree();
		void setup(int level, int leavesPerLevel);
		void setup();
		bool load(string jsonFile);
		bool save(string jsonFile="");
		void update();
		//void draw(ofColor color = ofColor::red);
		void draw(ofVec3f cameraPos);
		unsigned int getTotalNodes();
		Node &operator[](unsigned int index);		
		Node *growChild(Node *parent, float delay);
		Node *growChild(Node *parent, string texPath, float delay);
		void deleteNode(Node *node, float delay);
		Node *isInside(int x, int y);
		void move(Node *node, int x, int y);
		void setPhotoSize(float _photoSize);

		ofVec3f position;
		float size;

		int maxPhotosPerNode;

		//GUI
		ofParameterGroup lines;
		ofxFloatSlider lineWidth;
		ofxColorSlider level1LinesColor, level2LinesColor, level3LinesColor;

		ofParameterGroup photos;
		ofParameterGroup photosPositions;
		ofxFloatSlider secondRingMinRad, secondRingMaxRad, secondRingMinZ, secondRingMaxZ, firstRingMinRad, firstRingMaxRad, firstRingMinZ, firstRingMaxZ;
		ofxFloatSlider radXMultiplier;
		ofxFloatSlider angularNoiseVariance;
		ofParameterGroup photosFrame;
		ofxFloatSlider fotoRingFrameWidth;
		ofxColorSlider fotoRingFrameColor;

		ofxPanel gui;
		

		Node* selectedNode;
		

	private:
		list<Node> nodes;
		unsigned int level;
		void updateNodePositions();
		string jsonPath;

		vector <ofPoint> NormCirclePts;
		vector <ofPoint> NormCircleCoords;

		float photoSize;
};
