// ModDijkstra.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



#include"utilities.h"
//#include"Kshort.h"
#include"Kshort_mod.h"
#include<lemon/random.h>

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
int GlobalSpectrumState::alloc_pos(0); 
int GlobalSpectrumState::blokknum(0);
int GlobalSpectrumState::ALLOCMOD(2);
int CH::channel_num(55);
	/*	if(ALLOCMOD==0)
		{return BaseSpectrumCheck(width,spectrum);}
		if(ALLOCMOD==1)
		{return GapFillSpectrumCheck(width,spectrum);}
		if(ALLOCMOD==2)
		{return TwoSideSpectrumCheck(width,spectrum);}
		*/
//long int GlobalSpectrumState::global_time(0);



int main(int argc, char** argv) {
  
     typedef dim2::Point<int> Point;
     typedef ListGraph::Node Node;
     typedef ListGraph::Edge Edge;
     typedef PathNodeIt<Path<ListGraph> > PN;
	
	 enum SASP{
	 MODDIJK=1,
	 KSHORT=2,
	 };
	     
  ListGraph graph;  
  graphReader(graph,"26_usa.lgf").run(); //.edgeMap("cost",lengthmap)
  ListDigraph digraph;  
  digraphReader(digraph,"26_usa.lgf").run(); // két grafot olvasunk be egy irányított és egy irányítatlan
  ListGraph::EdgeMap<int> lengthmap(graph);
  ListGraph::EdgeMap<bool> permittingmap(graph);
  ListGraph::NodeMap<_tpath> pathmap(graph); //minden csomóponthoz tartozó tak halmaza
  ListGraph::EdgeMap<SpectrumState> spectrum_map(graph);
  GlobalSpectrumState globalspectrum(graph,spectrum_map);
  for(ListGraph::EdgeIt it(graph);it!=INVALID;++it){
    lengthmap.set(it,1);
    }
  for(ListGraph::EdgeIt eit(graph);eit!=INVALID;++eit)
    {
        permittingmap.set(eit,true);
    }
 
    Node u;
    Node v;

    Node procNode;
    Node nextNode;
    Edge e;


    Deparallel para;
   para(graph);

    
	
    //----------!!!!!!Dijkstra init!!!!-----------
	/*
    Dijkstra<ListGraph,eMap> dijkstra(graph,lengthmap);
    dijkstra.init();
    dijkstra.modaddSource(s,pathmap);
	dijkstra.modstart(permittingmap,pathmap);
	
	Path<ListGraph> p;
    p=dijkstra.path(t);
	
    PN pit(graph,p);
    for(pit;pit!=INVALID;++pit)
    {
     data2.push_back(graph.id(pit));   
    }
	cout<<pathmap[graph.nodeFromId(10)].size();
	
	printNode(pathmap[graph.nodeFromId(10)],graph);
	cout<<endl;
	
	std::pair<_tpath,SpectrumState> proba;
	//std::pair<int,char> proba2; //proba2.first=10;proba2.second='m';
	std::pair<int,char> proba2=std::make_pair(3,'k');
	cout<<proba2.first<<proba2.second;
	//cout<<endl; dijkstra.fullassign(graph.nodeFromId(2));
	*/
 /*
   Edge e1=findEdge(graph,graph.nodeFromId(2),graph.nodeFromId(4));
   for(int i=4;i<5;i++){
   spectrum_map[e1][i]=1;
   }
   
  
   */

  /*
       Node s=graph.nodeFromId(21);
    Node t=graph.nodeFromId(22);
	ModDijkstra md(graph,spectrum_map,globalspectrum);
	
	if(!globalspectrum.EndToEnd(s,t,2,59)){
	md.run(s,t,2,59);
	}
	globalspectrum.TimeCheck();
	printSpectrum(spectrum_map,graph);
	 int width(3);
	*/ 	
	
	
	
	/*
	md.run(s,t,3,5);
	printSpectrum(spectrum_map,graph);
	globalspectrum.TimeCheck();
	printSpectrum(spectrum_map,graph);
	//globalspectrum.printlinks();
	cout<<endl;
	globalspectrum.TimeCheck();
	if(!globalspectrum.EndToEnd(s,t,width,5))
		{
			md.run(s,t,width,5);
		} 
	if(!globalspectrum.EndToEnd(s,t,width,5))
		{
			md.run(s,t,5,5);
		} 
	//globalspectrum.TimeCheck();
	cout<<"masodik"<<endl;
	//globalspectrum.printlinks();
	printSpectrum(spectrum_map,graph);
	
	  

 /*
    std::vector< vector<int> > Kshortvec;
    typedef ListDigraph::Node DiNode;
    DiNode dt=digraph.nodeFromId(2);
    DiNode ds=digraph.nodeFromId(10);
        
    KShortestPath ksrt(digraph,Kshortvec);
    ksrt.YenKSP(dt,ds,30);
    ksrt.print();
	Kshort_set kset(graph,Kshortvec,globalspectrum);
	kset.conv().alloc(3,5);
	printSpectrum(spectrum_map,graph);
	*/
	/*
	Dijkstra<ListGraph,eMap> dijkstra(graph,lengthmap,spectrum_map);
	
    dijkstra.init();
    dijkstra.addSource(s);
	dijkstra.start();
		Path<ListGraph> p;
    p=dijkstra.path(t);

	globalspectrum.Alloc(p,3,2);
	Node n1=graph.target(p.back());
	printSpectrum(spectrum_map,graph);
	globalspectrum.insertPath(p,3,1);
	globalspectrum.insertPath(p,1,4);
	p=dijkstra.path(graph.nodeFromId(14));
	globalspectrum.insertPath(p,1,4); Node a=graph.nodeFromId(14);
	globalspectrum.ask(s,a);
	*/
	/*
	path
	pos
	for()
	{

		if(!globalspectrum.EndToEnd(s,t,width))
		{
			md.run(s,t,width);
		}
	else{
	
	md.init();
	md.data(path,pos);
	globalspectrum.Alloc(path,pos);
	}
	}
	*/
	
	//printSpectrum(spectrum_map,graph);
	 typedef ListDigraph::Node DiNode;
 
	std::vector< vector<int> > Kshortvec;
	ModDijkstra md1(graph,spectrum_map,globalspectrum);
	KShortestPath ksrt(digraph,Kshortvec);   // Kshort Digraphon fut
	
    //DiNode dt=digraph.nodeFromId(19);
    //DiNode ds=digraph.nodeFromId(22);
	//ksrt.YenKSP(dt,ds,100);
    //ksrt.print();
	Random random;
	// DiNode dt=digraph.nodeFromId(14);
    //DiNode ds=digraph.nodeFromId(17);
	//ksrt.YenKSP(dt,ds,10);
	

	
	int counter=0;
	long int dursum(0),durcnt(0);
	int n1(0),n2(0),width1(0),k(0);long int dur=0;
	int K0=0;
	lemon::Random random1(random);
	cout<<endl;
	cout<<"ModDijkstra:1"<<endl<<"Kshortest path: 2"<<endl<<"SADijkstra:3"<<endl<<"KshortTEST:4"<<endl;
	cin>>k;
	int j(0);
	switch(k)
	{
	case MODDIJK:
		cout<<"Keresek szama:"<<endl;
		cin>>j;
		cout<<endl<<"dijkstra";
		
		for(int i2=0;i2<j;i2++)
		{
		
		n1=random1.integer(0,25); Node t1=graph.nodeFromId(n1);
		n2=random1.integer(0,25); Node s1=graph.nodeFromId(n2);
		dur=(long int)random1.exponential(0.03);
		width1=random1.integer(1,5);
		
		if(n1!=n2&&dur>0){
			if(!globalspectrum.EndToEnd(s1,t1,width1,dur))
			{
				md1.run(s1,t1,width1,dur);
			} 
			globalspectrum.TimeCheck();
		}
		//printSpectrum(spectrum_map,graph);
		cout<<endl<<i2;
		}
		printSpectrum(spectrum_map,graph);
		cout<<endl<<"BLOKKOLASOK:";
		cout<<globalspectrum.blokknum;
		break;

	case KSHORT:
		
		cout<<"Keresek szama:"<<endl;
		cin>>j;
		cout<<"K:";
		cin>>K0;
		cout<<endl<<"Kshort";
		for(int i2=0;i2<j;i2++)
		{
		n1=random1.integer(0,25); DiNode t1=digraph.nodeFromId(n1);
		n2=random1.integer(0,25); DiNode s1=digraph.nodeFromId(n2);
		Node t2=graph.nodeFromId(n1);
		Node s2=graph.nodeFromId(n2);
		dur=(long int)random1.exponential(0.03);
		dursum=dursum+dur;
		width1=random1.integer(1,5);
		
		if(n1!=n2&&dur>0){
			if(!globalspectrum.EndToEnd(s2,t2,width1,dur))
			{
				Kshortvec.clear();
				KShortestPath ksrt(digraph,Kshortvec);
				ksrt.YenKSP(t1,s1,K0);
				Kshort_set kset(graph,Kshortvec,globalspectrum);
				kset.conv().alloc(width1,dur);
				Kshortvec.clear();
			} 
			globalspectrum.TimeCheck();
		}
		else{counter++;}
		//printSpectrum(spectrum_map,graph);
		
		}
		printSpectrum(spectrum_map,graph);
		cout<<endl<<"BLOKKOLASOK:";
		cout<<endl<<"counter"<<counter;
		cout<<endl<<"atlag"<<dursum/j;
		cout<<globalspectrum.blokknum;
		break;
	case 3: //SADIJKSTRA
		cout<<"Keresek szama:"<<endl;
		cin>>j;
	
		
		cout<<endl<<"SADijkstra";
		for(int i2=0;i2<j;i2++)
		{
		n1=random1.integer(0,25); 
		n2=random1.integer(0,25); 
		Node t2=graph.nodeFromId(n1);
		Node s2=graph.nodeFromId(n2);
		dur=(long int)random1.exponential(0.03);
		
		width1=random1.integer(1,5);
	
		if(n1!=n2&&dur>0){
			if(!globalspectrum.EndToEnd(s2,t2,width1,dur))
			{
				md1.runSADijkstra(s2,t2,width1,dur);
			} 
			globalspectrum.TimeCheck();
		}

		//printSpectrum(spectrum_map,graph);
		
		}
		printSpectrum(spectrum_map,graph);
		cout<<endl<<"BLOKKOLASOK:";
		cout<<globalspectrum.blokknum;


		break;
	case 4:
		while(cin.get()=='\n'){
		cout<<"Source:";
		cin>>n1;
		cout<<"Target:";
		cin>>n2;
		cout<<"K:";
		cin>>width1;
		dur=10;
		DiNode dt=digraph.nodeFromId(n1);
		DiNode ds=digraph.nodeFromId(n2);
		Kshortvec.clear();
				
		ksrt.YenKSP(dt,ds,width1);
	
			
				
		ksrt.print();
		ksrt.init();
	
		Kshortvec.clear();
		}
	}
	
	
	/*	
	int j(0);
	std::vector<int> K;K.push_back(1);K.push_back(2);K.push_back(5);K.push_back(10);K.push_back(20);
	cout<<CH::channel_num<<endl<<"Keresek szama:"<<endl;
		cin>>j;
	for(GlobalSpectrumState::ALLOCMOD=0;GlobalSpectrumState::ALLOCMOD<3;GlobalSpectrumState::ALLOCMOD++)
	{
		
		for(int i1=0;i1<K.size();i1++)
		{
			ListGraph::EdgeMap<SpectrumState> spectrum_map(graph);
			GlobalSpectrumState globalspectrum(graph,spectrum_map);
			
			lemon::Random random1(random);
			for(int i2=0;i2<j;i2++)
			{
				
			n1=random1.integer(0,27); DiNode t1=digraph.nodeFromId(n1);
			n2=random1.integer(0,27); DiNode s1=digraph.nodeFromId(n2);
			Node t2=graph.nodeFromId(n1);
			Node s2=graph.nodeFromId(n2);
			dur=(long int)random1.exponential(0.03);
			
			width1=random1.integer(1,5);
		
			if(n1!=n2&&dur>0){
				if(!globalspectrum.EndToEnd(s2,t2,width1,dur))
				{
					Kshortvec.clear();
					KShortestPath ksrt(digraph,Kshortvec);
					ksrt.YenKSP(t1,s1,K[i1]);
					Kshort_set kset(graph,Kshortvec,globalspectrum);
					kset.conv().alloc(width1,dur);
					Kshortvec.clear();
				} 
				globalspectrum.TimeCheck();
			}

			
			
			}
			
			cout<<endl<<"ALLOCMOD"<<GlobalSpectrumState::ALLOCMOD<<endl;
			cout<<"K:"<<K[i1];
			cout<<endl<<"BLOKKOLASOK:";
			cout<<globalspectrum.blokknum<<endl;
			globalspectrum.blokknum=0;
		}
		 ListGraph::EdgeMap<SpectrumState> spectrum_map1(graph);
		GlobalSpectrumState globalspectrum1(graph,spectrum_map1);
		ModDijkstra md1(graph,spectrum_map1,globalspectrum1);
		
		lemon::Random random1(random);
		for(int i2=0;i2<j;i2++)
		{
		
		n1=random1.integer(0,27); Node t1=graph.nodeFromId(n1);
		n2=random1.integer(0,27); Node s1=graph.nodeFromId(n2);
		dur=(long int)random1.exponential(0.03);
		width1=random1.integer(1,5);
		
		if(n1!=n2&&dur>0){
			if(!globalspectrum1.EndToEnd(s1,t1,width1,dur))
			{
				md1.run(s1,t1,width1,dur);
			} 
			globalspectrum1.TimeCheck();
		}
		//printSpectrum(spectrum_map,graph);
		
		}	
			cout<<endl<<"ALLOCMOD"<<GlobalSpectrumState::ALLOCMOD<<endl;
			cout<<"MODDIJKSTRA"<<endl;
			cout<<"BLOKKOLASOK:";
			cout<<globalspectrum.blokknum<<endl;
			globalspectrum.blokknum=0;


	}
	//printSpectrum(spectrum_map,graph);
	/*lemon::Random random1(random);
	int r(0);
	for(int i=0;i<10;i++)
	{
		lemon::Random random1(random);
		for(int j=0;j<10;j++)
		{
			r=random1.integer(1,10);
			cout<<r<<endl;
		}
		cout<<"UJ KOR"<<endl;
	}
	*/
	




	getchar();
	getchar();
    return 0;

	
	
}


