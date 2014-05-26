
/*
* Útvonalválasztó algortimusok és egyéb függvények.
* Moddijkstra a LEMON/dijkstra módostított állományt igényli
*
*
*/

#ifndef UTILITIES_H
#define	UTILITIES_H


#include <cstdlib>
#include<lemon/graph_to_eps.h>
#include<lemon/list_graph.h>
#include<lemon/core.h>
#include<iostream>
#include<set>
#include<lemon/lgf_reader.h>
#include<lemon/dijkstra.h>
#include<lemon/adaptors.h>
#include<lemon/connectivity.h>
#include<lemon/path.h>
#include"stl_out.h"
#include"Kshort_mod.h"


using namespace lemon;

typedef ListGraph::Node Node;
typedef ListGraph::Edge Edge;
typedef SubGraph<ListGraph, ListGraph::NodeMap<bool>, ListGraph::EdgeMap<bool> > Subgraph;

 template <typename gr> void test(gr &g)
{
	
	for (gr::NodeIt n(g); n != INVALID; ++n){ cout << g.id(n); }
}
/*
Párhuzamos élek kitörlése a LEMON::ListGraph tipusu gráfból

*/
class Deparallel{
public:
    void operator()(ListGraph &graph)
    {   
        Node u,v;
        
        for(ListGraph::EdgeIt eit(graph);eit!=INVALID;++eit)
         {
          u=graph.u(eit);
          v=graph.v(eit);
          
          for (ConEdgeIt<ListGraph> it(graph,u,v); it != INVALID; ++it) 
            {
             if(it!=eit)graph.erase(eit);
            }
        }
    }
};

/*
K-shortest path algoritmus beállítása
*/
template<typename GR>class Kshort : public KShortestPath<GR>, public DefaultAlgorithm{
	//const std::vector< std::vector<int> > &Kshortvec; //
	//const ListGraph &graph;  //main-ben lévő globális gráf
	GlobalSpectrumState &global;
	std::vector<listpath> pathvec;  //Kshrot által megtalált utak halmaza
	Path<GR> allocated;
	int K;
public:
	friend class MakeSubgraph;
	Kshort(GR &graph, std::vector< std::vector<int> > &vec, GlobalSpectrumState &global) :KShortestPath(graph, vec), global(global), pathvec()
	{
		
	}
	void setK(int &k){
		K = k;
	}
	void run(Node s, Node t, const int &width, const long int &timestamp){
		A.clear();
		YenKshort(s, t, K);
		conv();
		alloc(width, timestamp);
		A.clear();
	}

	void runSameSpect(int width, long int dur){
		conv();
		alloc(width, dur);
	}
	//visszaadja a lefoglalt utat
	Path<GR> allocatedPath(){
		return allocated;
	}
private:

    void conv() // pathvec-be kimenti a Kshort algoritmus által megtalált utakat Kshortvecbol
	{
		Path<ListGraph> tp;
	//std::vector<lemon::Path<temppath> > pathvec;
		for(std::vector<std::vector<int> >::const_iterator it=A.begin();it!=A.end();it++)
		{
			tp.clear();
			for(int i=1;i<it->size();i++)
			{
				
				
				int j=i-1;
				Node s=graph.nodeFromId(it->at(i));
				Node t=graph.nodeFromId(it->at(j));
				ListGraph::Arc arc=lemon::findArc(graph,s,t);
				tp.addBack(arc);
				
			}
			pathvec.push_back(tp);
		}
		
	}
	
	/*
	alloc-ban hívódik meg
	Végiglépked az útvonalakaon ahol van szabad spektrum tmpPath-ba betölti az utat, visszatér ture-val
	*/
	bool createPath(const int &width,Path<ListGraph> &tmpPath)
	{
		for(int i=0;i<pathvec.size();i++)
		{
			
			SpectrumState spectrum=global.PathSpectrum(pathvec[i]); //KOPI KONSTRUKTOR
			//Path<ListGraph> path(it->first);
			if(global.checkSelector(width,spectrum))
			{
				tmpPath=(lemon::Path<ListGraph>)pathvec[i];
				return true;
			}
			
		}
		return false;
	}
	/*
	Meghívja a global.Alloc-ot
	*/
	bool alloc(const int &width,const long int &timestamp)
	{
		lemon::Path<ListGraph> tmpPath;  //createpath-nak átadva 
		if(createPath(width,tmpPath))
		{
			allocated = tmpPath;
			

			if (GlobalSpectrumState::protection_round == false){
				global.Alloc(tmpPath, width, timestamp);
			}
			else if (GlobalSpectrumState::protection_round == true)
			{
				global.Alloc(tmpPath, width, timestamp,3);
			}
			return true;
		}
		else{
			GlobalSpectrumState::isblocked = true;
			if (GlobalSpectrumState::protection_round == false){
				GlobalSpectrumState::blokknum++;
			}
			else if (GlobalSpectrumState::protection_round == true)
			{
				GlobalSpectrumState::protection_blokknum++;
			}
			return false;
		}
	}

};


template <typename T>class MapFactory{
public:
	typedef T GR;
	typedef typename GR::template EdgeMap<bool> pMap; //permitting map
	typedef typename GR::template NodeMap<pathpair_vector> path_Map;  //path map
	typedef typename GR::template EdgeMap<SpectrumState> edge_spectrum_Map;
	typedef typename GR::template EdgeMap<double> cost_Map;
	pMap *createPermittingmap(const GR &g)
	{
		GR::EdgeMap<bool>* tmp = new GR::EdgeMap<bool>(g);
		for(GR::EdgeIt it(g);it!=INVALID;++it){
		tmp->set(it,true);
		}
		return tmp;
	}
	path_Map* createPathMap(const GR &g)
	{
	
		return new GR::NodeMap<pathpair_vector>(g);
	}
	edge_spectrum_Map* createEdgeSpectrumMap(const GR &g)
	{

		return new GR::EdgeMap<SpectrumState>(g);
	}
	cost_Map* createEdgeCostMap(const GR &g)
	{

		return new GR::EdgeMap<double>(g);
	}

};

class comp{
public:
	bool operator()(const pathpair &a, const pathpair &b)
	{
		return a.first.length() < b.first.length();
	}
};
// NEW-t tartalmazó osztályokba másoló konstrzktor és op= KELL
template <typename T> class ModDijkstra: public DefaultAlgorithm
 {
  typedef T GR;
  typedef typename GR::template EdgeMap<bool> pMap; //permitting map
  typedef typename GR::template NodeMap<pathpair_vector> path_Map;  //path map
  typedef typename GR::template EdgeMap<int> intMap;
  pMap* permittingmap; //
  path_Map *pathmap; //tárolja a nodeokban a dijkstra futás során tárolt értékeketet, útvonalak, és ahhoz tartozó spektrum
  GR &graph;
  ListGraph::EdgeMap<SpectrumState> &spectrum_state;  //globális spektrum
  std::multiset<pathpair,comp> _set;  //a két csomópont között megtalált útvonalak hossz szerint, (2 szer fut a dijkstra a 2 irányra ezeket pakolja bele)
  GlobalSpectrumState &globalspectrum;
  Path<GR> allocated;
  //listpath allocated;
public:
	friend class MakeSubgraph;
	ModDijkstra(GR &_graph,ListGraph::EdgeMap<SpectrumState> &sp,GlobalSpectrumState &gs):graph(_graph),spectrum_state(sp),globalspectrum(gs)
	{
		MapFactory<GR> mapf;
		permittingmap = mapf.createPermittingmap(graph);
		pathmap = mapf.createPathMap(graph);
	}

	~ModDijkstra()
	{
		delete permittingmap;
		delete pathmap;
	}
	//\
		feltölti a _set változót útvonalakkal, oda-vissza fut \
		createpath -et hvja

	void run(Node s, Node t, const int &width, const long int &timestamp)
	{
		int pos(0);
		intMap lengthmap(graph);
		for (GR::EdgeIt it(graph); it != INVALID; ++it){
			lengthmap.set(it, 1);
		}
		lemon::csabi::Dijkstra<GR, intMap> dijkstra(graph, lengthmap, spectrum_state);
		setperm(width);
		dijkstra.init();
		dijkstra.modaddSource(s, *pathmap);
		dijkstra.modstart(*permittingmap, *pathmap);


		pathpair_vector temp(pathmap->operator[](t));

		this->init();
		setperm(width);
		dijkstra.init();
		dijkstra.modaddSource(t, *pathmap);
		dijkstra.modstart(*permittingmap, *pathmap);
		setfill(pathmap->operator[](s), temp);

		listpath tmpPath;

		if (createPath(width, tmpPath)){
			allocated = tmpPath;
			if (GlobalSpectrumState::protection_round == false){
				globalspectrum.Alloc(tmpPath, width, timestamp);
			}
			else if (GlobalSpectrumState::protection_round == true)
			{
				globalspectrum.Alloc(tmpPath, width, timestamp, 3);
			}
		}
		else{
			GlobalSpectrumState::isblocked = true;
			if (GlobalSpectrumState::protection_round == false){
				GlobalSpectrumState::blokknum++;
			}
			else if (GlobalSpectrumState::protection_round == true)
			{
				GlobalSpectrumState::protection_blokknum++;
			}
		}
		this->init();

	}

	void printAllocated(){
		for (Path<GR>::ArcIt it(allocated); it != INVALID; ++it) cout << graph.id(it) / 2 << " ";
		cout << endl;
	}
	void printAllocatedNodes(){
		for (PathNodeIt<Path<GR> > it(graph, allocated); it != INVALID; ++it) cout << graph.id(it) << " ";
		cout << endl;
	}
	Path<GR> allocatedPath(){
		return allocated;
	}


 private:

	void init()
	{
		GR::NodeIt it(graph);
			for(it;it!=INVALID;++it)
		{
			pathmap->operator[](it).clear();
		}
		delete pathmap;
		MapFactory<GR> mapf;
		this->pathmap=mapf.createPathMap(graph);
		GR::EdgeIt eit(graph);
		_set.clear();

	}
	/**
	Két útvonalhalmazt ad össze és rendez hossz szerint, majd bemásolja a halmaz tárolóba
	*/
	void setfill(pathpair_vector &t1, pathpair_vector &t2)
	{
		
		for(int i=0;i<t2.size();i++)
			{
				t1.push_back(t2[i]);
				
			}
		for(int i=0;i<t1.size();i++){
		_set.insert(t1[i]);				
		}
	}

	void setperm(const int &width)
	{
		for(GR::EdgeIt it(graph);it!=INVALID;++it) //végigjárjuk az összes élet és megnézzük van e elég hely
		{
			permittingmap->operator[](it)=GlobalSpectrumState::SetPermittingMap(width,spectrum_state[it]);
		}
	}

	//\
	Végigmegyünk az utvonalhalmazon amit megtalált az algoritmus, kiválasztjuk azt az ótvonalat amin az spektrumallokálási módszer talál szabad sávot
	
	bool createPath(int width,Path<ListGraph> &tmpPath)
	{
		for(std::multiset<pathpair,comp>::iterator it=_set.begin();it!=_set.end();it++)
		{
			
			SpectrumState spectrum(it->second); //KOPI KONSTRUKTOR
			
			if(globalspectrum.checkSelector(width,spectrum))
			{
				tmpPath=(lemon::Path<ListGraph>)it->first;
				return true;
			}
			
		}
		return false;
	}


};

//typedef ListGraph::EdgeMap<bool> permMap;

class MakeSubgraph{
	
	ListGraph &graph;
	Path<ListGraph> &allocated;
	ListGraph::EdgeMap<SpectrumState> &spectrum_state;
	ListGraph::NodeMap<bool> *node_filter;
	ListGraph::EdgeMap<bool> *arc_filter;
	Subgraph::EdgeMap<SpectrumState> *tempMap;
public:
	MakeSubgraph(ModDijkstra<ListGraph> &_moddijkstra) :graph(_moddijkstra.graph),allocated(_moddijkstra.allocated),spectrum_state(_moddijkstra.spectrum_state){		
		node_filter = new ListGraph::NodeMap<bool>(graph);
		arc_filter = new ListGraph::EdgeMap<bool>(graph);
	}
	MakeSubgraph(Kshort<ListGraph> &ks, ListGraph::EdgeMap<SpectrumState> &st) :graph(ks.graph), allocated(ks.allocated), spectrum_state(st){
		node_filter = new ListGraph::NodeMap<bool>(graph);
		arc_filter = new ListGraph::EdgeMap<bool>(graph);
	}
	MakeSubgraph(ListGraph &kgraph, Path<ListGraph> kallocated, ListGraph::EdgeMap<SpectrumState> &st) :graph(kgraph), allocated(kallocated), spectrum_state(st){
		node_filter = new ListGraph::NodeMap<bool>(graph);
		arc_filter = new ListGraph::EdgeMap<bool>(graph);
	}

	~MakeSubgraph(){
	
		delete node_filter;
		delete arc_filter;
		delete tempMap;
	}
	Subgraph run(){

		for (ListGraph::NodeIt it(graph); it != INVALID; ++it){ node_filter->set(it, true); }
		for (ListGraph::EdgeIt it(graph); it != INVALID; ++it){ arc_filter->set(it, true); }
		Subgraph sub(graph, *node_filter, *arc_filter);
		Path<ListGraph>::ArcIt arc_it(allocated);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph.target(arc_it);
			Node s = graph.source(arc_it);
			Edge e = lemon::findEdge(graph, t, s);
			sub.disable(e);
		}
		return sub;
	}
	// Subgraph-hoz tartozó spektrumállapot "EdgeMap" létrehozása
	Subgraph::EdgeMap<SpectrumState>* makeSpectrum(Subgraph &subgraph){
		Subgraph::EdgeMap<SpectrumState> *tempMap = new Subgraph::EdgeMap<SpectrumState>(subgraph);
		for (Subgraph::EdgeIt eit(subgraph); eit != INVALID; ++eit){
			if (subgraph.id(eit) == graph.id(eit)){
				tempMap->operator[](eit) = spectrum_state[eit];
				//std::cout << i++;
			}
		}
		return tempMap;
	}
	
};


#endif	/* UTILITIES_H */
