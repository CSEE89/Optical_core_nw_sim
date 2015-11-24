
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
*Data structure for a node computational capacity
*/
struct NodeComp{
	int max_cap;
	int current_cap;
	NodeComp(int m_cap):max_cap(m_cap),current_cap(0){}
};

class ServerLoads{
	struct Req{
		int capacity;
		int duration;
		Req(int comp, int dur) :capacity(comp), duration(dur){}
	};
	//typedef std::pair<int, int> req;// type capacity, duration
	std::multimap<int, Req> comp_load; // node, (cap, dur) minden igény 1 enként csökkent
	std::map<int,NodeComp> servers;
public:
	ServerLoads(){

	}
	void addServer(int NodeId,int max_cap){		
		servers.insert(std::pair<int,NodeComp>(NodeId, NodeComp(max_cap)));
	}

	// @param NodeId, Comp req, duration
	void addLoad(int NodeId,int comp_req,int dur){
		Req r(comp_req,dur);
		comp_load.insert(std::pair<int, Req>(NodeId, r));
		servers.at(NodeId).current_cap+=comp_req;
	}	
	/* decrease the duration by one,
	*  
	*/
	void timeCheck(){ 
		//to-do TEST!!!!!!!!!
		std::vector<std::multimap<int,Req>::iterator> it_vec;		
		for (std::multimap<int, Req>::iterator it = comp_load.begin(); it != comp_load.end(); it++)
		{
			it->second.duration--;
			if (it->second.duration == 0)
			{
				it_vec.push_back(it);
				servers.at(it->first).current_cap -= it->second.capacity;
				//comp_load.erase(eit);				
			}
		}
		for (int i = 0; i < it_vec.size();i++){			
			comp_load.erase(it_vec[i]);				
		}
	}
	int get(int id){
		return servers.at(id).current_cap;
	}
	int getMax(int id){
		return servers.at(id).max_cap;
	}
private:

};
/**
* 
* Routing 1 igény -> 1-4  utválasztás RSA
*/
class Anycast{
protected:
	std::vector<Node> serverNodes;  // szerverek
	std::vector<Path<ListGraph> > paths;  // szerverekhaz talált utak 
	ListGraph &graph;
	int anycastblock;  // blokkolások száma
	int anycastblock_prot;
	Path<ListGraph> allocated; // lefoglalt üzemi út, a védelmihez kell	
	
public:	
	Anycast(ListGraph &g) :graph(g){}
	void setReplicaServer(int id){
		serverNodes.push_back(graph.nodeFromId(id));
		//server_loads.addServer(id);
	}	
	void clearAll(){
		serverNodes.clear();
		paths.clear();
		anycastblock = 0;
		anycastblock_prot = 0;
		allocated.clear();
		//allocatedNode = INVALID;		
		
	}
	/** korábban megtalált utak törlése
	*/
	void clearPaths(){
		paths.clear();	
	}
	void inRunClear()
	{
		paths.clear();		
		allocated.clear();
	}
	/**
	* vezérlõblokk az anycast futtatáshoz
	*/
	void runModDijkstra(Node s_i, const int &width_i, const long int &dur_i){

		inRunClear();
		if (!end2endforAnycast(s_i,width_i,dur_i)) // van e már út, tudunk e mellé foglalni
		{
			if (workingpath_modD(s_i, width_i, dur_i)){  // modjiktra uzemi ut keres, ha van 
				if (!protectionEnd2endforAnycast(s_i, width_i, dur_i))
				{
					runModPortection(s_i, width_i, dur_i);
				}
			}
			//blokkolas
		}
		else
		{
			if (!protectionEnd2endforAnycast(s_i, width_i, dur_i))
			{
				runModPortection(s_i, width_i, dur_i);
			}
		}		
	}

	void runModDijkstraNoProt(Node s, const int &width, const long int &dur){
		inRunClear();
		workingpath_modD(s,width,dur);  // modjiktra uzemi ut keres, ha van 		
	}

	int getBlock(){ return anycastblock; }
	int getBlockProt(){ return anycastblock_prot; }  // védelmi blokkolások
	void printPathSpectrum(){}

protected:
	/**
	* Üzemi út lefoglalása, RSA minden szerverhez megnézzük az utakat, legrövedebb nyer
	* modDijktra talál utat lefoglaljuk, 
	* ha nem blokkolások számát növeljük
	*/
	bool workingpath_modD(Node s, const int &width, const long int &dur){
	
		paths.clear();
		Path<ListGraph> p_path;
		ModDijkstra<ListGraph> md_dijkstra(graph);
		for (int i = 0; i < serverNodes.size(); i++) // összes szerverre megnezzuk az utakat
		{		
			if (md_dijkstra.calcpath(s, serverNodes[i], width)) // ha találtunk útvonalat, amin lehet lefoglalni(szabad spektrum)
			{								
				p_path.clear();
				p_path = md_dijkstra.allocatedPath();  
				if (!p_path.empty())
				{
					paths.push_back(p_path); //sorrendben berakjuk
				}
			}
		}
		if (!paths.empty())
		{
			if (alloc(width, dur).empty()) // Anycast::alloc -> lefoglaljk a legrövidebb utat	!!!!!!!!!!!! allocatedNode beállít		
			{
				anycastblock++;
				anycastblock_prot++;
				return false;
			}
			allocated = GlobalSpectrumState::getInstance().getWorkingPath(); // lefoglalt üzemi út, a védelmihez kell
			return true;
		}
		else{
			anycastblock++;  
			return false;
		}
	}
	
	/**
	* Modjikstra anycast-hez védelmi út keresése, DEDIKÁLT védelem moddijktraval
	*/
	bool runModPortection(Node s, const int &width, const long int &dur){
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
			paths.push_back(p_path);
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
	

	// end2end grooming , anycastnek is kell, ha már van összekötetés jobb ahhoz csapni
	//ha sikerül end2end nem kell moddijktra
	bool end2endforAnycast(Node s, const int &width, const long int &dur){
		bool switcher = false;
		for (int i = 0; i < serverNodes.size(); i++){
			Node t = serverNodes[i];
			if (GlobalSpectrumState::getInstance().EndToEnd(s, t, width, dur))
			{				
				allocated = GlobalSpectrumState::getInstance().getWorkingPath();  // lefoglalt üzemi út, a védelmihez kell
				switcher = true;
				break;
			}
		}
		return switcher;
	}
	
	// védelmet is groomingolunk
	bool protectionEnd2endforAnycast(Node s, const int &width, const long int &dur){
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

	
	
	Path<ListGraph> cost(){// ha terhelés kisebb mint max spektrum akkor ok
		
		double lengt_val = 0;
		double load_val = 0;
		double cost = 0;		
		std::map<double, int> map1; // cost index pairs
		
		for (int i=0; i<paths.size(); i++)
		{
			Path<ListGraph> tp = paths[i];
			lengt_val = tp.length();			
			map1.insert(std::pair<double,int>(lengt_val, i));			
		}
		int idx = map1.begin()->second;  //to-do test!!  kiír sorrendben set-et map-ot cost-ot				
		return paths[idx];
	}

	void printMap(std::map<double, int> map){
		cout << "printMap " << endl;
		for (auto it = map.begin(); it != map.end(); it++)
		{
			cout <<it->first<<":"<<it->second<<" ";
		}
	}
	void printCostMap(std::map<double, Path<ListGraph> > &cost_path){
		for (int i = 0; i < cost_path.size(); i++)
		{
			//std::cout << cost_path.;
		}
	}
	Node getServerOfPath(const Path<ListGraph> path){
		PathNodes pit(path, graph);
		Node n1 = pit.beginNode();
		Node n2 = pit.endNode();

		bool sw = false;

		for (auto it = serverNodes.begin(); it != serverNodes.end(); it++){
			if (n1 == *it)
			{
				sw = true;
				return n1;
			}
			if (n2 == *it)
			{
				sw = true;
				return n2;
			}
		}
	}
	virtual Path<ListGraph> alloc(const int &width, const long int &dur)
	{
		Path<ListGraph> alloc = cost();
		GlobalSpectrumState::getInstance().Alloc(alloc, width, dur);
		return alloc;
	}

	
};
/**
* this class represnts a network where we don't mind the server load in routing,
* but computational capacity is assigned to every server node
* when comp capacity is reaches it's limit, the server node is out of routing
*/
class AnycastServerCap : public Anycast {
	ServerLoads server_loads; // szerverek terheltsége
	int comp_req;
public:
	AnycastServerCap(ListGraph &g) : Anycast(g){}

	void setReplicaServer(int id, int max_cap){
		serverNodes.push_back(graph.nodeFromId(id));
		server_loads.addServer(id, max_cap);
	}

	void runModDijkstraNoProt(Node s, const int &width, const long int &dur, int comp){
		comp_req = comp;
		server_loads.timeCheck();
		inRunClear();
		workingpath_modD(s, width, dur);  // modjiktra uzemi ut keres, ha van 		
	}

private:

	Path<ListGraph> cost(){// ha terhelés kisebb mint max spektrum akkor ok

		double lengt_val = 0;
		double load_val = 0;
		double cost = 0;
		std::map<double, int> map1; // cost index pairs

		for (int i = 0; i < paths.size(); i++)
		{
			Path<ListGraph> tp = paths[i];
			lengt_val = tp.length();
			Node s = getServerOfPath(tp);
			load_val = server_loads.get(graph.id(s));
			//if (load_val < server_loads.getMax(graph.id(s)))
			map1.insert(std::pair<double, int>(lengt_val, i));
		}
		int idx = map1.begin()->second;  //to-do test!!  kiír sorrendben set-et map-ot cost-ot				
		return paths[idx];
	}
	
	
	Path<ListGraph> alloc(const int &width, const long int &dur)
	{
		Path<ListGraph> alloc = cost();
		Node s = getServerOfPath(alloc);
		double load_val = server_loads.get(graph.id(s));
		if (load_val + comp_req >= server_loads.getMax(graph.id(s)))
			alloc.clear();
		if (!alloc.empty())
		{			
			server_loads.addLoad(graph.id(s), comp_req, dur);
			GlobalSpectrumState::getInstance().Alloc(alloc, width, dur);
			return alloc;
		}
		else{					
			return alloc;
		}
	}
};

/** ????? ha alosztalyban modostom ososztaly függvenyeit amit õ hav egy függvényben, akkor alosztályban hívva melyik hívódik?
* Class for server loads simulation
* 
* Changes: 1 addServer() node, max cap
* 2. timchek in every run, run has new parameters
* 3.rewirite cost function
* 4. addLoad in alloc
*/
class AnycastCUCA : public Anycast{
	ServerLoads server_loads; // szerverek terheltsége
	int comp_req;
public:
	AnycastCUCA(ListGraph &g): Anycast(g){}

	void setReplicaServer(int id,int max_cap){
		serverNodes.push_back(graph.nodeFromId(id));
		server_loads.addServer(id,max_cap);
	}
	
	void runModDijkstraNoProt(Node s, const int &width, const long int &dur,int comp){
		comp_req = comp;
		server_loads.timeCheck();
		inRunClear();
		workingpath_modD(s, width, dur);  // modjiktra uzemi ut keres, ha van 		
	}

private:

	double getServerLoad(int s){
		int cl =server_loads.get(s);
		int ml = server_loads.getMax(s);
		double val = (double)cl / (double)ml;
		return val;
	}

	double calcCost(double lengt_val, double load_val){
		double cost = 0;
		double tres = 0.5;
		if (load_val < tres)
		{
			cost = lengt_val + load_val;
		}
		else {
			//cost = lengt_val + exp(COST_CONST / (1 - load_val)); //0.4..2
			cost = lengt_val + exp(load_val*COST_CONST); //0.4..2
			//cost = lengt_val + (load_val*COST_CONST); //0.4..2

		}
		//if (load_val>1)
			//cout << cost << "load " << load_val << endl;
		return cost;
	}

	Path<ListGraph> cost(){// ha terhelés kisebb mint max spektrum akkor ok

		double lengt_val = 0;
		double load_val = 0;
		double cost = 0;
		std::map<double, int> map1; // cost index pairs

		for (int i = 0; i<paths.size(); i++)
		{
			Path<ListGraph> tp = paths[i];
			lengt_val = tp.length();
			Node s = getServerOfPath(tp);
			load_val = getServerLoad(graph.id(s)); //TO-DO test if % value
			//cout << "load val" << load_val << endl;
			cost = calcCost(lengt_val, load_val);
			if (server_loads.get(graph.id(s)) + comp_req < server_loads.getMax(graph.id(s)))
			{
				map1.insert(std::pair<double, int>(cost, i));			
				//map1.insert(std::pair<double, int>(lengt_val, i));
			}
		}
		if (map1.empty())
		{
			paths[0].clear();
			return paths[0];
		}
		int idx = map1.begin()->second;  //to-do test!!  kiír sorrendben set-et map-ot cost-ot				
		//for (auto it = map1.begin(); it != map1.end(); it++){
		//	cout << it->first<<" ";
		//}
		//cout << endl;
		return paths[idx];		
	}

	
	
	Path<ListGraph> alloc(const int &width, const long int &dur)
	{
		Path<ListGraph> alloc = cost();
		if (!alloc.empty())
		{
			Node s = getServerOfPath(alloc);
			server_loads.addLoad(graph.id(s), comp_req, dur);
			GlobalSpectrumState::getInstance().Alloc(alloc, width, dur);
			return alloc;
		}
		else{
			return alloc;
		}
	}
};