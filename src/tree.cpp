#include "tree.h"

//--------------------------------------------------------------
Node::Node(Node *_parent, float photoSize, string _texPath) {
	parent = _parent;
	growth.animateFromTo(0., 1.);
	//growth.reset(1.);
	growth.setDuration(1.5);
	growth.setCurve(EXPONENTIAL_SIGMOID_PARAM);
	//growth.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);

	originPos.set(0,0,0);
	originSize = 0.;

	isBeingDeleted = false;
	hasFixedPos = false;	

	if (_parent != NULL) {		
		level = _parent->level + 1;
		currentPos = _parent->currentPos;
		targetSize = 5.;
	}
	else {
		level = 0;
		currentPos.set(0,0,0);
		targetSize = 20.;
	}	

	//targetSize = 20. * exp(-level*.001);
	texPath = _texPath;
	if (texPath != "") {
		
		ofPixels pix;
		ofLoadImage(pix, texPath);
		pix.resize(photoSize, photoSize);
		//pix.resize(250, 250);
		tex.loadData(pix);
		
		//ofLoadImage(tex, texPath);
		
	}

	targetSize = 30*exp(-(level*.5));
	needsToUpdatePos = false;
}

Tree::Tree() {
	int numPts = 128;
	float angle = 0.0;
	float step = TWO_PI / (float)(numPts - 1);

	selectedNode = NULL;

	for (int i = 0; i < numPts; i++) {

		//get the -1 to 1 values - we will use these for drawing our pts.   
		float px = cos(angle);
		float py = sin(angle);
		NormCirclePts.push_back(ofPoint(px, py));
	
		float tx = ofMap(NormCirclePts[i].x, -1.0, 1.0, 0, photoSize);
		float ty = ofMap(NormCirclePts[i].y, 1.0, -1.0, 0, photoSize);
	
		/*
		float tx = ofMap(NormCirclePts[i].x, -1.0, 1.0, 0, 250);
		float ty = ofMap(NormCirclePts[i].y, 1.0, -1.0, 0, 250);
		*/
		NormCircleCoords.push_back(ofPoint(tx, ty));

		angle += step;
	}
	

	string visualizationSettingsFile = "visualizationSettings.xml";	
	gui.setup("", visualizationSettingsFile); // most of the time you don't need a name

	lineWidth.setup("GROSOR", 2, 0, 15);
	level1LinesColor.setup("NIVEL 1 - COLOR", ofColor(100, 100, 100, 100), ofColor(0, 0, 0, 0), ofColor(255, 255, 255, 255));
	level2LinesColor.setup("NIVEL 2 - COLOR", ofColor(100, 100, 100, 100), ofColor(0, 0, 0, 0), ofColor(255, 255, 255, 255));
	level3LinesColor.setup("NIVEL 3 - COLOR", ofColor(100, 100, 100, 100), ofColor(0, 0, 0, 0), ofColor(255, 255, 255, 255));	
	lines.setName("LINEAS");
	lines.add(lineWidth.getParameter());
	lines.add(level1LinesColor.getParameter());
	lines.add(level2LinesColor.getParameter());
	lines.add(level3LinesColor.getParameter());	
	gui.add(lines);
	

	photos.setName("FOTOS");
	photosPositions.setName("POSICION");
	photos.add(photosPositions);
	photosFrame.setName("MARCO CIRCULAR");
	photos.add(photosFrame);

	photosPositions.add(firstRingMinRad.setup("firstRingMinRad", 7, 0, 100)->getParameter());
	photosPositions.add(firstRingMaxRad.setup("firstRingMaxRad", 30.5, 0, 100)->getParameter());
	photosPositions.add(firstRingMinZ.setup("firstRingMinZ", -10, -20, 20)->getParameter());
	photosPositions.add(firstRingMaxZ.setup("firstRingMaxZ", 7.4, -20, 20)->getParameter());
	photosPositions.add(secondRingMinRad.setup("secondRingMinRad", 23, 0, 100)->getParameter());
	photosPositions.add(secondRingMaxRad.setup("secondRingMaxRad", 33, 0, 100)->getParameter());
	photosPositions.add(secondRingMinZ.setup("secondRingMinZ", 5, -20, 20)->getParameter());
	photosPositions.add(secondRingMaxZ.setup("secondRingMaxZ", -10, -20, 20)->getParameter());
	photosPositions.add(radXMultiplier.setup("radXMultiplier", 1., 0., 2.)->getParameter());
	photosPositions.add(angularNoiseVariance.setup("angularNoiseVariance", 1., 0, 2)->getParameter());

	photosFrame.add(fotoRingFrameWidth.setup("GROSOR", 0.15, 0., 1.)->getParameter());
	photosFrame.add(fotoRingFrameColor.setup("COLOR", ofColor(100, 150, 200, 255), ofColor(0, 0, 0, 0), ofColor(255, 255, 255, 255))->getParameter());
	
	gui.add(photos);

	
	gui.loadFromFile(visualizationSettingsFile);
	
}

//--------------------------------------------------------------
void Tree::setup(int levels, int leavesPerLevel){

	nodes.clear();	
	vector<vector<Node*> > leveledNodes;

	for (int l = 0; l < levels; l++) {		
		leveledNodes.push_back(vector<Node *>());
		if (l > 0) {
			for (int parent = 0; parent < leveledNodes[l - 1].size(); parent++) {
				Node *currentParent = leveledNodes[l - 1][parent];
				for (int child = 0; child < leavesPerLevel; child++) {					
					nodes.push_back(Node(currentParent));
					leveledNodes.back().push_back(&nodes.back());
					currentParent->children.push_back(&(nodes.back()));				
				}
			}			
		}
		else {
			//Create root node
			nodes.push_back(Node(NULL));
			leveledNodes.back().push_back(&(nodes.back()));
		}		
	}

	updateNodePositions();

	level = levels;
}

//--------------------------------------------------------------
void Tree::setup() {

	nodes.clear();
	vector<vector<Node*> > leveledNodes;

	int levels = 3;
	int leavesPerLevel = 5;
	float R = 1;

	for (int l = 0; l < levels; l++) {
		leveledNodes.push_back(vector<Node *>());
		switch(l) {
			case 0:
				//Create root node
				nodes.push_back(Node(NULL));
				leveledNodes.back().push_back(&(nodes.back()));
				break;

			case 1:
				{
					//Create main categories with spherical spread			
					Node *currentParent = leveledNodes[0][0];
					for (int child = 0; child < leavesPerLevel; child++) {
						nodes.push_back(Node(currentParent));
						leveledNodes.back().push_back(&nodes.back());
						currentParent->children.push_back(&(nodes.back()));						
					}
				}				
				break;

			default:
				for (int parent = 0; parent < leveledNodes[l - 1].size(); parent++) {
					Node *currentParent = leveledNodes[l - 1][parent];
					for (int child = 0; child < leavesPerLevel; child++) {
						nodes.push_back(Node(currentParent));
						leveledNodes.back().push_back(&nodes.back());
						currentParent->children.push_back(&(nodes.back()));
					}
				}
				break;

		}		
	}

	updateNodePositions();

	level = levels;
}

//--------------------------------------------------------------
bool Tree::load(string jsonFile) {

	// Now parse the JSON
	ofxJSONElement result;
	bool parsingSuccessful = result.open(jsonFile);
	if (parsingSuccessful)
	{
		jsonPath = jsonFile;
		position.set(result["xPos"].asFloat(), result["yPos"].asFloat(), result["zPos"].asFloat());
		size = result["size"].asFloat();

		result = result["nodes"];
		nodes.clear();
		nodes.push_back(Node(NULL, photoSize, result[0]["texPath"].asString()));
		nodes.back().currentPos.set(result[0]["currentXPos"].asFloat(), result[0]["currentYPos"].asFloat(), result[0]["currentZPos"].asFloat());
		nodes.back().hasFixedPos = result[0]["hasFixedPos"].asBool();

		vector<vector<Node*> > leveledNodes;
		leveledNodes.push_back(vector<Node *>());
		leveledNodes.back().push_back(&(nodes.back()));
		vector<vector<ofxJSONElement> > leveledNodesJson;
		leveledNodesJson.push_back(vector<ofxJSONElement>());
		leveledNodesJson.back().push_back(result[0]);

		cout << "||- Loading Nodes" << endl;

		bool childFound = true;
		while (childFound) {
			vector<Node *> newChildren;
			vector<ofxJSONElement> newChildrenJson;
			for (int i = 0; i < leveledNodesJson.back().size(); i++) {
				//newChildren.insert(newChildren.end(), leveledNodes.back()[i]->children.begin(), leveledNodes.back()[i]->children.end());					
				ofxJSONElement& parent = leveledNodesJson.back()[i];
				
				if (parent.isMember("children")) {
					for (int j = 0; j < parent["children"].size(); j++) {
						if (leveledNodes.size() == 4 && j >= maxPhotosPerNode) {
							break;
						}
						nodes.push_back(Node(leveledNodes.back()[i], photoSize, parent["children"][j]["texPath"].asString()));
						nodes.back().hasFixedPos = parent["children"][j]["hasFixedPos"].asBool();
						nodes.back().targetPos.set(parent["children"][j]["currentXPos"].asFloat(), parent["children"][j]["currentYPos"].asFloat(), parent["children"][j]["currentZPos"].asFloat());												
						
						if (leveledNodes.size() == 4) {
							nodes.back().needsToUpdatePos = true;
						}
						
						leveledNodes.back()[i]->children.push_back(&nodes.back());

						newChildren.push_back(&nodes.back());
						newChildrenJson.push_back(parent["children"][j]);

						cout << "||- " << ofToString(j) << " / " << ofToString(i) << " of " << ofToString(leveledNodesJson.back().size()) << " Levels" << endl;
					}
				}				
			}
			if (newChildren.size() > 0) {
				leveledNodes.push_back(newChildren);
				leveledNodesJson.push_back(newChildrenJson);
			}
			else {
				childFound = false;
			}
		}

		level = leveledNodes.size()-1;
		
		updateNodePositions();
		jsonPath = jsonFile;
	}	
	
	return parsingSuccessful;
}

//--------------------------------------------------------------
bool Tree::save(string jsonFile) {	
	ofxJSONElement jsonNodes;	
	
	vector<vector<Node*> > leveledNodes;
	leveledNodes.push_back(vector<Node *>());
	leveledNodes.back().push_back(&nodes.front());

	vector<vector<ofxJSONElement *> > leveledNodesJson;
	leveledNodesJson.push_back(vector<ofxJSONElement *>());
	leveledNodesJson.back().push_back(&jsonNodes);
	
	jsonNodes["currentXPos"] = nodes.front().currentPos.x;
	jsonNodes["currentYPos"] = nodes.front().currentPos.y;
	jsonNodes["currentZPos"] = nodes.front().currentPos.z;
	jsonNodes["hasFixedPos"] = nodes.front().hasFixedPos;
	jsonNodes["texPath"] = nodes.front().texPath;

	bool childFound = true;
	while (childFound) {
		vector<Node *> newChildren;
		vector<ofxJSONElement *>newChildrenJson;
		for (int i = 0; i < leveledNodes.back().size(); i++) {
			newChildren.insert(newChildren.end(), leveledNodes.back()[i]->children.begin(), leveledNodes.back()[i]->children.end());		
			if (leveledNodes.size() > 4) {
				int r = 0;
			}
			for (int j = 0; j < leveledNodes.back()[i]->children.size(); j++) {
				ofxJSONElement newChild;
				newChild["currentXPos"] = leveledNodes.back()[i]->children[j]->targetPos.x;
				newChild["currentYPos"] = leveledNodes.back()[i]->children[j]->targetPos.y;
				newChild["currentZPos"] = leveledNodes.back()[i]->children[j]->targetPos.z;
				newChild["hasFixedPos"] = leveledNodes.back()[i]->children[j]->hasFixedPos;		
				newChild["texPath"] = leveledNodes.back()[i]->children[j]->texPath;
				leveledNodesJson.back()[i]->operator[]("children").append(newChild);								
				newChildrenJson.push_back((ofxJSONElement *)(&(leveledNodesJson.back()[i]->operator[]("children")[j])));
			}
		}
		if (newChildren.size() > 0) {
			leveledNodes.push_back(newChildren);
			leveledNodesJson.push_back(newChildrenJson);
		}
		else {
			childFound = false;
		}
	}

	ofxJSONElement jsonTree;
	jsonTree["xPos"] = position.x;
	jsonTree["yPos"] = position.y;
	jsonTree["zPos"] = position.z;
	jsonTree["size"] = size;
	jsonTree["nodes"].append(jsonNodes);

	if (jsonFile != "") {
		jsonTree.save(jsonFile, true);
		jsonPath = jsonFile;
	}
	else {
		jsonTree.save(jsonPath, true);
	}

	return true;
}

//--------------------------------------------------------------
void Tree::update() {
	unsigned int maxLevel = 0;
	vector<std::list<Node>::iterator> nodesToErase;
	for (std::list<Node>::iterator it = nodes.end(); it != nodes.begin();) {
		it--;
		if (ofGetLastFrameTime()<1.) {
			it->growth.update(ofGetLastFrameTime());
		}		

		it->currentSize = (it->targetSize*it->growth.getCurrentValue() + it->originSize*(1. - it->growth.getCurrentValue()));

		if (it->level == 0) {
			it->currentPos.set(0, 0, 0);
		}
		else {
			//it->currentPos.set(it->targetPos*it->growth.getCurrentValue() + it->parent->currentPos*(1.-it->growth.getCurrentValue()));
			it->currentPos.set(it->targetPos*it->growth.getCurrentValue() + it->originPos*(1. - it->growth.getCurrentValue()));		
			/*
			if (nodes.size()>156) {
				float val = it->growth.getCurrentValue();
				int i = 0;
			}
			*/
		}

		if (it->isReadyToDelete()) {
			for (int i = 0; i < it->parent->children.size(); i++) {
				if (it->parent->children[i] == &(*it)) {
					it->parent->children.erase(it->parent->children.begin() + i);
					break;
				}
			}
			nodesToErase.push_back(it);			
		}
		else {
			maxLevel = max(maxLevel, it->level);
		}
	}

	for (int i = 0; i < nodesToErase.size(); i++) {
		nodes.erase(nodesToErase[i]);
	}

	level = maxLevel;
}

//--------------------------------------------------------------
void Tree::draw(ofVec3f cameraPos) {
	
	ofPushMatrix();
	ofPushStyle();

	ofEnableDepthTest();
	ofEnableAlphaBlending();
	ofEnableAntiAliasing();
	//ofEnableSmoothing();
	ofDisableSmoothing();
	ofSetSphereResolution(100);
	ofSetCircleResolution(100);

	ofSetLineWidth(lineWidth);

	

	for (std::list<Node>::iterator it = nodes.begin(); it != nodes.end(); ++it) {	
		bool nodeOnFront = cameraPos.angle(it->currentPos) < 30.;

		if (it->level == 0) {
			ofSetColor(level1LinesColor);
			for (int child = 0; child < it->children.size(); child++) {
				ofVec2f projectedChildPos = it->children[child]->currentPos;
				ofDrawLine(projectedChildPos.normalized()*it->currentSize, it->children[child]->currentPos);
			}

			if (it->tex.isAllocated()) {

				ofSetColor(255);
				it->tex.bind();

				glBegin(GL_POLYGON);
				for (int i = 0; i < NormCirclePts.size(); i++) {
					glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
					glVertex2f(NormCirclePts[i].x * it->currentSize, NormCirclePts[i].y * it->currentSize);
				}
				glEnd();

				it->tex.unbind();
			}
			else {
				ofSetColor(level1LinesColor);
				ofDrawSphere(0, 0, 0, it->currentSize);
			}

		}
		else if (it->level < 3) {			
			ofMatrix4x4 lookAtMat;
			lookAtMat.makeLookAtMatrix(it->currentPos, ofVec3f(0, 0, 0), ofVec3f(0, 1, 0));
			
			if (it->level == 1) {
				ofSetColor(level2LinesColor);
			}
			else {
				ofSetColor(level3LinesColor);
			}
						
			ofPushMatrix();
			ofMultMatrix(lookAtMat);

			for (int child = 0; child < it->children.size(); child++) {
				
				ofVec3f childPos = (it->children[child]->currentPos - it->currentPos)*lookAtMat.getInverse();
				ofVec3f circleEdge = (ofVec3f(childPos.x, childPos.y, 0).normalized()*it->currentSize);
				ofDrawLine(circleEdge, (it->children[child]->currentPos)*lookAtMat.getInverse());								
				//ofDrawLine(it->currentPos, it->children[child]->currentPos);								
			}
			
			
			if (it->tex.isAllocated()) {
				
				ofSetColor(255);
				it->tex.bind();

				glBegin(GL_POLYGON);
				for (int i = 0; i < NormCirclePts.size(); i++) {
					glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
					glVertex2f(NormCirclePts[i].x * it->currentSize, NormCirclePts[i].y * it->currentSize);
				}
				glEnd();

				it->tex.unbind();
			}
			else {
				ofDrawSphere(0, 0, 0, it->currentSize);
			}
			ofPopMatrix();
		}
		else if (it->level == 3) {
			//photoContainers[point].draw();
			ofPushMatrix();
			ofMatrix4x4 lookAtMat;
			lookAtMat.makeLookAtMatrix(it->currentPos, ofVec3f(0, 0, 0), ofVec3f(0, 1, 0));
			ofMultMatrix(lookAtMat);			
			
			//ofSetColor(255, 255, 255);
			ofSetColor(fotoRingFrameColor);
			

			for (int photo = 0; photo < it->children.size(); photo++) {
				//ofDrawLine(ofVec3f(x, y, 0).normalized()*spheresSizes[sphere][point], ofVec3f(x, y, z) - ofVec3f(x, y, 0).normalized()*size*.5);
				ofVec3f pos = it->children[photo]->currentPos;
				ofDrawLine(ofVec3f(pos.x, pos.y, 0).normalized()*it->currentSize, pos - ofVec3f(pos.x, pos.y, 0).normalized()*it->children[photo]->currentSize);
				//ofDrawLine(ofVec3f(it->children[photo]->currentPos.x, it->children[photo]->currentPos.y, 0).normalized()*5., ofVec3f(it->children[photo]->currentPos.x, it->children[photo]->currentPos.y, 0));
			}
			
			//if (&(*it) == selectedNode) {
			if(nodeOnFront){
				glDepthFunc(GL_ALWAYS);
			}

			for (int photo = 0; photo < it->children.size(); photo++) {				
				ofVec3f pos = it->children[photo]->currentPos;

				ofPushMatrix();
				ofTranslate(0, 0, pos.z);												

				if (it->children[photo]->tex.isAllocated()) {	
					ofSetColor(255, 255, 255);
					it->children[photo]->tex.bind();
					
					glBegin(GL_POLYGON);
					for (int i = 0; i < NormCirclePts.size(); i++) {
						glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
						glVertex2f(NormCirclePts[i].x * it->children[photo]->currentSize + pos.x, NormCirclePts[i].y * it->children[photo]->currentSize + pos.y);
					}
					glEnd();

					it->children[photo]->tex.unbind();

					//Draw surrounding frame.
					//Drawing a thick line makes it look weird so we create a closed polygon arc.
					
					//if (&(*it) == selectedNode) {						
					if (nodeOnFront){
						ofPath frame;
						ofVec2f arcStartPoint(pos - pos.normalized()*it->children[photo]->currentSize);
						frame.moveTo(arcStartPoint);
						frame.setCircleResolution(100);
						frame.setFilled(true);

						ofVec2f aux(pos - arcStartPoint);
						aux.x = -aux.x;
						float startAngle = aux.angle(ofVec2f(1, 0));
						frame.arc(pos.x, pos.y, it->children[photo]->currentSize, it->children[photo]->currentSize, startAngle, ofMap(it->children[photo]->growth.val(), 0, 1, startAngle, startAngle + 360));
						frame.arcNegative(pos.x, pos.y, it->children[photo]->currentSize*(1.- fotoRingFrameWidth), it->children[photo]->currentSize*(1. - fotoRingFrameWidth), ofMap(it->children[photo]->growth.val(), 0, 1, startAngle, startAngle + 360), startAngle);
						frame.setColor(fotoRingFrameColor);
						frame.draw();
					}										
				}
				else {
					ofSetColor(fotoRingFrameColor);
					ofDrawCircle(pos.x, pos.y, it->children[photo]->currentSize);
				}								
				
				ofPopMatrix();
			}

			
			
			if (it->tex.isAllocated()) {
				ofSetColor(255);

				it->tex.bind();

				glBegin(GL_POLYGON);
				for (int i = 0; i < NormCirclePts.size(); i++) {
					glTexCoord2f(NormCircleCoords[i].x, NormCircleCoords[i].y);
					glVertex2f(NormCirclePts[i].x * it->currentSize, NormCirclePts[i].y * it->currentSize);
				}
				glEnd();

				it->tex.unbind();				
			}	
			else {
				ofSetColor(level3LinesColor);
				ofDrawCircle(0, 0, it->currentSize);
			}

			//if (&(*it) == selectedNode) {
			if (nodeOnFront) {
				glDepthFunc(GL_LESS);
			}
			
			ofPopMatrix();
		}		
	}
	ofPopStyle();
	ofPopMatrix();	

}

//--------------------------------------------------------------
unsigned int Tree::getTotalNodes() {
	return nodes.size();
}

//--------------------------------------------------------------
//Avoid using it, it's slow!
Node &Tree::operator[](unsigned int index) {
	std::list<Node>::iterator it = nodes.begin();
	for (int i = 0; i < index; i++) {
		++it;
	}	
	return *it;
}

//--------------------------------------------------------------
void Tree::updateNodePositions() {
	for (std::list<Node>::iterator it = nodes.begin(); it != nodes.end(); ++it) {				

		if ( (it->children.size()>0)  && (it->children[0]->needsToUpdatePos) ) {
			if (!it->isBeingDeleted) {
				it->originPos.set(it->currentPos);
			}
			else {
				it->targetPos.set(it->currentPos);
			}

			switch (it->level) {
			case 3:
				int totalFaces = 0;
				for (int i = 0; i < it->children.size(); i++) {
					if (!it->children[i]->isBeingDeleted) {
						totalFaces++;
					}
				}
				//totalFaces = it->children.size();
				
				int angularDivisions = totalFaces;				
				if(totalFaces>(maxPhotosPerNode*.5)){
					angularDivisions = (maxPhotosPerNode*.5);
				}
				int notBeingDeletedIndex = 0;
				for (int face = 0; face < it->children.size(); face++) {

					if (!it->children[face]->isBeingDeleted) {
						it->children[face]->originPos.set(it->children[face]->currentPos);

						ofPoint pos;

						float rad = ofRandom(firstRingMinRad, firstRingMaxRad);
						//float angularNoise = ofRandom(-(.5*PI / angularDivisions), (.5*PI / angularDivisions));
						float angularNoise = ofRandom(-(angularNoiseVariance*PI / angularDivisions), (angularNoiseVariance*PI / angularDivisions));
						pos.z = ofRandom(firstRingMinZ, firstRingMaxZ);
						if (face > maxPhotosPerNode*.5-1) {							
							rad = ofRandom(secondRingMinRad, secondRingMaxRad);
							pos.z = ofRandom(secondRingMinZ, secondRingMaxZ);
							
						}						

						pos.x = rad * radXMultiplier * cos((2.*PI / angularDivisions)*notBeingDeletedIndex + angularNoise);
						pos.y = rad* sin((2.*PI / angularDivisions)*notBeingDeletedIndex + angularNoise);

						it->children[face]->originPos = it->children[face]->currentPos;
						it->children[face]->targetPos = pos;
						it->children[face]->originSize = it->children[face]->currentSize;
						it->children[face]->targetSize = ofRandom(3.5, 6.5);
						it->children[face]->needsToUpdatePos = false;

						++notBeingDeletedIndex;
					}
					else {
						it->children[face]->targetPos.set(it->children[face]->currentPos);
						it->children[face]->originPos.set(0,0);
						it->children[face]->targetSize = it->children[face]->currentSize;
						it->children[face]->originSize = 0;
					}					
				}
				break;
			}
		}
					
	}
}

//--------------------------------------------------------------
Node *Tree::growChild(Node *parent, float delay) {
	return growChild(parent, "", delay);
}

//--------------------------------------------------------------
Node *Tree::growChild(Node *parent, string texturePath, float delay) {

	nodes.push_back(Node(parent, photoSize, texturePath));
	parent->children.push_back(&nodes.back());		

	vector<vector<Node*> > leveledNodes;
	leveledNodes.push_back(vector<Node *>());
	leveledNodes.back().push_back(parent);


	//Update siblings' positions to be uniformly distributed
	bool childFound = true;
	while (childFound) {
		vector<Node *> newChildren;
		for (int i = 0; i < leveledNodes.back().size(); i++) {
			newChildren.insert(newChildren.end(), leveledNodes.back()[i]->children.begin(), leveledNodes.back()[i]->children.end());
			for (int j = 0; j < leveledNodes.back()[i]->children.size(); j++) {				
				leveledNodes.back()[i]->children[j]->growth.reset(0.);
				leveledNodes.back()[i]->children[j]->growth.animateToAfterDelay(1., delay+ofRandom(0,1));
				//leveledNodes.back()[i]->children[j]->growth.animateFromTo(0., 1.);
				leveledNodes.back()[i]->children[j]->needsToUpdatePos = true;
			}
		}
		if (newChildren.size() > 0) {
			leveledNodes.push_back(newChildren);
		}
		else {
			childFound = false;
		}
	}

	nodes.back().currentPos.set(0, 0, -0.1);
	nodes.back().currentSize = 0;
	//nodes.back().growth.setRepeatType(LOOP_BACK_AND_FORTH);

//	nodes.back().growth.animateFromTo(0., 1.);

	level = max(level, nodes.back().level);

	if (parent->children.size()>maxPhotosPerNode) {
		for (int i = 0; i < parent->children.size(); i++) {
			if (!parent->children[i]->isBeingDeleted) {
				deleteNode(parent->children[i], delay);
				break;
			}
		}
		
	}

	updateNodePositions();

	return &nodes.back();
}

//--------------------------------------------------------------
void Tree::deleteNode(Node *node, float delay) {
	Node *parent = node->parent;

	if (parent == NULL) return;

	vector<vector<Node*> > leveledNodes;
	leveledNodes.push_back(vector<Node *>());
	leveledNodes.back().push_back(parent);

	bool childFound = true;
	while (childFound) {
		vector<Node *> newChildren;
		for (int i = 0; i < leveledNodes.back().size(); i++) {
			newChildren.insert(newChildren.end(), leveledNodes.back()[i]->children.begin(), leveledNodes.back()[i]->children.end());
			for (int j = 0; j < leveledNodes.back()[i]->children.size(); j++) {
				if (!leveledNodes.back()[i]->children[j]->isBeingDeleted) {
					leveledNodes.back()[i]->children[j]->growth.reset(0.);
					leveledNodes.back()[i]->children[j]->growth.animateToAfterDelay(1., delay + ofRandom(0.,1.));
					//leveledNodes.back()[i]->children[j]->growth.animateFromTo(0., 1.);
					leveledNodes.back()[i]->children[j]->needsToUpdatePos = true;
				}					
			}
		}
		if (newChildren.size() > 0) {
			leveledNodes.push_back(newChildren);
		}
		else {
			childFound = false;
		}
	}

	leveledNodes.clear();
	leveledNodes.push_back(vector<Node *>());
	leveledNodes.back().push_back(node);
	node->growth.reset(1.);
	node->growth.animateToAfterDelay(0., delay);
	node->needsToUpdatePos = true;
	node->originPos.set(0,0);
	node->originSize = 0;
	node->isBeingDeleted = true;

	childFound = true;
	while (childFound) {
		vector<Node *> newChildren;
		for (int i = 0; i < leveledNodes.back().size(); i++) {
			newChildren.insert(newChildren.end(), leveledNodes.back()[i]->children.begin(), leveledNodes.back()[i]->children.end());
			for (int j = 0; j < leveledNodes.back()[i]->children.size(); j++) {								
				leveledNodes.back()[i]->children[j]->growth.reset(1.);
				leveledNodes.back()[i]->children[j]->growth.animateToAfterDelay(0., delay + ofRandom(0.,1.));
				leveledNodes.back()[i]->children[j]->originPos.set(0,0);
				leveledNodes.back()[i]->children[j]->originSize=0;
				//leveledNodes.back()[i]->children[j]->growth.animateFromTo(0., 1.);
				leveledNodes.back()[i]->children[j]->needsToUpdatePos = true;
				leveledNodes.back()[i]->children[j]->isBeingDeleted = true;
			}
		}
		if (newChildren.size() > 0) {
			leveledNodes.push_back(newChildren);
		}
		else {
			childFound = false;
		}
	}
	updateNodePositions();	
}

//--------------------------------------------------------------
Node *Tree::isInside(int x, int y) {
	/*
	TO DO
	float _x = ((float)x - position.x) / size;
	float _y = ((float)y - position.y) / size;
	
	for (std::list<Node>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
		if ((ofPoint(_x, _y) - it->currentPos).length() < NODE_RADIUS / size) {
			return &(*it);
		}
	}
	*/
	return NULL;	
}

//--------------------------------------------------------------
void Tree::move(Node *node, int x, int y) {
	/*
		TO DO
	*/
}

//--------------------------------------------------------------
void Tree::setPhotoSize(float _photoSize) {
	photoSize = _photoSize;
	NormCircleCoords.clear();
	for (int i = 0; i < NormCirclePts.size(); i++) {
		float tx = ofMap(NormCirclePts[i].x, -1.0, 1.0, 0, photoSize);
		float ty = ofMap(NormCirclePts[i].y, 1.0, -1.0, 0, photoSize);
		NormCircleCoords.push_back(ofPoint(tx, ty));
	}
	
}