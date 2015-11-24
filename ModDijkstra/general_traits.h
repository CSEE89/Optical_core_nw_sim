#include <iostream>
#include<lemon/list_graph.h>
#include<lemon/core.h>
#include<lemon/adaptors.h>
#include<algorithm>
#include<iterator>
#include<bitset>
#include<time.h>

/*

* Author: csabi
*A spektrumszeltek defini�l�sa. Spektrumkit�lt�si strat�gi�k implement�l�sa.
* �zemi �s v�delmi utak t�rol�sa, karbantart�sa, felszabad�t�sa.
*/

#ifndef TRAITS_H
#define	TRAITS_H

using namespace lemon;
using namespace std;



typedef ListGraph::Node Node;
typedef ListGraph::Edge Edge;
class SpectrumState;
typedef Path<ListGraph> listpath; //tipusdefinici�k a ModDijkstra t�rol�si egys�geinek
typedef std::pair<listpath, SpectrumState> pathpair;
typedef std::vector<pathpair> pathpair_vector;
enum ALLOC {OneSideChannelFill,GapFill,TwoSideChannelFill};  //soektrum felt�lt�si strat�gi�k
/**
Csatorn�k sz�ma, bea�ll�tva a main.cpp ben
*/
struct CH{
	static int channel_num;	
};

static double COST_CONST = 1;
#define BITSETCNT 64

class SpectrumState{
	std::bitset<BITSETCNT> carrier;
public:
	friend class GlobalSpectrumState;
	friend class SharedProtection;
	SpectrumState()
	{
		
		carrier = std::bitset<BITSETCNT>(0);

	}

	/// K�t sz�l spektrum�nak �sszevagyol�sa, this-> carrier kapja az �sszevagyolt �rt�keket
	void or(SpectrumState &s)
	{
		carrier |= s.carrier;
	}

	/// spektrum kiirasa
	void print()
	{
		for (int i = 0; i<CH::channel_num; i++)
		{
			std::cout << carrier[i] << " ";
		}
	}

	/// Egy-egy spektrumszelet el�r�se, referenci�val t�r vissza ,hogy m�dos�that� legyen az elem
	std::bitset<BITSETCNT>::reference operator[](int i){ return carrier[i]; }
	std::bitset<BITSETCNT>::reference at(int i){ return carrier[i]; }

};
/**
* Egy sz�l vagy link spektrum�t reprezent�l� egys�g

class SpectrumState{
	std::vector<short> carrier;
public:
	friend class GlobalSpectrumState;
	friend class SharedProtection;
	SpectrumState()
	{
		std::vector<short> tmp(CH::channel_num, 0);
		carrier = tmp;

	}

	/// K�t sz�l spektrum�nak �sszevagyol�sa, this-> carrier kapja az �sszevagyolt �rt�keket
	void or(const SpectrumState &s)
	{
		for (int i = 0; i<carrier.size(); i++)
		{
			carrier[i] = (carrier[i] || s[i]); //hiba: s �res
		}
	}
	~SpectrumState()
	{
		carrier.clear();
	}
	/// spektrum kiirasa
	void print()
	{
		for (int i = 0; i<carrier.size(); i++)
		{
			std::cout << carrier[i]<<" ";
		}
	}

	SpectrumState& operator=(const SpectrumState &st){
		if (&st == this)
			return *this;
		this->carrier = st.carrier;
		return *this;
	}
	/// Egy-egy spektrumszelet el�r�se, referenci�val t�r vissza ,hogy m�dos�that� legyen az elem
	std::vector<short>::reference operator[](int i){ return carrier[i]; }
	std::vector<short>::const_reference operator[](int i)const { return carrier[i]; }
	std::vector<short>::reference at(int i){ return carrier[i]; }
	std::vector<short> data(){ return carrier; }

};
*/
// kompar�tor oszt�ly: �tvonalakat(pathpair) hasonl�t �ssze hossz szerint,\
modDijkstra haszn�lja
class comp{
public:
	bool operator()(const pathpair &a, const pathpair &b)
	{
		return a.first.length() < b.first.length();
	}
};

//\
Print f�ggv�nyek 
//�tvonal ki�r�sa
void printPath(listpath &path, const ListGraph &graph)
{
	std::cout << "[";
	PathNodeIt<Path<ListGraph> > PN(graph, path);
	for (PN; PN != INVALID; PN++)
	{
		std::cout << ", " << graph.id(PN);
	}

	std::cout << "]";
	std::cout << std::endl;
}
void printSet(pathpair lp, const ListGraph &graph)
{
	lemon::Path<ListGraph> path;
	path = lp.first;
	std::cout << "[";
	PathNodeIt<Path<ListGraph> > PN(graph, path);
	for (PN; PN != INVALID; PN++)
	{
		std::cout << ", " << graph.id(PN);
	}

	std::cout << "]";
	std::cout << std::endl;
}

void printNode(const pathpair_vector &vec, const ListGraph &graph)
{
	for (pathpair_vector::const_iterator cvi = vec.begin(); cvi != vec.end(); cvi++)
	{
		lemon::Path<ListGraph> path;
		path = cvi->first;
		std::cout << "[";
		PathNodeIt<Path<ListGraph> > PN(graph, path);
		for (PN; PN != INVALID; PN++)
		{
			std::cout << ", " << graph.id(PN);
		}

		std::cout << "]" << std::endl;
	}


}

void printSpectrum(ListGraph::EdgeMap<SpectrumState> &spectrum_map, const ListGraph &g)
{
	ListGraph::EdgeIt eit(g);
	for (eit; eit != INVALID; ++eit)
	{
		spectrum_map[eit].print();
		std::cout << std::endl;
	}

}

class PathNodes{
	std::vector<Node> p_nodes;
	
public:
	PathNodes(Path<ListGraph> path, const ListGraph &graph){
		for (PathNodeIt<Path<ListGraph> > pnit(graph, path); pnit != INVALID; ++pnit){
			p_nodes.push_back(pnit);
		}
	}
	Node at(int i){
		return p_nodes[i];
	}
	Node endNode(){
		return p_nodes.back();
	}
	Node beginNode(){
		return p_nodes.front();
	}
private:
	PathNodes(){}
};

class Stopper{
	long time;
	long elapsed=0;
public:
	Stopper(){}
	void start(){ time = clock(); }
	void stop(){ elapsed += clock() - time; }
	long getTime(){ return clock() - time; }
	long getElapsed(){ return elapsed; }
	void reset(){ time = 0; elapsed = 0; }
};


#endif TRAITS_H