#include "GraphUtils.h"
#include "ofAppRunner.h"
#include "ofGraphics.h"
#include "ofLog.h"

#define MAX_FONT_SIZE 64
#define MIN_FONT_SIZE 16
#define FONT_NAME "verdana.ttf"

map<int,ofTrueTypeFont> PhysNode::fontMap = map<int,ofTrueTypeFont>();

PhysNode::PhysNode(){
	size = 20;
	name = "";
	pos = ofVec2f(ofRandom(ofGetWidth()),ofRandom(ofGetHeight()));
	vel = ofVec2f(0,0);
	if(fontMap.size() < 1){
		for(int i=0; i<11; ++i){
			int fs = i*5+MIN_FONT_SIZE;
			ofTrueTypeFont ottf;
			ottf.loadFont(FONT_NAME, fs, true, true);
			fontMap[fs] = ottf;
		}
	}
}
PhysNode::~PhysNode(){}

void PhysNode::setVelocity(const ofVec2f& vel_){
	vel = vel_;
}
void PhysNode::setSize(const float size_){
	size = size_;
}
const string PhysNode::getName() const{
	return name;
}
const ofVec2f& PhysNode::getPos() const{
	return pos;
}
const float& PhysNode::getSize() const{
	return size;
}

const ofRectangle PhysNode::getBoundingBox() const{
	return ofRectangle(pos.x-size/2, pos.y-size/2, size, size);
}

void PhysNode::draw(){
	ofFill();
	ofSetColor(100,100);
	ofCircle(pos,size/2);
	ofNoFill();
	ofSetColor(255);
	ofCircle(pos,size/2);

	std::map<int,ofTrueTypeFont>::iterator it = fontMap.lower_bound((int)10);
	ofTrueTypeFont mFont = (it->second);
	mFont.drawString(name, pos.x, pos.y);
}


inline const bool PhysNode::isMouseInside(ofMouseEventArgs & args) const{
	return ((args.x > (pos.x-size/2)) && (args.x < (pos.x+size/2)) && (args.y > (pos.y-size/2)) && (args.y < (pos.y+size/2)));
}

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
ofEvent<Node> Node::addNodeToGraph = ofEvent<Node>();

Node::Node(const string name_): PhysNode(){
	name = name_;
	distance = 1e9;
	ofNotifyEvent(Node::addNodeToGraph, *this);
	ofRegisterMouseEvents(this);
}

Node::~Node(){
	ofUnregisterMouseEvents(this);
}

bool Node::operator < (const Node &on) const{
	return distance < on.distance;
}

bool Node::sortComp(Node *n0, Node *n1){
	return *n0 < *n1;
}

void Node::setDistance(float f){
	distance = f;
}
const float Node::getDistance() const{
	return distance;
}
const bool Node::isInQ() const{
	return bInQ;
}
void Node::setInQ(const bool q){
	bInQ = q;
}

const map<string, Edge*>& Node::getEdges() const{
	return theEdges;
}

void Node::process() const{
	// send my distance to all edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		(it->second)->setCost(distance);
	}
}

// add edge to map. use name as key.
void Node::addEdge(Edge* e){
	if(theEdges.find(e->getName()) == theEdges.end()){
		theEdges[e->getName()] = e;
		// link back to node...
		e->addNode(this);
	}
}

void Node::mouseMoved(ofMouseEventArgs & args){}
void Node::mouseDragged(ofMouseEventArgs & args){}
void Node::mousePressed(ofMouseEventArgs & args){
	if(this->isMouseInside(args)){
		// DEBUG
		size += 10;
		ofNotifyEvent(NodeClickEvent, *this);
	}
}
void Node::mouseReleased(ofMouseEventArgs & args){}

//////////////////////////////////
//////////////////////////////////
//////////////////////////////////
ofEvent<Node> Edge::addNodeToQ = ofEvent<Node>();
ofEvent<Edge> Edge::addEdgeToGraph = ofEvent<Edge>();

Edge::Edge(const string name_, const int cost_): PhysNode(){
	name = name_;
	cost = cost_;
	minCost = 1e9;
	ofNotifyEvent(Edge::addEdgeToGraph, *this);
	ofRegisterMouseEvents(this);
}
Edge::~Edge(){
	ofUnregisterMouseEvents(this);
}

bool Edge::operator < (const Edge &oe) const{
	return minCost < oe.minCost;
}

bool Edge::sortComp(Edge *e0, Edge *e1){
	return *e0 < *e1;
}

void Edge::setCost(const float td){
	// if there's a shorter way to get here, update minCost
	if(td+avgCost < minCost){
		minCost = td+avgCost;
		
		// send new cost to all nodes
		for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
			if(minCost+avgCost < (it->second)->getDistance()){
				(it->second)->setDistance(minCost+avgCost);
				// Add node to graph Q
				(it->second)->setInQ(true);
				ofNotifyEvent(Edge::addNodeToQ, *(it->second));
			}
		}
	}
}

void Edge::resetMinCost(){
	minCost = 1e9;
}

float Edge::getCost() const{
	return minCost;
}

const map<string, Node*>& Edge::getNodes() const{
	return theNodes;
}

void Edge::addNode(Node* n){
	if(theNodes.find(n->getName()) == theNodes.end()){
		theNodes[n->getName()] = n;
		avgCost = cost/theNodes.size();
	}
}

// click events for triggering sub-menu
void Edge::mouseMoved(ofMouseEventArgs & args){}
void Edge::mouseDragged(ofMouseEventArgs & args){}
void Edge::mousePressed(ofMouseEventArgs & args){
	if(this->isMouseInside(args)){
		// DEBUG
		size += 10;
		ofNotifyEvent(EdgeClickEvent, *this);
	}
}
void Edge::mouseReleased(ofMouseEventArgs & args){}

//////////////////////////////////////
//////////////////////////////////////
//////////////////////////////////////

// adds every created node and edge to graph using listeners. ????
Graph::Graph(){
	ofAddListener(Edge::addNodeToQ, this, &Graph::addNodeToQ);
	ofAddListener(Node::addNodeToGraph, this, &Graph::addNodeToGraph);
	ofAddListener(Edge::addEdgeToGraph, this, &Graph::addEdgeToGraph);
}

Graph::~Graph(){
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		delete it->second;
	}
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		delete it->second;
	}
}

void Graph::addNodeToGraph(Node& n){
	theNodes[n.getName()] = &n;
	orderedNodes.push_back(&n);
	// TODO: when clicked, not only calculate distances, but also:
	//       (1) update PhysNode sizes
	ofAddListener(n.NodeClickEvent, this, &Graph::calculateDists);
}
void Graph::addEdgeToGraph(Edge& e){
	theEdges[e.getName()] = &e;
	orderedEdges.push_back(&e);
	// TODO: what to do when edge is clicked? open a sub menu?
	// ofAddListener(e.EdgeClickEvent, this, &Graph::openSubMenu);
}

void Graph::addNodeToQ(Node& n){
	// add to Q
	theQ.push(&n);
}

void Graph::calculateDists(Node& fromNode){
	// clear costs from nodes
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		(it->second)->setDistance(1e9);
	}
	// clear minDist from edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		(it->second)->resetMinCost();
	}
	
	// push root calculate dists.
	fromNode.setDistance(0);
	theQ.push(&fromNode);
	
	while(!theQ.empty()){
		Node n = *(theQ.front());
		n.setInQ(false);
		theQ.pop();
		n.process();
	}
}

void Graph::orderGraph(){
	// check sizes
	if(orderedNodes.size() != theNodes.size()){
		ofLogWarning("orderedNodes and theNodes are inconsistent");
		orderedNodes.clear();
		for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
			orderedNodes.push_back(it->second);
		}
	}
	if(orderedEdges.size() != theEdges.size()){
		ofLogWarning("orderedEdges and theEdges are inconsistent");
		orderedEdges.clear();
		for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
			orderedEdges.push_back(it->second);
		}
	}
	// sort
	sort(orderedNodes.begin(), orderedNodes.end(), Node::sortComp);
	sort(orderedEdges.begin(), orderedEdges.end(), Edge::sortComp);
}

// physical functions
void Graph::draw(){
	// draw Edges
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		Edge* e = it->second;
		if(e){
			e->draw();
		}
	}
	// draw Nodes
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		Node* n = it->second;
		if(n){
			n->draw();
		}
	}
}

// for DEBUG
void Graph::calculateDists(){
	int mi = (int)ofRandom(theNodes.size());
	int i = 0;
	Node *n0 = NULL;
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end()&&i<mi; ++it,++i){
		n0 = it->second;
	}
	if(n0){
		long long unsigned int t0 = AbsoluteToDuration(UpTime());
		calculateDists(*n0);
		long long unsigned int et = AbsoluteToDuration(UpTime())-t0;
		cout << "calculated from: " << n0->getName()+ " in: " << et << " millis"<<endl;
	}
}

// for DEBUG
void Graph::printGraph() const{
	ofLogWarning("Nodes:")<< "";
	for (map<string,Node*>::const_iterator it=theNodes.begin(); it!=theNodes.end(); ++it){
		ofLogWarning() << it->first << ": " << (it->second)->getDistance();
	}
	ofLogWarning("Edges:") << "";
	for (map<string,Edge*>::const_iterator it=theEdges.begin(); it!=theEdges.end(); ++it){
		ofLogWarning() << it->first << ": " << (it->second)->getCost();
	}
}
