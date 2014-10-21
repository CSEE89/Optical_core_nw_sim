
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
* kompar�tor oszt�ly a sima lemon:Path �tvonal rendez�shez
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
* Routing 1 ig�ny -> 1-4  utv�laszt�s RSA
*/
class Anycast{
	std::vector<Node> serverNodes;  // szerverek
	std::multiset<Path<ListGraph>, compPath> paths;  // szerverekhaz tal�lt utak hossz szerint
	ListGraph &graph;
	int anycastblock;  // blokkol�sok sz�ma
	int anycastblock_prot;
	Node s;   
	int width;
	long int dur;
	Path<ListGraph> allocated; // lefoglalt �zemi �t, a v�delmihez kell
	Node allocatedNode; // a kiv�lasztott szerver

public:
	Anycast(ListGraph &g) :graph(g){}
	void setReplicaServer(int id){
		serverNodes.push_back(graph.nodeFromId(id));
	}

	/** kor�bban megtal�lt utak t�rl�se
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
	* futtat�s: for ciklus kisz�molja az �sszes szerverhez modDijkstraval az utat
	* paths v�ltoz�ba elmenti
	*/
	void runModDijkstra(Node s_i, const int &width_i, const long int &dur_i){
		s = s_i;
		width = width_i;
		dur = dur_i;
		inRunClear();
		if (!end2endforAnycast()) // van e m�r �t, tudunk e mell� foglalni
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
	* �zemi �t lefoglal�sa, RSA minden szerverhez megn�zz�k az utakat, legr�vedebb nyer
	* modDijktra tal�l utat lefoglaljuk, 
	* ha nem blokkol�sok sz�m�t n�velj�k
	*/
	bool workingpath_modD(){
	
		paths.clear();
		Path<ListGraph> p_path;
		ModDijkstra<ListGraph> md_dijkstra(graph);
		for (int i = 0; i < serverNodes.size(); i++)
		{		
			if (md_dijkstra.calcpath(s, serverNodes[i], width)) // ha tal�ltunk �tvonalat, amin lehet lefoglalni(szabad spektrum)
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
			alloc(width, dur); // Anycast::alloc -> lefoglaljk az utat	!!!!!!!!!!!! allocatedNode be�ll�t		
			allocated = GlobalSpectrumState::getInstance().getWorkingPath(); // lefoglalt �zemi �t, a v�delmihez kell
			return true;
		}
		else{
			anycastblock++;  
			return false;
		}
	}

	/**
	* Modjikstra anycast-hez v�delmi �t keres�se
	*/
	bool runModPortection(){
		paths.clear();
		Path<ListGraph> p_path;
		GlobalSpectrumState::protection_round = true;
		SubgraphMaker makesub(graph, allocated);

		// ellen�rz�s, NEM SZ�KS�GES----------------------------------------
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

	// end2end grooming , anycastnek is kell, ha m�r van �sszek�tet�s jobb ahhoz csapni
	//ha siker�l end2end nem kell moddijktra
	bool end2endforAnycast(){
		bool switcher = false;
		for (int i = 0; i < serverNodes.size(); i++){
			Node t = serverNodes[i];
			if (GlobalSpectrumState::getInstance().EndToEnd(s, t, width, dur))
			{
				allocatedNode = t;
				allocated = GlobalSpectrumState::getInstance().getWorkingPath();  // lefoglalt �zemi �t, a v�delmihez kell
				switcher = true;
				break;
			}
		}
		return switcher;
	}
	
	// v�delmet is groomingolunk
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
	int getBlockProt(){ return anycastblock_prot; }  // v�delmi blokkol�sok
private:
	void alloc(const int &width, const long int &dur)
	{
		Path<ListGraph> tp = *paths.begin();	
		PathNodeIt<Path<ListGraph> > it(graph, tp);
		allocatedNode = (Node)it;
		GlobalSpectrumState::getInstance().Alloc(tp, width, dur);
	}

	
};