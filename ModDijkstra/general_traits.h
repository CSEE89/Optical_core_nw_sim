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
*A spektrumszeltek definiálása. Spektrumkitöltési stratégiák implementálása.
* Üzemi és védelmi utak tárolása, karbantartása, felszabadítása.
*/

#ifndef TRAITS_H
#define	TRAITS_H

using namespace lemon;
using namespace std;



typedef ListGraph::Node Node;
typedef ListGraph::Edge Edge;
class SpectrumState;
typedef Path<ListGraph> listpath; //tipusdefiniciók a ModDijkstra tárolási egységeinek
typedef std::pair<listpath, SpectrumState> pathpair;
typedef std::vector<pathpair> pathpair_vector;
enum ALLOC {OneSideChannelFill,GapFill,TwoSideChannelFill};  //soektrum feltöltési stratégiák
/**
Csatornák száma, beaállítva a main.cpp ben
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

	/// Két szál spektrumának összevagyolása, this-> carrier kapja az összevagyolt értékeket
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

	/// Egy-egy spektrumszelet elérése, referenciával tér vissza ,hogy módosítható legyen az elem
	std::bitset<BITSETCNT>::reference operator[](int i){ return carrier[i]; }
	std::bitset<BITSETCNT>::reference at(int i){ return carrier[i]; }

};
/**
* Egy szál vagy link spektrumát reprezentáló egység

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

	/// Két szál spektrumának összevagyolása, this-> carrier kapja az összevagyolt értékeket
	void or(const SpectrumState &s)
	{
		for (int i = 0; i<carrier.size(); i++)
		{
			carrier[i] = (carrier[i] || s[i]); //hiba: s üres
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
	/// Egy-egy spektrumszelet elérése, referenciával tér vissza ,hogy módosítható legyen az elem
	std::vector<short>::reference operator[](int i){ return carrier[i]; }
	std::vector<short>::const_reference operator[](int i)const { return carrier[i]; }
	std::vector<short>::reference at(int i){ return carrier[i]; }
	std::vector<short> data(){ return carrier; }

};
*/
// komparátor osztály: útvonalakat(pathpair) hasonlít össze hossz szerint,\
modDijkstra használja
class comp{
public:
	bool operator()(const pathpair &a, const pathpair &b)
	{
		return a.first.length() < b.first.length();
	}
};

//\
Print függvények 
//Útvonal kiírása
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