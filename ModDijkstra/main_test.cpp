#include "stdafx.h"
#include"utilities.h"
//#include"SharedProtection.h"
#include"Anycast.h"
#include<lemon/random.h>
#define REQUESTS 10000

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
void simulationDedicatedKshort(ListGraph &graph, Random &random);
void simulationDedicatedModdijk(ListGraph &graph,  Random &random);
void simulationSharedKshort(ListGraph &graph,  Random &random);
void simulationSharedModdijkstra(ListGraph &graph, Random &random);
int main() {
	
	std::vector<vector<int> > eu_servers;
	vector<int> v = { 12 };
	eu_servers.push_back(v);
	v = { 4 };
	eu_servers.push_back(v);
	v = { 6 };
	eu_servers.push_back(v);
	//2:
	v = { 4,12 };
	eu_servers.push_back(v);
	v = { 24,12 };
	eu_servers.push_back(v);
	v = { 14,16};
	eu_servers.push_back(v);
	//3:
	v = { 4,14,16 };
	eu_servers.push_back(v);
	v = { 8,24,16 };
	eu_servers.push_back(v);
	v = { 8,24,12 };
	eu_servers.push_back(v);
	//4:
	v = { 24,4,8,16 };
	eu_servers.push_back(v);
	v = { 22,16,12,4 };
	eu_servers.push_back(v);
	v = { 4,14,16,12 };
	eu_servers.push_back(v);
	for (int i = 0; i < eu_servers.size(); i++){ cout << eu_servers[i]; }

	int ch_num_tomb[] = { 40, 50, 60 };
	ALLOC a_mod[] = {ALLOC::OneSideChannelFill,ALLOC::GapFill,ALLOC::TwoSideChannelFill};
	for (int ch_i = 0; ch_i < 3; ch_i++)
	{
		cout << "CHANNEL NUMBER =" << ch_num_tomb[ch_i]<<"------------------------------------------" << endl;
		CH::channel_num = ch_num_tomb[ch_i];
		for (int a_i= 0; a_i < 3; a_i++){
			cout << "ALLOC MOD =" << a_mod[a_i]<<"----------------------------------------" << endl;
		
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
			GlobalSpectrumState::getInstance().ALLOCMOD =a_mod[a_i];   // spektrul foglalási strat beállítása
			
			Deparallel para;
			para(graph);
			GlobalSpectrumState::blokknum = 0;    // bolokkolások számát tároló változó incializálása
			GlobalSpectrumState::protection_blokknum = 0;
			Random random;

			  
			int counter = 0;



//-------------------------------------------------------------------------------------------------------------------------------------------
			//anycast dedikált védelemmel
			Anycast any_rsa(graph);
			for (size_t i_servers = 0; i_servers < eu_servers.size(); i_servers++)
			{


				for (size_t i_serv = 0; i_serv < eu_servers[i_servers].size(); i_serv++)
				{
					any_rsa.setReplicaServer(eu_servers[i_servers][i_serv]);
				}
				cout << "Anycast for servers:" << eu_servers[i_servers] << endl;
				long int dursum(0), durcnt(0);
				int n1(0), n2(0), width1(0), k(0); long int dur = 0;
				lemon::Random random1(random);
				GlobalSpectrumState::getInstance().clear();
				GlobalSpectrumState::blokknum = 0;
				GlobalSpectrumState::protection_blokknum = 0;
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
						if (!GlobalSpectrumState::getInstance().EndToEnd(s1, t1, width1, dur))
						{
							GlobalSpectrumState::protection_round = false;
							any_rsa.runModDijkstra(s1, width1, dur);
							GlobalSpectrumState::getInstance().TimeCheck();
						}
					}
				}
				//printSpectrum(spectrum_map, graph);
				cout << endl << "ANYCAST BLOKKOLASOK:";
				cout << any_rsa.getBlock() << endl;
				cout << "ANYCAST vedelmi blokkolas" << any_rsa.getBlockProt() << endl;
			}

//-------------------------------------------------------------------------------------------------------------------------------------------

			/**
			* Módosított dijkstra algoritmussal futó üzemi és védelmi útválasztás szimulációja
			
				GlobalSpectrumState::getInstance().clear();
			simulationDedicatedModdijk(graph, random); /// módosított dijkstra
				//Globalspectrum visszaállítása kezdeti állapotba
				GlobalSpectrumState::getInstance().clear();
			simulationSharedModdijkstra(graph,  random);
				GlobalSpectrumState::getInstance().clear();
			simulationDedicatedKshort(graph, random);
				GlobalSpectrumState::getInstance().clear();
			simulationSharedKshort(graph, random); 
			
			*/
	}
	}
	return 0;
}

void simulationDedicatedKshort(ListGraph &graph, Random &random)
{
GlobalSpectrumState::getInstance().clear();
int tomb[] = { 1, 2, 5, 10, 20 };
GlobalSpectrumState::blokknum = 0;
GlobalSpectrumState::protection_blokknum = 0;
int K0;
int n1(0), n2(0), width1(0), k(0); long int dur = 0;
lemon::Random random1(random);
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
				else{ allocated = GlobalSpectrumState::getInstance().getWorkingPath(); }
				// ellenörzi hogy egyezeik az allocated tartalma GlobalSpectrum getWorkinPath-al
				std::vector<int> cmp1;
				for (PathNodeIt<Path<ListGraph> > pit(graph, allocated); pit != INVALID; pit++){ cmp1.push_back(graph.id(pit)); }
				std::vector<int> cmp2;
				Path<ListGraph> allocated2 = GlobalSpectrumState::getInstance().getWorkingPath();
				for (PathNodeIt<Path<ListGraph> > pit(graph, allocated2); pit != INVALID; pit++){ cmp2.push_back(graph.id(pit)); }
				if (cmp1 == cmp2){
					if (!std::equal(cmp1.begin(), cmp1.end(), cmp2.begin())) cout << "nem egyenlo" << endl;
				}
				else{ cout << "nem egyenlo hosszu" << endl; }
				//--------------------------
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
	std::cout << "Kshort DEDIKÁLT VÉDELEMMEL:" << endl;
	std::cout << "Kshor blokkolas" << GlobalSpectrumState::blokknum << endl;
	std::cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}
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
					ModDijkstra<ListGraph> md_dijkstra(graph);
					if (md_dijkstra.run(s1, t1, width1, dur))
					{
						allocated = md_dijkstra.allocatedPath();
						
					}
					else{ throw "Uzemi blokkolas"; }

				}
				else{ allocated = GlobalSpectrumState::getInstance().getWorkingPath(); }

				if (!GlobalSpectrumState::getInstance().dedicated_EndToEnd(s1, t1, width1, dur, allocated))
				{
					GlobalSpectrumState::protection_round = true;
					SubgraphMaker makesub(graph, allocated);
					Subgraph *subgraph = makesub.make();
					ModDijkstra<Subgraph> md_dijkstra1(*subgraph);
					md_dijkstra1.run(s1, t1, width1, dur);

				}
			}
			catch (char* c)
			{
			}
			GlobalSpectrumState::getInstance().TimeCheck();
		}

	}
	//printSpectrum(spectrum_map, graph);
	std::cout << "ModDijkstra DEDIKÁLT VÉDELEMMEL:" << endl;
	cout << GlobalSpectrumState::blokknum << endl;
	cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void simulationSharedKshort(ListGraph &graph, Random &random)
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
		GlobalSpectrumState::getInstance().clear();
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
						Kshort<ListGraph> _ks(graph);
						_ks.setK(K0);
						if (_ks.run(s1, t1, width1, dur))
						{
							allocated = _ks.allocatedPath();
						}
						else{ throw "Uzemi blokkolas"; }
					}
					else{ allocated = GlobalSpectrumState::getInstance().getWorkingPath(); }

					GlobalSpectrumState::protection_round = true;
					SharedProtection shared_protection(allocated);
					shared_protection.runKshort(s1, t1, width1, dur, K0);
				}
				catch (char* c)
				{
				}
				GlobalSpectrumState::getInstance().TimeCheck();
			}
		}
		std::cout << "Kshort MEGOSZTOTT VÉDELEMMEL:" << endl;
		cout << "Kshor blokkolas" << GlobalSpectrumState::blokknum << endl;
		cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
	}
}

void simulationSharedModdijkstra(ListGraph &graph, Random &random)
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
				if (!GlobalSpectrumState::getInstance().EndToEnd(s1, t1, width1, dur))
				{
					GlobalSpectrumState::protection_round = false;
					ModDijkstra<ListGraph> md_dijkstra(graph);
					if (md_dijkstra.run(s1, t1, width1, dur))
					{
						allocated = md_dijkstra.allocatedPath();

					}
					else{ throw "Uzemi blokkolas"; }
				}
				else{ allocated = GlobalSpectrumState::getInstance().getWorkingPath(); }

				GlobalSpectrumState::protection_round = true;
				SharedProtection shared_protection( allocated);
				shared_protection.runmoddijkstra(s1, t1, width1, dur);
			}
			catch (char* c)
			{
			}
			GlobalSpectrumState::getInstance().TimeCheck();
		}
	}
	//printSpectrum(spectrum_map, graph);
	std::cout << "ModDijkstra MEGOSZTOTT VÉDELEMMEL:" << endl;
	cout << GlobalSpectrumState::blokknum << endl;
	cout << "vedelmi blokkolas" << GlobalSpectrumState::protection_blokknum << endl;
}