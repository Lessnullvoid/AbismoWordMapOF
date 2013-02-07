#include "testApp.h"

//--------------------------------------------------------------
testApp::testApp(): ofBaseApp(){ }

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	ofBackgroundHex(0x00);
	//////////// graph
	testGraphSetup();
}

//--------------------------------------------------------------
void testApp::update(){
	if(ofGetFrameNum()%120 == 0){
		cout << ofGetFrameRate() << endl;
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	long long unsigned int t0 = AbsoluteToDuration(UpTime());
	mGraph.draw();
	long long unsigned int t1 = AbsoluteToDuration(UpTime());
	cout << "draw took: " << (t1-t0) << " millis"<<endl;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void testApp::testGraphSetup(){
	vector<Node*> someNodes;
	int numNodes = 10;
	int numEdges = 80;
	for(int i=0; i<numNodes; ++i){
		Node *n = new Node("v"+ofToString(i));
		someNodes.push_back(n);
	}
	
	for(int i=0; i<numEdges; ++i){
		int edgeCost = (int)(ofRandom(2)+1);
		string edgeType = (edgeCost<2)?"cat":"tag";
		Edge *e = new Edge(edgeType+ofToString(i), edgeCost);
		int npe = (int)ofRandom(numEdges/numNodes);
		for(int j=0; j<npe; ++j){
			// pick random node
			Node *n = someNodes.at((int)ofRandom(someNodes.size()));
			n->addEdge(e);
		}
	}
	
	Node *n0 = someNodes.at((int)ofRandom(someNodes.size()));
	long long unsigned int t0 = AbsoluteToDuration(UpTime());
	mGraph.calculateDists(*n0);
	long long unsigned int t1 = AbsoluteToDuration(UpTime());
	mGraph.orderGraph();
	long long unsigned int t2 = AbsoluteToDuration(UpTime());
	//mGraph.printGraph();
	cout << "calculated from: " << n0->getName()+ " in: " << (t1-t0) << " millis"<<endl;
	cout << "ordered graph in: " << (t2-t1) << " millis"<<endl;
}

void testApp::testGraphUpdate(){
	long long unsigned int t0 = AbsoluteToDuration(UpTime());
	mGraph.calculateDists();
	long long unsigned int t1 = AbsoluteToDuration(UpTime());
	mGraph.orderGraph();
	long long unsigned int t2 = AbsoluteToDuration(UpTime());
	cout << "calculated distance in: " << (t1-t0) << " millis"<<endl;
	cout << "ordered graph in: " << (t2-t1) << " millis"<<endl;
}

// doesn't exclude empty words
string testApp::fitStringToWidth(const string s, const int w, ofTrueTypeFont ttf){
	string retStr="";
	istringstream ss( s );
	while (!ss.eof()){
		string word;
		getline(ss,word,' ');
		if(ttf.stringWidth(retStr+" "+word) > w){
			retStr += "\n";
		}
		else if(retStr.size() != 0){
			retStr += " ";
		}
		retStr += word;
	}
	return retStr;
}
