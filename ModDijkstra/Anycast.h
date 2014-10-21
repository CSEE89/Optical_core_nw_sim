
#include"SharedProtection.h"


/*class AnycastPath{
public:
	AnycastPath(){}
	AnycastPath(const AnycastPath &a){}
	Path<ListGraph> path;
	Node t;
};
*/

/**
* komparátor osztály a sima lemon:Path õtvonal rendezéshez
*/
class compPath{
public:
	bool operator()(const Path<ListGraph> &a, const Path<ListGraph> &b)
	{
		return a.length() < b.length();
	}
};

/**
* 
* Routing 1 igény -> 1-4  utválasztás RSA
*/
class Anycast{
	std::vector<Node> serverNodes;  // szerverek
	std::multiset<Path<ListGraph>, compPath> paths;  // szerverekhaz talált utak hossz szerint
	ListGraph &graph;
	int anycastblock;  // blokkolások száma
	int anycastblock_prot;
	Node s;   
	int width;
	long int dur;
	Path<ListGraph> allocated; // lefoglalt üzemi út, a védelmihez kell
	Node allocatedNode; // a kiválasztott szerver

public:
	Anycast(ListGraph &g) :graph(g){}
	void setReplicaServer(int id){
		serverNodes.push_back(graph.nodeFromId(id));
	}

	/** korábban megtalált utak törlése
	*/
	void clearPaths(){
		paths.clear();	
	}
	void inRunClear()
	{
		paths.clear();
		allocatedNode = INVALID;
		allocated.clear();
	}
	/**
	* futtatás: for ciklus kiszámolja az összes szerverhez modDijkstraval az utat
	* paths változóba elmenti
	*/
	void runModDijkstra(Node s_i, const int &width_i, const long int &dur_i){
		s = s_i;
		width = width_i;
		dur = dur_i;
		inRunClear();
		if (!end2endforAnycast()) // van e már út, tudunk e mellé foglalni
		{
			if (workingpath_modD()){  // modjiktra uzemi ut keres, ha van 
				if (!protectionEnd2endforAnycast())
				{
					runModPortection();
				}
			}
		}
		else
		{
			if (!protectionEnd2endforAnycast())
			{
				runModPortection();
			}
		}		
	}

	/**
	* Üzemi út lefoglalása, RSA minden szerverhez megnézzük az utakat, legrövedebb nyer
	* modDijktra talál utat lefoglaljuk, 
	* ha nem blokkolások számát növeljük
	*/
	bool workingpath_modD(){
	
		paths.clear();
		Path<ListGraph> p_path;
		ModDijkstra<ListGraph> md_dijkstra(graph);
		for (int i = 0; i < serverNodes.size(); i++)
		{		
			if (md_dijkstra.calcpath(s, serverNodes[i], width)) // ha találtunk útvonalat, amin lehet lefoglalni(szabad spektrum)
			{								
				p_path.clear();
				p_path = md_dijkstra.allocatedPath();  
				if (!p_path.empty())
				{
					paths.insert(p_path);
				}
			}
		}
		if (!paths.empty())
		{
			alloc(width, dur); // Anycast::alloc -> lefoglaljk az utat	!!!!!!!!!!!! allocatedNode beállít		
			allocated = GlobalSpectrumState::getInstance().getWorkingPath(); // lefoglalt üzemi út, a védelmihez kell
			return true;
		}
		else{
			anycastblock++;  
			return false;
		}
	}

	/**
	* Modjikstra anycast-hez védelmi út keresése
	*/
	bool runModPortection(){
		paths.clear();
		Path<ListGraph> p_path;
		GlobalSpectrumState::protection_round = true;
		SubgraphMaker makesub(graph, allocated);

		// ellenörzés, NEM SZÜKSÉGES----------------------------------------
		std::vector<int> cmp;
		for (PathNodeIt<Path<ListGraph> > it(graph, allocated); it != INVALID; it++) { cmp.push_back(graph.id(it)); }
		std::vector<int> cmp2;
		Path<ListGraph> allocated2 = GlobalSpectrumState::getInstance().getWorkingPath();
		for (PathNodeIt<Path<ListGraph> > it(graph, allocated2); it != INVALID; it++){ cmp2.push_back(graph.id(it)); }
		if (cmp == cmp2)
		{
			if (!std::equal(cmp.begin(), cmp.end(), cmp2.begin()))
				std::cout << "The contents of both sequences differ.\n";
		}
		else
			std::cout << "The contents of both sequences differ.\n";
		//------------------------------------------------

		Subgraph *subgraph = makesub.make();
		ModDijkstra<Subgraph> md_dijkstra(*subgraph);
		for (int i = 0; i < serverNodes.size(); i++)
		{
			md_dijkstra.calcpath(s, serverNodes[i], width);
			p_path.clear();
			p_path = md_dijkstra.allocatedPath();
			if (!p_path.empty())
			paths.insert(p_path);
		}
		if (!paths.empty())
		{
			alloc(width, dur);
			return true;
		}
		else
		{
			anycastblock_prot++;
			return false;
		}
	}
	
	void runKshort(){}
	Path<ListGraph> allocatedPath(){
		return *paths.begin();
	}

	// end2end grooming , anycastnek is kell, ha már van összekötetés jobb ahhoz csapni
	//ha sikerül end2end nem kell moddijktra
	bool end2endforAnycast(){
		bool switcher = false;
		for (int i = 0; i < serverNodes.size(); i++){
			Node t = serverNodes[i];
			if (GlobalSpectrumState::getInstance().EndToEnd(s, t, width, dur))
			{
				allocatedNode = t;
				allocated = GlobalSpectrumState::getInstance().getWorkingPath();  // lefoglalt üzemi út, a védelmihez kell
				switcher = true;
				break;
			}
		}
		return switcher;
	}
	
	// védelmet is groomingolunk
	bool protectionEnd2endforAnycast(){
		bool switcher = false;
		for (int i = 0; i < serverNodes.size(); i++){
			Node t = serverNodes[i];
			if (GlobalSpectrumState::getInstance().dedicated_EndToEnd(s, t, width, dur, allocated))
			{
				switcher = true;
				break;
			}
		}
		return switcher;
	}
	int getBlock(){ return anycastblock; }
	int getBlockProt(){ return anycastblock_prot; }  // védelmi blokkolások
private:
	void alloc(const int &width, const long int &dur)
	{
		Path<ListGraph> tp = *paths.begin();	
		PathNodeIt<Path<ListGraph> > it(graph, tp);
		allocatedNode = (Node)it;
		GlobalSpectrumState::getInstance().Alloc(tp, width, dur);
	}

	
};