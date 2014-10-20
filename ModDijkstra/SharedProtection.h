
/*
* Megosztott v�delem implement�ci�ja.
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

// GlobelSpectrumState bar�t f�ggv�nye
//template <typename rsa=DefaultAlgorithm> 
class SharedProtection{

	Subgraph::EdgeMap<SpectrumState> *mod_spectrum_map; // m�dos�tott spektrum map: v�delmi �tvonalv�laszt�shoz, �zemi �s �zemivel k�z�s v�delmi 1 
	Subgraph::EdgeMap<double> *cost_map;  // length_map figyelembe veszi, hogy megosztand� kapacit�son megy-e
	Path<ListGraph> &allocated;   // a lefoglalat �zemi �t
	Path<ListGraph> p_path;
	ListGraph::NodeMap<bool> *node_filter;
	ListGraph::EdgeMap<bool> *arc_filter;
	Subgraph *graph;

public:

	SharedProtection( Path<ListGraph> &alloc) :allocated(alloc){
		node_filter = new ListGraph::NodeMap<bool>(*GlobalSpectrumState::getInstance().graph);
		arc_filter = new ListGraph::EdgeMap<bool>(*GlobalSpectrumState::getInstance().graph);
		makeSubgraph();
		MapFactory<Subgraph> mapf;
		mod_spectrum_map = mapf.createEdgeSpectrumMap(*graph);
		cost_map = mapf.createEdgeCostMap(*graph);
		
	}
	~SharedProtection(){
		delete node_filter;
		delete arc_filter;
		delete mod_spectrum_map;
		delete cost_map;
		delete graph;
	}
	void makeSubgraph(){

		for (ListGraph::NodeIt it(*GlobalSpectrumState::getInstance().graph); it != INVALID; ++it){ node_filter->set(it, true); }
		for (ListGraph::EdgeIt it(*GlobalSpectrumState::getInstance().graph); it != INVALID; ++it){ arc_filter->set(it, true); }
		graph = new Subgraph(*GlobalSpectrumState::getInstance().graph, *node_filter, *arc_filter);
		Path<ListGraph>::ArcIt arc_it(allocated);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = GlobalSpectrumState::getInstance().graph->target(arc_it);
			Node s = GlobalSpectrumState::getInstance().graph->source(arc_it);
			Edge e = lemon::findEdge(*GlobalSpectrumState::getInstance().graph, t, s);
			graph->disable(e);
		}		
	}

	/** Algoritmus futtat�sa
	* dijktra algorikmussal t�rt�nik a v�delmi �t v�laszt�sa
	*/
	//DEPRECATED
	void run(Node s, Node t, const int &width, const long int &timestamp){
		makeModSpectrumMap();  
		makeCostMap();
		Path<ListGraph> p_path=runDijkstra(s, t);  // �tvonalv�laszt�s
		SpectrumState spectrum=p_pathSpectrum(p_path);  //�tvonal spektruma mod_spectrum_map alapj�n
		if (GlobalSpectrumState::getInstance().checkSelector(width, spectrum)){  // spektrumallok�l�s
			p_alloc(p_path, width, timestamp, -1);
		}
		else{		
			GlobalSpectrumState::getInstance().protection_blokknum++;
		}
	}
	/** ALgoritmus futtat�sa
	*  M�dos�tott dijkstra algoritmussal �tv�laszt�s
	*/
	void runmoddijkstra(Node s, Node t, const int &width, const long int &timestamp){
		makeModSpectrumMap();  // az �zemik �s �zemivel k�z�s el� �zemik v�delme nem haszn�lhat�
		//printSpectrum(*mod_spectrum_map, graph);
		makeCostMap();  // length map a dijkstrahoz ha van el�g szabad(megoszthat�+szabad) kapacit�s kisebb a k�lts�g
		if (modDijkstra_routing(s, t, width)){  // �tvonalv�laszt�s
			p_alloc(p_path, width, timestamp, -1);  //spektrumlefoglal�s
		}

	}
	/** ALgoritmus futtat�sa
	*  K legr�videbb �t algoritmussal �tv�laszt�s
	*/
	void runKshort(Node s, Node t, const int &width, const long int &timestamp,int k0){
		makeModSpectrumMap();
		makeCostMap();
		if (kshort_routing(s, t, width,k0)){  // �tvonalv�laszt�s
			p_alloc(p_path, width, timestamp, -1);  //spektrumlefoglal�s
		}
	}

	//Dijkstra
	Path<ListGraph>  runDijkstra(Node s, Node t){	
		Dijkstra<Subgraph,Subgraph::EdgeMap<double> > dijk(*graph,*cost_map);
		dijk.run(s);
		return dijk.path(t);
	}
	// a runModDijkstra-hoz tartoz� �tvonalv�laszt�s\
	ModDijkstra algoritmssal �tvonalat keres�nk
	bool modDijkstra_routing(Node s, Node t, const int &width){
		bool switcher = false;
		ModDijkstra<Subgraph> md_dijkstra(*graph, *mod_spectrum_map);
		//md_dijkstra.initlengthMap(*cost_map);
		switcher=md_dijkstra.calcpath(s, t, width);
		p_path.clear();
		p_path=md_dijkstra.allocatedPath();	
		return switcher;
	}
	
	//
	bool kshort_routing(Node s, Node t, const int &width,int K0){
		bool switcher = false;
		Kshort<Subgraph> _ks(*graph);
		_ks.setK(K0);
		p_path.clear();
		switcher = _ks.calcPath(s, t, width, *this); //egy �tvonalat v�runk ha van, �s alloc_pos be�ll�t�s�t
		p_path.clear();
		p_path = _ks.allocatedPath();
		return switcher;
	}

	//�temi �t m�dos�t�sa
	void addAllocated(Path<ListGraph> &alloc){
		allocated = alloc;
	}
		
	//k�lts�gf�ggv�ny :\
	ahol van megoszthat� kapacit�s 0 k�lts�g ahol nincs ott egy \
	plusz egys�gnyi �lhossz k�lts�g
	void makeCostMap(){
		double cost = 0;
		double alpha = 0.5;
		double w = 0;
		double s = 0;
		for (Subgraph::EdgeIt eit(*graph); eit != INVALID; ++eit){
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

	// makeCostMap h�vja:\
	van e megoszthat� kapacit�s az �len
	bool isEnoughShareable(Subgraph::EdgeIt eit){
	
		for (int i = 0; i<GlobalSpectrumState::getInstance().spectrum_map->operator[](eit).carrier.size(); i++)
		{
			if (GlobalSpectrumState::getInstance().spectrum_map->operator[](eit).carrier[i] == -1) return true;

		}
		return false;
	}

	//\
	-1 haszn�lhat� \
	- jel�lt kapacit�s nem  haszn�lhat�\
	- 0 haszn�lhat�
	void makeModSpectrumMap(){
	
		//�zemik �ltal haszn�lt spektrumok blokkol�sa
		for (Subgraph::EdgeIt eit(*graph); eit != INVALID; ++eit){
			
			for (int i = 0; i<GlobalSpectrumState::getInstance().spectrum_map->operator[](eit).carrier.size(); i++)
				{
				if (GlobalSpectrumState::getInstance().spectrum_map->operator[](eit).carrier[i] == 1){ mod_spectrum_map->operator[](eit).carrier[i] = 1; }
			
				}
		}
		//az �zemit�l nem �ldiszjunkt �zemik v�delmij�nek blokkol�sa
		for (std::multimap<int, PathMatrix>::iterator it = GlobalSpectrumState::getInstance().path_matrix.begin(); it != GlobalSpectrumState::getInstance().path_matrix.end(); ++it){
			if (!GlobalSpectrumState::getInstance().isEdgeDisjoint(it->second.path, allocated))
			{
				ProtectionPathMatrix tempMatrix = GlobalSpectrumState::getInstance().pm[it->second.unquie_key];
						lemon::Path<ListGraph> tmp_path = tempMatrix.path;
						denyPath(tmp_path, tempMatrix.pos, tempMatrix.width);
			}			
		}
		

	}

	//�tvonal spektruma 1
	void denyPath(lemon::Path<ListGraph> &path, const int &pos, const int &width, int index = 1)
	{

		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = GlobalSpectrumState::getInstance().graph->target(arc_it);
			Node s = GlobalSpectrumState::getInstance().graph->source(arc_it);
			Edge e = lemon::findEdge(*GlobalSpectrumState::getInstance().graph, t, s);
			if (graph->status(e))
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
			Node t = GlobalSpectrumState::getInstance().graph->target(arc_it);
			Node s = GlobalSpectrumState::getInstance().graph->source(arc_it);
			Edge e = lemon::findEdge(*graph, t, s);
			spectrum.or(mod_spectrum_map->operator[](e));

		}
		return spectrum;
	}

	//protection_path_matrix felt�lt�se
	void p_insertPath(lemon::Path<ListGraph> &path, const int &width, const int &pos, const long int &timestamp)
	{
		ProtectionPathMatrix tempMatrix(path, pos, width, timestamp);
		std::pair<int, ProtectionPathMatrix> pr(GlobalSpectrumState::getInstance().global_key, tempMatrix);
		GlobalSpectrumState::getInstance().pm[GlobalSpectrumState::getInstance().global_key] = tempMatrix;
	}

	// A kiv�lasztott �tvonalon a spektrum lefoglal�sa\
	GlobalSpectrum::alloc_pos m�r kor�bban be lett �ll�tva, modDijlstraRouting ->calcpath->creatpath->checkselector
	void p_alloc(lemon::Path<ListGraph> &path, const int &width, const long int &timestamp, int index = 1)
	{
			
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = GlobalSpectrumState::getInstance().graph->target(arc_it);
			Node s = GlobalSpectrumState::getInstance().graph->source(arc_it);
			Edge e = lemon::findEdge(*GlobalSpectrumState::getInstance().graph, t, s);
			for (int i = 0; i<width; i++){
				GlobalSpectrumState::getInstance().spectrum_map->operator[](e).carrier[GlobalSpectrumState::getInstance().alloc_pos + i] = index;
			}
		}	
		p_insertPath(path, width, GlobalSpectrumState::getInstance().alloc_pos, timestamp);
		GlobalSpectrumState::getInstance().alloc_pos = -1;
	}
	
};

#endif SHARED