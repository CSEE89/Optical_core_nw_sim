/* 
 * File:   Kshort.h
 * Author: csabi
 *
 * Created on 2013. február 13., 23:51
 */
#ifndef KSHORT_H
#define	KSHORT_H
#include<lemon/core.h>
#include<iostream>
#include<lemon/lgf_reader.h>
#include<lemon/dijkstra.h>
#include<queue>
#include<algorithm>
#include<lemon/random.h>
#include<lemon/list_graph.h>
#include<lemon/adaptors.h>
#include<functional>


using namespace lemon;
using namespace std;



class DefaultAlgorithm{


public:
	bool protection_run; // 1 értle jelzi hogy védelmi utat foglalunk-e
	virtual void run(Node s, Node t, const int &width, const long int &timestamp) = 0;
	virtual bool calcpath(Node s, Node t, const int &width) = 0;


};


//-------------------------------------------------------------------------
template<typename T> class KShortestPath{
protected:

	typedef T GR;
	typedef typename GR::template ArcMap<int> Length_Map; 
	typedef typename GR::template NodeMap<int> Distance_Map;
	typedef typename GR::template NodeMap<bool> NodeFilter_Map;
	typedef typename GR::template EdgeMap<bool> EdgeFilter_Map;
	typedef ListGraph::Node Node;
	typedef ListGraph::Arc Arc;
	std::vector< vector<int> > A;
	GR &graph;

	class comp{
	public:
		bool operator()(const std::vector<int> &a, const std::vector<int> &b)
		{
			return a.size()<b.size();
		}
	};
	std::multiset< vector<int>, comp> _set;
public:

	KShortestPath(GR &graph) :graph(graph), A(){}

	bool YenKshort(Node source, Node sink, int K){
		// Determine the shortest path from the source to the sink.
		//-----SubGraph tesztelő---------

		Node spurNode;
		Arc E1;
		Arc E2;
		Node N1;
		Node N2;
		std::vector<int> nodes;
		std::vector<int> arcs;
		std::vector<int> rootPath;
		std::vector<int> spurPath;
		std::vector<int> totalPath;
		std::vector<int> obs;
		std::vector<int>::iterator it;
		std::vector<int>::iterator it2;
		bool allow;
		std::vector<ListDigraph::Node> nodevec;
		//lengthmap a Dijkstrahoz
		Length_Map lengthmap(graph);
		Distance_Map dist(graph);
		NodeFilter_Map node_filter(graph);
		EdgeFilter_Map arc_filter(graph);
		Path<GR> p;
		for (GR::ArcIt it(graph); it != INVALID; ++it)
		{
			lengthmap.set(it, 1);
		}


		for (GR::NodeIt it(graph); it != INVALID; ++it){ node_filter.set(it, true); }
		for (GR::ArcIt it(graph); it != INVALID; ++it){ arc_filter.set(it, true); }
		//---------------SUBGRAPH------------------------       
		SubGraph<GR, NodeFilter_Map, EdgeFilter_Map >
			subDigraph(graph, node_filter, arc_filter);

		dijkstra(graph, lengthmap).distMap(dist).path(p).run(source, sink);
		PathNodeIt<lemon::Path<GR> > pn(graph, p);
		typedef PathNodeIt<lemon::Path<GR> > PathNodeIt;


		for (pn; pn != INVALID; ++pn)
		{
			nodes.push_back(graph.id(pn));
		}


		A.push_back(nodes);



		// Initialize the heap to store the potential kth shortest path.

		p.clear();
		//Iteartion k
		int size = 0;
		for (int k = 1; k<K; k++){
			//I.     
			if (k <= A.size()){ size = (A[k - 1].size() - 1); }
			else return false;
			for (int i = 0; i<size; i++){
				// (a)       

				spurNode = graph.nodeFromId(A[k - 1][i]);

				it = A[k - 1].begin();
				it2 = it + i;
				rootPath.clear();

				rootPath.assign(it, it2);
				for (size_t lep = 0; lep < A.size(); ++lep)
					//for(int lep=0;lep<A.size();lep++)
				{
					if ((A[lep].size() - 1)>i)
					{
						if (coincide(rootPath, A[lep]))
						{

							E1 = findArc(subDigraph, subDigraph.nodeFromId(A[lep][i]), subDigraph.nodeFromId(A[lep][i + 1]));
							E2 = findArc(subDigraph, subDigraph.nodeFromId(A[lep][i + 1]), subDigraph.nodeFromId(A[lep][i]));
							if ((E1 != INVALID) || (E2 != INVALID))
							{
								subDigraph.disable(E1);
								subDigraph.disable(E2);

							}
						}

					}


				}
				// (b)
				spurPath.clear();
				//----------------graph-rootPath-------------
				nodevec.clear();
				for (int j = 0; j<rootPath.size(); j++)
				{
					N1 = subDigraph.nodeFromId(rootPath[j]);

					if (N1 != INVALID)
					{
						subDigraph.disable(N1);

					}
				}
				p.clear();

				if (dijkstra(subDigraph, lengthmap).run(spurNode, sink) == true)
				{

					dijkstra(subDigraph, lengthmap).distMap(dist).path(p).run(spurNode, sink);
					PathNodeIt pit(graph, p);

					for (pit; pit != INVALID; ++pit)
					{
						spurPath.push_back(graph.id(pit));
					}

					// (c)     

					if (rootPath.empty())
					{
						_set.insert(spurPath);
					}
					else
					{

						rootPath.insert(rootPath.end(), spurPath.begin(), spurPath.end());
						_set.insert(rootPath);

					}
				}
				for (GR::ArcIt it(graph); it != INVALID; ++it){ arc_filter.set(it, true); }
				for (GR::NodeIt it(graph); it != INVALID; ++it){ node_filter.set(it, true); }
				for (int j = 0; j<rootPath.size(); j++)
				{
					N1 = subDigraph.nodeFromId(rootPath[j]);

					if (N1 != INVALID)
					{
						subDigraph.enable(N1);

					}
				}

			}

			//  II.
			if (!_set.empty()){
				std::vector<int> tmp = *_set.begin();
				A.push_back(tmp);
				_set.erase(_set.begin());
				std::sort(A.begin(), A.end(), comp());
			}
		}
		return true;
	}
	void init(){ this->A.clear(); this->_set.clear(); }
	template<typename foo> bool iselement(std::vector<foo> &vec, foo &element)
	{
		for (std::vector<foo>::iterator it = vec.begin(); it != vec.end(); it++)
		{
			foo tmp = *it;
			if (tmp == element) return true;
		}
		return false;
	}

	bool coincide(const std::vector<int> &a, const std::vector<int> &b)
	{

		if (((b.size() - 1)>a.size()))
		{

			for (int i = 0; i<a.size(); i++){
				//if(b.size()==i) return false;
				if (a[i] != b.at(i)) return false;
			}
			return true;

		}
		else{
			return false;
		}
	}
	void print()
	{
		for (std::vector< vector<int> >::iterator it = A.begin(); it != A.end(); it++)
		{
			for (std::vector<int>::iterator it2 = it->begin(); it2 != it->end(); it2++)
			{
				cout << *it2 << " ";

			}
			cout << endl;
		}
	}

	void printB(std::vector<vector<int> > &vec)
	{
		for (std::vector< vector<int> >::iterator it = vec.begin(); it != vec.end(); it++)
		{
			for (std::vector<int>::iterator it2 = it->begin(); it2 != it->end(); it2++)
			{
				cout << *it2 << " ";
			}
			cout << endl;
		}
	}

};

template <class T,class X>
std::ostream &operator<<(std::ostream &a, const std::set<T,X> &b)
{
	a<<"(";
		// typename explicit kiírása
	for (typename std::set<T,X>::const_iterator csi=b.begin(); csi!=b.end(); csi++)
		a<<(csi==b.begin()?"":", ")<<*csi;
	return a<<")";
};

template <class T>
std::ostream &operator<<(std::ostream &a, const std::list<T> &b)
{
	a<<"(";
		// typename explicit kiírása
	for (typename std::list<T>::const_iterator cli=b.begin(); cli!=b.end(); cli++)
		a<<(cli==b.begin()?"":", ")<<*cli;
	return a<<")";
};

template <class T>
std::ostream &operator<<(std::ostream &a, const std::vector<T> &b)
{
	a<<"[";
		// typename explicit kiírása
	for (typename std::vector<T>::const_iterator cvi=b.begin(); cvi!=b.end(); cvi++)
		a<<(cvi==b.begin()?"":", ")<<*cvi;
	return a<<"]";
};
#endif	/* KSHORT_H */

