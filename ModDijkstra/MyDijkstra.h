
#include<lemon/list_graph.h>
#include<lemon/core.h>
#include<iostream>
#include<set>
#include<lemon/lgf_reader.h>
#include<lemon/dijkstra.h>
#include<lemon/adaptors.h>
#include<lemon/connectivity.h>
#include<lemon/path.h>
#include<lemon/maps.h>

using namespace lemon;
typedef ListGraph::Node Node;

class MyDijkstra{
	ListGraph &graph;
	std::set<ListGraph::Edge> setI;
	std::set<ListGraph::Edge> setII;
	std::set<ListGraph::Edge> setIII;
	std::set<Node> a;
	std::set<Node> b;

	//ListGraph::EdgeMap<int> map;
public:
	MyDijkstra(ListGraph g) :graph(g){

	}
	void setSource(Node s){
		for (ListGraph::NodeIt it(graph); it != INVALID; it++)
		{
			b.insert((Node)it);
		}
	}
	void run(){
		while (!b.empty())
		{
			Node u = b.begin(); // legrövidebb él
			for (u neighbours in b){
				//rövidebb-e ha igen beállítom
			}
		}
	}
	std::vector<Node> neighbours(ListGraph::Node b){
		std::vector<Node> nodeVec;
		for (ListGraph::OutArcIt it(graph, b); it != INVALID; it++){
			graph.
			nodeVec.push_back(it);
		}
	}
};