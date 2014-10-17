
#include"SharedProtection.h"
/**
* 
* Routing 1 ig�ny -> 1-4  utv�laszt�s RSA
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
	* futtat�s: for ciklus kisz�molja az �sszes szerverhez modDijkstraval az utat
	* paths v�ltoz�ba elmenti
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