

#ifndef STL_OUT
#define STL_OUT

#include <iostream>
#include <set>
#include <list>
#include <vector>
#include<lemon/list_graph.h>
#include<lemon/path.h>
#include <cstdlib>
#include"moddijkstra.h"



namespace lemon
{ 
	
	
	
	void printPath(listpath &path,const ListGraph &graph)
	{
		std::cout<<"[";
			  PathNodeIt<Path<ListGraph> > PN(graph,path);
			for(PN;PN!=INVALID;PN++)
				{
					std::cout<<", "<<graph.id(PN);
				}
			
		 std::cout<<"]";
		 std::cout<<std::endl;
	}
	void printSet(pathpair lp,const ListGraph &graph)
	{
		lemon::Path<ListGraph> path; 
		path=lp.first;
			std::cout<<"[";
			  PathNodeIt<Path<ListGraph> > PN(graph,path);
			for(PN;PN!=INVALID;PN++)
				{
					std::cout<<", "<<graph.id(PN);
				}
			
		 std::cout<<"]";
		 std::cout<<std::endl;
	}

	void printNode(const pathpair_vector &vec, const ListGraph &graph)
	{
		for (pathpair_vector::const_iterator cvi = vec.begin(); cvi != vec.end(); cvi++)
		{
			lemon::Path<ListGraph> path; 
			path=cvi->first;
			std::cout<<"[";
			  PathNodeIt<Path<ListGraph> > PN(graph,path);
			for(PN;PN!=INVALID;PN++)
				{
					std::cout<<", "<<graph.id(PN);
				}
			
		 std::cout<<"]"<<std::endl;
		}
		
	
	}
	
	void printSpectrum(ListGraph::EdgeMap<SpectrumState> &spectrum_map,const ListGraph &g)
	{
		ListGraph::EdgeIt eit(g);
		for(eit;eit!=INVALID;++eit)
		{
			spectrum_map[eit].print();
			std::cout<<std::endl;
		}
	
	}
	/*
	std::ostream& operator<<(std::ostream &a, const listpath &vec,)
	{
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			a << graph.id(arc_it) << " ";
		}
		return a;
	}

	*/
}
#endif