#include"SharedProtection.h";
#include<lemon/suurballe.h>

class SuurballeRoute{
public:
	void run(ListGraph gr, ListGraph::EdgeMap<int> map){
		//lemon::csabi::Dijkstra<GR, cost_Map> dijkstra(graph, *lengthmap, spectrum_map);
		//lemon::Suurballe<>()
		Suurballe<ListGraph, ListGraph::EdgeMap<int> > suurballe(gr,map);
		//suurballe.
	}
};