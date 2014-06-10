#include "stdafx.h"
#include"utilities.h"
//#include"SharedProtection.h"
#include<lemon/random.h>
#define REQUESTS 10000
//#include"Dijkstraplus.h"

using namespace std;
using namespace lemon;
/*
*      a dijkstraba vhogy tiltást kell tenni egy adott élre vonatkozólag
*      TIPS:
*              -lemon/dijkstra lengthmap-el párhuzamosan engedélyzõ map
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
int GlobalSpectrumState::ALLOCMOD(2);
bool GlobalSpectrumState::isblocked(false);
int CH::channel_num(35);

void testerfunction_moddijk(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &spectrum_map, GlobalSpectrumState &globalspectrum, Random &random);
void testerfunction_K_short(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &spectrum_map, GlobalSpectrumState &globalspectrum, Random &random);
int main() {

	int ch_num_tomb[] = { 35, 40, 45, 50, 55 };
	for (int ii0 = 0; ii0 < 5; ii0++)
	{
		CH::channel_num = ch_num_tomb[ii0];
		cout << "CSATORNA SZAM: " << CH::channel_num << endl;
		cout << "---------------------------------------------------" << endl;


		for (int ii1 = 0; ii1 < 3; ii1++)
		{
			GlobalSpectrumState::ALLOCMOD = ii1;

			cout << endl << "                               STRATEGIA: " << GlobalSpectrumState::ALLOCMOD << endl;
			typedef dim2::Point<int> Point;
			typedef ListGraph::Node Node;
			typedef ListGraph::Edge Edge;
			typedef PathNodeIt<Path<ListGraph> > PN;
			typedef ListDigraph::Node DiNode;
			ListGraph graph;
			graphReader(graph, "28_eu.lgf").run(); //.edgeMap("cost",lengthmap)
			ListDigraph digraph;
			digraphReader(digraph, "28_eu.lgf").run(); // két grafot olvasunk be egy irányított és egy irányítatlan
			ListGraph::EdgeMap<int> lengthmap(graph);
			ListGraph::EdgeMap<bool> permittingmap(graph);
			ListGraph::NodeMap<pathpair_vector> pathmap(graph); //minden csomóponthoz tartozó utak hlamza 
			ListGraph::EdgeMap<SpectrumState> spectrum_map(graph);
			GlobalSpectrumState globalspectrum(graph, spectrum_map);

			Deparallel para;
			para(graph);
			GlobalSpectrumState::blokknum = 0;
			GlobalSpectrumState::protection_blokknum = 0;
			Random random;
			testerfunction_moddijk(graph, spectrum_map, globalspectrum, random);



			std::vector< vector<int> > Kshortvec;
			int counter = 0;
			long int dursum(0), durcnt(0);
			int n1(0), n2(0), width1(0), k(0); long int dur = 0;
			int K0 = 0;
			lemon::Random random1(random);

			int j(0);

			//Kshortest
			int tomb[] = { 1, 2, 5, 10, 20 };
			for (int ii = 0; ii < 5; ii++)
			{
				K0 = tomb[ii];
				cout << "K0" << K0 << endl;

				GlobalSpectrumState::blokknum = 0;
				GlobalSpectrumState::protection_blokknum = 0;
				for (int i2 = 0; i2 < REQUESTS; i2++)
				{
					Path<ListGraph> allocated;
					n1 = random1.integer(0, 27);
					n2 = random1.integer(0, 27);
					Node s = graph.nodeFromId(n1);
					Node t = graph.nodeFromId(n2);
					dur = (long int)random1.exponential(0.03);
					dursum = dursum + dur;
					width1 = random1.integer(1, 5);
					allocated.clear();
					if (n1 != n2&&dur > 0){
						if (!globalspectrum.EndToEnd(s, t, width1, dur))
						{

							GlobalSpectrumState::protection_round = false;
							Kshort<ListGraph> _ks(graph, globalspectrum);
							_ks.setK(K0);
							_ks.run(s, t, width1, dur);
							allocated = _ks.allocatedPath();
						}

						if ((!globalspectrum.disjoint_EndToEnd(s, t, width1, dur, allocated)) && (!GlobalSpectrumState::isblocked))
						{
							GlobalSpectrumState::protection_round = true;
							SubgraphMaker makesub(graph, allocated);
							Subgraph *subgraph = makesub.make();
							Kshort<Subgraph> _ks(*subgraph, globalspectrum);
							_ks.setK(K0);
							_ks.run(s, t, width1, dur);
						}

						globalspectrum.TimeCheck();
					}
					else{ counter++; }
					//printSpectrum(spectrum_map,graph);

				}
				//printSpectrum(spectrum_map, graph);

				//cout << globalspectrum.blokknum;
				//cout << endl << "counter" << counter;
				cout << "Kshor blokkolas" << GlobalSpectrumState::blokknum << endl;
				cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
			}

		}
	}
	return 0;
}

void testerfunction_moddijk(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &spectrum_map, GlobalSpectrumState &globalspectrum, Random &random)
{
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
			if (!globalspectrum.EndToEnd(s1, t1, width1, dur))
			{
				GlobalSpectrumState::protection_round = false;
				ModDijkstra<ListGraph> md_dijkstra(graph, spectrum_map, globalspectrum);
				md_dijkstra.run(s1, t1, width1, dur);
				allocated = md_dijkstra.allocatedPath();
			}


			if ((!globalspectrum.disjoint_EndToEnd(s1, t1, width1, dur, allocated)) && (!GlobalSpectrumState::isblocked))
			{
				GlobalSpectrumState::protection_round = true;
				SubgraphMaker makesub(graph, allocated);
				Subgraph *subgraph = makesub.make();
				ModDijkstra<Subgraph> md_dijkstra1(*subgraph, spectrum_map, globalspectrum);
				md_dijkstra1.run(s1, t1, width1, dur);

			}
			globalspectrum.TimeCheck();
		}

	}
	//printSpectrum(spectrum_map, graph);
	cout << endl << "Mod BLOKKOLASOK:";
	cout << GlobalSpectrumState::blokknum << endl;
	cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}
