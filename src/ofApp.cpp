#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){	
	ofxJSONElement settings;

	ofHideCursor();

	settings.openLocal("settings.json");
	usedPhotosPath = settings["ubicacion de las fotos"]["usadas"].asString();
	newPhotosPath = settings["ubicacion de las fotos"]["nuevas"].asString();
	
	depthOfField.setup(ofGetWidth(), ofGetHeight());
	depthOfField.setFocalDistance(settings["camara"]["distancia focal"].asFloat());

	treeConfigFile = settings["visualizacion"]["archivo de configuracion"].asString();
	tree.setPhotoSize(settings["visualizacion"]["dimension foto"].asFloat());
	tree.maxPhotosPerNode = settings["visualizacion"]["max fotos por nodo"].asInt();
	tree.load(treeConfigFile);


	
	//Setup tree category images

	string texPath = "Categorias/RAIZ.png";
	tree[0].texPath = texPath;

	ofPixels pix;
	ofLoadImage(pix, texPath);
	pix.resize(settings["visualizacion"]["dimension foto"].asFloat(), settings["visualizacion"]["dimension foto"].asFloat());
	tree[0].tex.loadData(pix);
	
	for (int i = 0; i < 5; i++) {
		string texPath = "Categorias/CAT_" + ofToString(i) + ".png";
		tree[0].children[i]->texPath = texPath;

		ofPixels pix;
		ofLoadImage(pix, texPath);
		pix.resize(settings["visualizacion"]["dimension foto"].asFloat(), settings["visualizacion"]["dimension foto"].asFloat());
		tree[0].children[i]->tex.loadData(pix);

		for (int j = 0; j < 5; j++) {
			string texPath = "Categorias/CAT_" + ofToString(i) + "_" + ofToString(j) + ".png";
			tree[0].children[i]->children[j]->texPath = texPath;

			ofPixels pix;
			ofLoadImage(pix, texPath);
			pix.resize(settings["visualizacion"]["dimension foto"].asFloat(), settings["visualizacion"]["dimension foto"].asFloat());
			tree[0].children[i]->children[j]->tex.loadData(pix);

			for (int k = 0; k < 5; k++) {
				string texPath = "Categorias/CAT_" + ofToString(i) + "_" + ofToString(j) + "_" + ofToString(k) + ".png";
				tree[0].children[i]->children[j]->children[k]->texPath = texPath;

				ofPixels pix;
				ofLoadImage(pix, texPath);
				pix.resize(settings["visualizacion"]["dimension foto"].asFloat(), settings["visualizacion"]["dimension foto"].asFloat());
				tree[0].children[i]->children[j]->children[k]->tex.loadData(pix);
			}
		}
	}	
	
	tree.save();
	

	cameraRadius = settings["camara"]["radio"].asFloat();
	if (cameraRadius <= 0.) {
		cameraRadius = 1.3;
	}
	
	camOriginPos.set(0,0,0);

	Node *pNode = tree[0].children[ofRandom(0, 4.9999)]->children[ofRandom(0, 4.9999)]->children[ofRandom(0, 4.9999)];
	camTargetPos.set(pNode->targetPos*cameraRadius);
	nextTargetNode = pNode;
	camOriginPos = camTargetPos;
	cameraMovementDuration = settings["camara"]["tiempo de desplazamiento"].asFloat();
	posAnim.setDuration(cameraMovementDuration);
	posAnim.setCurve(EXPONENTIAL_SIGMOID_PARAM);
	cameraDelay = settings["camara"]["delay"].asFloat();	

	visualizationDelay = settings["visualizacion"]["delay"].asFloat();	

	posAnim.animateFromTo(0., 1.);

	autoSave = settings["visualizacion"]["guardar automaticamente"].asBool();
	
	hideGui = true;


	// SPOUT stuff

	spoutTexture.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGB);
	spoutTexture.begin();
	ofClear(0);
	spoutTexture.end();

	spoutSender.init("textureOut");

}

//--------------------------------------------------------------
void ofApp::update(){	
	tree.update();

	if (ofGetLastFrameTime()<1.) {
		posAnim.update(ofGetLastFrameTime());
	}
	
	
	if (posAnim.isOrWillBeAnimating()) {
		cam.setPosition(camTargetPos*posAnim.val() + camOriginPos*(1. - posAnim.val()));
		cam.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 0));

		if (posAnim.getPercentDone() > 0.5) {
			tree.selectedNode = nextTargetNode;
		}
	}	

	ofDirectory newPhotosDir(newPhotosPath);
	newPhotosDir.allowExt("png");
	newPhotosDir.listDir();


	//Check if there are new photos available.
	for(int i=0;i<newPhotosDir.size(); i++){
		if (newPhotosDir[i].moveTo(usedPhotosPath, true, true)) {
			//check if file is loadable
			ofImage img;
			string imgPath = usedPhotosPath + "/" + newPhotosDir[i].getFileName();
			if (img.loadImage(imgPath) && img.getWidth()*img.getHeight()>0) {
				//once the file integrity is assured, proceed to add the new node in the tree
				//the node location in the tree is taken from its name (<answer1>_<answer2>_<answer3>_<timestamp>.png)
				vector<string> categories = ofSplitString(newPhotosDir[i].getFileName(), "_");
				Node *pNode = tree[0].children[ofToInt(categories[0])]->children[ofToInt(categories[1])]->children[ofToInt(categories[2])];
				tree.growChild(pNode, imgPath, cameraMovementDuration + cameraDelay + visualizationDelay);
				//tree.selectedNode = pNode;
				nextTargetNode = pNode;
				ofPoint pos = pNode->currentPos;
				camOriginPos = cam.getPosition();
				camTargetPos.set(pos*cameraRadius);
				posAnim.reset(0.);
				posAnim.animateToAfterDelay(1., cameraDelay);
				//posAnim.animateFromTo(0., 1.);

				if (autoSave) {
					tree.save();
				}				

				break;
			}
		}
	}

	// SPOUT stuff - BEGIN

	spoutTexture.begin();
	ofClear(0);


	depthOfField.begin();
	cam.begin(depthOfField.getDimensions());

	tree.draw(cam.getPosition());

	cam.end();
	depthOfField.end();
	depthOfField.getFbo().draw(0, 0);

	if (!hideGui) {
		tree.gui.draw();
	}
	//ofDrawBitmapStringHighlight(ofToString(posAnim.val()), 20,20, ofColor::red);

	spoutTexture.end();

	spoutSender.send(spoutTexture.getTextureReference());

	// SPOUT STUFF - END
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(25,0,0);
	
	// NORMAL DRAWING IN THIS APP´s WINDOW
	if (!hideGui) {
		ofSetColor(255);
		spoutTexture.draw(0, 0);
	} else {
		ofSetColor(255, 255, 0);
		ofDrawBitmapString("Sending image to Spout. Not drawing on this window.\nPress 'H' for debug", 20,20);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == ' ') {
		ofToggleFullscreen();
	}
	else if (key == 's') {
		tree.save();
	}
	else if (key == 'l') {
		tree.load(treeConfigFile);
	}
	else if (key == 'h') {		
		hideGui = !hideGui;
		if (hideGui) {
			ofHideCursor();
		}
		else {
			ofShowCursor();
		}
	}
	else {
		/*
		Node *pNode = tree[0].children[ofRandom(0, 4.9999)]->children[ofRandom(0, 4.9999)]->children[ofRandom(0, 4.9999)];
		//Node *pNode = tree[0].children[0]->children[0]->children[0];

		tree.growChild(pNode, "test.png");

		ofPoint pos = pNode->currentPos;
		camOriginPos = cam.getPosition();
		camTargetPos.set(pos*cameraRadius);
		posAnim.animateFromTo(0., 1.);
		*/
	}
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
	depthOfField.setup(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

