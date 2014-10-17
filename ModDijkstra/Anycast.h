
#include"SharedProtection.h"
/**
* 
* Routing 1 igény -> 1-4  utválasztás RSA
*/
class Anycast{
	std::vector<Node> serverNodes;
	std::multiset<Path<ListGraph>,compPath> paths;
	ListGraph &graph;
	int anycastblock;
public:
	Anycast(ListGraph &g):graph(g){}
	void setReplicaServer(int id){
		serverNodes.push_back(graph.nodeFromId(id));
	}
	/**
	* futtatás: for ciklus kiszámolja az összes szerverhez modDijkstraval az utat
	* paths változóba elmenti
	*/
	bool runModDijkstra(Node s, const int &width, const long int &dur){
		paths.clear();
		Path<ListGraph> p_path;
		ModDijkstra<ListGraph> md_dijkstra(graph);
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
		else{ 
			anycastblock++;
			return false; 
		}
	}
	void runKshort(){}
	Path<ListGraph> allocatedPath(){
		return *paths.begin();
	}
	int getBlock(){ return anycastblock; }
private:
	void alloc(const int &width, const long int &dur)
	{	
		Path<ListGraph> tp = *paths.begin();
		GlobalSpectrumState::getInstance().Alloc(tp, width, dur);
	}

};