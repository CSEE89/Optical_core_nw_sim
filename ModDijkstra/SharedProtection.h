
/*
* Megosztott védelem implementációja.
*
*/

#ifndef SHARED
#define SHARED

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
#include"utilities.h"

using namespace std;

/*
1, run
2, MakeModspectrumMap
3, MakeCostMap
4, runDijkstra
5, alloc
*/

// GlobelSpectrumState barát függvénye
class SharedProtection: public DefaultAlgorithm{

	GlobalSpectrumState &global;
	ListGraph::EdgeMap<SpectrumState> *mod_spectrum_map;
	ListGraph::EdgeMap<double> *cost_map;
	Path<ListGraph> &allocated;
	

public:
	static int cnt;

	 //védelmi utak
	
	SharedProtection(GlobalSpectrumState &gb, Path<ListGraph> &alloc) :global(gb),allocated(alloc){
		MapFactory<ListGraph> mapf;
		mod_spectrum_map = mapf.createEdgeSpectrumMap(global.graph);
		cost_map = mapf.createEdgeCostMap(global.graph);
		cnt++;
	}
	~SharedProtection(){
		delete mod_spectrum_map;
		delete cost_map;
	}

	// algoritmus futtatása, hvja a többi függvényt
	void run(Node s, Node t, const int &width, const long int &timestamp){
		makeModSpectrumMap();
	
		makeCostMap();
		Path<ListGraph> p_path=runDijkstra(s, t);
		SpectrumState spectrum=p_pathSpectrum(p_path);
		if (global.checkSelector(width, spectrum)){
			p_alloc(p_path, width, timestamp, -1);
		}
		else{		
			global.protection_blokknum++;
		}
	}
	
	//Dijkstra
	Path<ListGraph>  runDijkstra(Node s, Node t){
		MakeSubgraph makesub(global.graph, allocated, global.spectrum_map);
		Subgraph subgraph = makesub.run();
		Subgraph::EdgeMap<double> lengthmap(subgraph);
		for (Subgraph::EdgeIt it(subgraph); it != INVALID; ++it){
			
				lengthmap.set(it,cost_map->operator[](it));			
		}
		Dijkstra<Subgraph,Subgraph::EdgeMap<double> > dijk(subgraph,lengthmap);
		dijk.run(s);
		return dijk.path(t);
	}

	void addAllocated(Path<ListGraph> &alloc){
		allocated = alloc;
	}
		
	//költségfüggvény :\
	ahol van megosztható kapacitás 0 költség ahol nincs ott egy \
	plusz egységnyi élhossz költség
	void makeCostMap(){
		double cost = 0;
		double alpha = 0.5;
		double w = 0;
		double s = 0;
		for (ListGraph::EdgeIt eit(global.graph); eit != INVALID; ++eit){
			//van-e megosztott 
			if (isEnoughShareable(eit))
			{
				w = 0;
			}
			else{ w = 1; }
				s = 1;
			cost = w*alpha + (1 - alpha)*s;
			cost_map->operator[](eit) = cost;
		}
	}

	// makeCostMap hívja:\
	van e megosztható kapacitás
	bool isEnoughShareable(ListGraph::EdgeIt eit){
	
		for (int i = 0; i<global.spectrum_map[eit].carrier.size(); i++)
		{
			if (global.spectrum_map[eit].carrier[i] == -1) return true;

		}
		return false;
	}

	//\
	-1 használható \
	- jelölt kapacitás nem  használható\
	- 0 használható
	void makeModSpectrumMap(){
	
		//üzemi által használt spektrumok blokkolása
		for (ListGraph::EdgeIt eit(global.graph); eit != INVALID; ++eit){
			
				for (int i = 0; i<global.spectrum_map[eit].carrier.size(); i++)
				{
					if (global.spectrum_map[eit].carrier[i] == 1){ mod_spectrum_map->operator[](eit).carrier[i] = 1; }
			
				}
		}
		//az üzemitõl nem éldiszjunkt üzemik védelmijének blokkolása
		for (std::multimap<int, PathMatrix>::iterator it = global.path_matrix.begin(); it != global.path_matrix.end(); ++it){
			if (!global.isEdgeDisjoint(it->second.path, allocated))
			{
				for (int i = 0; i < global.pm.size(); i++){
					if (global.pm[i].first == it->second.unquie_key){
						ProtectionPathMatrix tempMatrix = global.pm[i].second;
						lemon::Path<ListGraph> p_path = tempMatrix.path;
						denyPath(p_path, tempMatrix.pos, tempMatrix.width);
					}
				}
				//ProtectionPathMatrix tempMatrix = global.protection_path_matrix[it->second.unquie_key];
				
			}
		}

	}


	void denyPath(lemon::Path<ListGraph> &path, const int &pos, const int &width, int index = 1)
	{

		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = global.graph.target(arc_it);
			Node s = global.graph.source(arc_it);
			Edge e = lemon::findEdge(global.graph, t, s);
			for (int i = 0; i<width; i++){
				mod_spectrum_map->operator[](e).carrier[pos + i] = index;
			}
		}
		//printSpectrum(*mod_spectrum_map, global.graph);
	}




	SpectrumState p_pathSpectrum(const Path<ListGraph> &path)
	{
		SpectrumState spectrum;
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = global.graph.target(arc_it);
			Node s = global.graph.source(arc_it);
			Edge e = lemon::findEdge(global.graph, t, s);
			spectrum.or(mod_spectrum_map->operator[](e));

		}
		return spectrum;
	}

	//protection_path_matrix feltöltése
	void p_insertPath(lemon::Path<ListGraph> &path, const int &width, const int &pos, const long int &timestamp)
	{
		ProtectionPathMatrix tempMatrix(path, pos, width, timestamp);
		std::pair<int, ProtectionPathMatrix> pr(global.global_key, tempMatrix);
		global.pm.push_back(pr);
		//global.protection_path_matrix.insert(pr);
	}


	void p_alloc(lemon::Path<ListGraph> &path, const int &width, const long int &timestamp, int index = 1)
	{
			
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = global.graph.target(arc_it);
			Node s = global.graph.source(arc_it);
			Edge e = lemon::findEdge(global.graph, t, s);
			for (int i = 0; i<width; i++){
				global.spectrum_map[e].carrier[global.alloc_pos + i] = index;
			}
		}	
		p_insertPath(path, width, global.alloc_pos, timestamp);
		global.alloc_pos = -1;
	}
	
};

#endif SHARED