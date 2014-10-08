#include "stdafx.h"
#include"utilities.h"
#include"SharedProtection.h"
#include<lemon/random.h>
#define REQUESTS 1000
//#include"Dijkstraplus.h"

using namespace std;
using namespace lemon;
/*
*      a dijkstraba vhogy tiltást kell tenni egy adott élre vonatkozólag
*      TIPS:
*              -lemon/dijkstra lengthmap-el párhuzamosan engedélyző map
*
*
* ModDijkstra:
*      -kupac adatszerk megért(wiki)
*      -lemon/heap átnéz
*
*
*/


bool GlobalSpectrumState::protection_round(false);
int GlobalSpectrumState::alloc_pos(0);
int GlobalSpectrumState::global_key(0);
int GlobalSpectrumState::blokknum(0);
int GlobalSpectrumState::protection_blokknum(0);


int CH::channel_num(35);

void simulationDedicatedModdijk(ListGraph &graph,  Random &random);
void simulationSharedKshort(ListGraph &graph,  Random &random);
void simulationSharedModdijkstra(ListGraph &graph, Random &random);
int main() {

	int ch_num_tomb[] = { 60, 50, 60 };

	/**
	* gráf beolvasás lgf fileból
	* irányítatlan gráfot használunk
	*/
			typedef ListGraph::Node Node;
			typedef ListGraph::Edge Edge;
			typedef PathNodeIt<Path<ListGraph> > PN;
			ListGraph graph;
			graphReader(graph, "28_eu.lgf").run(); //.edgeMap("cost",lengthmap)
			ListGraph::EdgeMap<int> lengthmap(graph);
			ListGraph::EdgeMap<bool> permittingmap(graph);
			
			GlobalSpectrumState::getInstance().setGraph(graph);  // globál spectrum, allokállás menedzselése
			GlobalSpectrumState::getInstance().ALLOCMOD = OneSideChannelFill;   // spektrul foglalási strat beállítása
			
			Deparallel para;
			para(graph);
			GlobalSpectrumState::blokknum = 0;    // bolokkolások számát tároló változó incializálása
			GlobalSpectrumState::protection_blokknum = 0;
			Random random;

			  
			int counter = 0;
			long int dursum(0), durcnt(0);
			int n1(0), n2(0), width1(0), k(0); long int dur = 0;
			int K0 = 1;
			lemon::Random random1(random);

			/**
			* Módosított dijkstra algoritmussal futó üzemi és védelmi útválasztás szimulációja
			*/
			simulationDedicatedModdijk(graph, random); /// módosított dijkstra
				//Globalspectrum visszaállítása kezdeti állapotba
			GlobalSpectrumState::getInstance().clear();
			simulationSharedModdijkstra(graph,  random);
			GlobalSpectrumState::getInstance().clear();
			simulationSharedKshort(graph, random);
			/**Kshortest
			*/
			GlobalSpectrumState::getInstance().clear();
			int tomb[] = { 1, 2, 5, 10, 20 };
			for (int ii = 0; ii < 5; ii++)
			{
				K0 = tomb[ii];  /// K0 állítása
				std::cout << "K0" << K0 << endl;
				GlobalSpectrumState::blokknum = 0;
				GlobalSpectrumState::protection_blokknum = 0;
				GlobalSpectrumState::getInstance().clear();
				for (int i2 = 0; i2<REQUESTS; i2++)
				{
					Path<ListGraph> allocated;  /// lefoglalt üzemi út
					n1 = random1.integer(0, 27);  /// kezdő és vég csomópont
					n2 = random1.integer(0, 27);
					Node s1 = graph.nodeFromId(n1);
					Node t1 = graph.nodeFromId(n2);
					dur = (long int)random1.exponential(0.03);
					width1 = random1.integer(1, 5);
					allocated.clear();
					if (n1 != n2&&dur>0){
						try{
							if (!GlobalSpectrumState::getInstance().EndToEnd(s1, t1, width1, dur))
							{
								GlobalSpectrumState::protection_round = false;
								Kshort<ListGraph> _ks(graph);
								_ks.setK(K0);
								if (_ks.run(s1, t1, width1, dur))
								{
									allocated = _ks.allocatedPath();
								}
								else{ throw "Uzemi blokkolas"; }

							}
							if (!GlobalSpectrumState::getInstance().dedicated_EndToEnd(s1, t1, width1, dur, allocated))
							{
								GlobalSpectrumState::protection_round = true;
								SubgraphMaker makesub(graph, allocated);
								Subgraph *subgraph = makesub.make();
								Kshort<Subgraph> _ks(*subgraph);
								_ks.setK(K0);
								_ks.run(s1, t1, width1, dur);
							}
						}
						catch (char* c)
						{

						}
						GlobalSpectrumState::getInstance().TimeCheck();
					}
				}
				std::cout << "Kshor blokkolas" << GlobalSpectrumState::blokknum << endl;
				std::cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
			}
		
	return 0;
}

/**
* Módosított dijkstra algoritmussal futó üzemi és védelmi útválasztás szimulációja
*/
void simulationDedicatedModdijk(ListGraph &graph, Random &random)
{
	long int dursum(0), durcnt(0);
	int n1(0), n2(0), width1(0), k(0); long int dur = 0;
	lemon::Random random1(random);
	GlobalSpectrumState::blokknum = 0;
	GlobalSpectrumState::protection_blokknum = 0;
	cout << "ModDijkstra";
	for (int i2 = 0; i2<REQUESTS; i2++)
	{


		Path<ListGraph> allocated;
		n1 = random1.integer(0, 27);
		n2 = random1.integer(0, 27);
		Node s1 = graph.nodeFromId(n1);
		Node t1 = graph.nodeFromId(n2);
		dur = (long int)random1.exponential(0.03);
		width1 = random1.integer(1, 5);
		allocated.clear();
		if (n1 != n2&&dur>0){
			try{
				if (!GlobalSpectrumState::getInstance().EndToEnd(s1, t1, width1, dur))
				{
					GlobalSpectrumState::protection_round = false;
					ModDijkstra<ListGraph> md_dijkstra(graph, globalspectrum);
					if (md_dijkstra.run(s1, t1, width1, dur))
					{
						allocated = md_dijkstra.allocatedPath();
					}
					else{ throw "Uzemi blokkolas"; }

				}
				if (!globalspectrum.dedicated_EndToEnd(s1, t1, width1, dur, allocated))
				{
					GlobalSpectrumState::protection_round = true;
					SubgraphMaker makesub(graph, allocated);
					Subgraph *subgraph = makesub.make();
					ModDijkstra<Subgraph> md_dijkstra1(*subgraph, spectrum_map, globalspectrum);
					md_dijkstra1.run(s1, t1, width1, dur);

				}
			}
			catch (char* c)
			{
			}
			globalspectrum.TimeCheck();
		}

	}
	//printSpectrum(spectrum_map, graph);
	cout << endl << "Mod BLOKKOLASOK:";
	cout << GlobalSpectrumState::blokknum << endl;
	cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void simulationSharedKshort(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &spectrum_map, GlobalSpectrumState &globalspectrum, Random &random)
{
	GlobalSpectrumState::blokknum = 0;
	GlobalSpectrumState::protection_blokknum = 0;
	int K0;
	int n1(0), n2(0), width1(0), k(0); long int dur = 0;
	lemon::Random random1(random);
	int tomb[] = { 1, 2, 5, 10, 20 };
	for (int ii = 0; ii < 5; ii++)
	{
		K0 = tomb[ii];
		cout << "K0" << K0 << endl;
		GlobalSpectrumState::blokknum = 0;
		GlobalSpectrumState::protection_blokknum = 0;
		globalspectrum.clear();
		for (int i2 = 0; i2<REQUESTS; i2++)
		{
			Path<ListGraph> allocated;
			n1 = random1.integer(0, 27);
			n2 = random1.integer(0, 27);
			Node s1 = graph.nodeFromId(n1);
			Node t1 = graph.nodeFromId(n2);
			dur = (long int)random1.exponential(0.03);
			width1 = random1.integer(1, 5);
			allocated.clear();
			if (n1 != n2&&dur>0){
				try{
					if (!globalspectrum.EndToEnd(s1, t1, width1, dur))
					{
						GlobalSpectrumState::protection_round = false;
						Kshort<ListGraph> _ks(graph, globalspectrum);
						_ks.setK(K0);
						if (_ks.run(s1, t1, width1, dur))
						{
							allocated = _ks.allocatedPath();
						}
						else{ throw "Uzemi blokkolas"; }
					}
					GlobalSpectrumState::protection_round = true;
					SharedProtection shared_protection(globalspectrum, allocated);
					shared_protection.runKshort(s1, t1, width1, dur, K0);
				}
				catch (char* c)
				{
				}
				globalspectrum.TimeCheck();
			}
		}
		cout << "Kshor blokkolas" << GlobalSpectrumState::blokknum << endl;
		cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
	}
}

void simulationSharedModdijkstra(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &spectrum_map, GlobalSpectrumState &globalspectrum, Random &random)
{
	GlobalSpectrumState::blokknum = 0;
	GlobalSpectrumState::protection_blokknum = 0;
	long int dursum(0), durcnt(0);
	int n1(0), n2(0), width1(0), k(0); long int dur = 0;
	lemon::Random random1(random);
	cout << "ModDijkstra";
	for (int i2 = 0; i2<REQUESTS; i2++)
	{
		Path<ListGraph> allocated;
		n1 = random1.integer(0, 27);
		n2 = random1.integer(0, 27);
		Node s1 = graph.nodeFromId(n1);
		Node t1 = graph.nodeFromId(n2);
		dur = (long int)random1.exponential(0.03);
		width1 = random1.integer(1, 5);
		allocated.clear();
		if (n1 != n2&&dur>0){
			try{
				if (!globalspectrum.EndToEnd(s1, t1, width1, dur))
				{
					GlobalSpectrumState::protection_round = false;
					ModDijkstra<ListGraph> md_dijkstra(graph, globalspectrum);
					if (md_dijkstra.run(s1, t1, width1, dur))
					{
						allocated = md_dijkstra.allocatedPath();
					}
					else{ throw "Uzemi blokkolas"; }
				}
				GlobalSpectrumState::protection_round = true;
				SharedProtection shared_protection(globalspectrum, allocated);
				shared_protection.runmoddijkstra(s1, t1, width1, dur);
			}
			catch (char* c)
			{
			}
			globalspectrum.TimeCheck();
		}
	}
	//printSpectrum(spectrum_map, graph);
	cout << endl << "Mod BLOKKOLASOK:";
	cout << GlobalSpectrumState::blokknum << endl;
	cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}