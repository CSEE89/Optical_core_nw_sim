
#ifndef SPECTRUMSTATE
#define SPECTRUMSTATE
#include "general_traits.h"





	
struct GAP{
		int pos;
		int width;
		GAP(int pos,int width):pos(pos),width(width){}
		GAP operator()(int pos,int width)
		{
			GAP g(pos,width);
		return g;
		}
};
class GAPcmp{
public:
	bool operator()(const GAP &g1,const GAP &g2)
	{
		return g1.width>g2.width;
	}
};

struct ProtectionPathMatrix{
	Path<ListGraph>  path;
	int pos;
	int width;
	int timestamp;
	ProtectionPathMatrix(){}
	ProtectionPathMatrix(Path<ListGraph>  &path, const int &pos, const int &width, const long int &timestamp) :path(path), pos(pos), width(width), timestamp(timestamp){}
	~ProtectionPathMatrix()
	{
		path.clear();
	}
};
/**
Global spektrum path_matrix váltózójának <value> tagja,
Entitás ami egy útvonalat tartalmaz és hozzá tartozó pozíciót, szélességet és élettartamot
*/
struct PathMatrix{
	Path<ListGraph>  path;
	int unquie_key; // megegyezik a protection path key-el, minden őj útnak más
	int pos;
	int width;
	int timestamp;
	PathMatrix(int k,Path<ListGraph>  &path, const int &pos, const int &width, const long int &timestamp) :unquie_key(k), path(path), pos(pos), width(width), timestamp(timestamp){}
	~PathMatrix()
	{
		path.clear();
	}
};

/**
*Spektrumlefoglalásért felelős osztály
* Csak egy lehet belőle
*	tartalmazza a spektrumot 
*	lefoglalt üzemi és védelmi utakat
*/
class GlobalSpectrumState{
protected:
	std::vector< std::vector<int> > traffic_matrix; //van e már összeköttetés a 2 pont között, nincs használva
	 // hálózat spektruma
	std::multimap<int,PathMatrix> path_matrix;// tárolja a két pont közötti összeköttetések útvonalait, spektrum pozicióját és szélességét, az összes meglévő link itt van, minden 2 ponthoz
	std::map<int, ProtectionPathMatrix> pm;  //védelmi utak
	std::pair <std::multimap<int,PathMatrix>::iterator, std::multimap<int,PathMatrix>::iterator> end2end_paths;//EndToEnd-ben
	ListGraph &graph;
	int n;  //csomópontok száma
	static int alloc_pos;   //belső tagváltozót váltizatatnak a függvények, innen felfelé lesz lefoglalva a sávszélesség
	
	
	
public:
	ListGraph::EdgeMap<SpectrumState> &spectrum_map;
	static int global_key;  //protecton_path_matrix key erteke, szerepel path_matrix elemeiben
	ALLOC ALLOCMOD;  // lefoglalási strategia
	static int blokknum;  // blokkolások száma
	static int protection_blokknum; //védelmi utak blokkolás száma
	//static bool isblocked;  // Az üzemi útnál blokkolást kaptunk-e,  az adott algoritmus állítja true-ra és a GlobalSpectrumState::alloc flase-ra
	static bool protection_round; // védelmi e
	friend class SharedProtection;

	GlobalSpectrumState(ListGraph &graph, ListGraph::EdgeMap<SpectrumState> &map) :graph(graph), spectrum_map(map), path_matrix(), end2end_paths()
	{
		
		n=lemon::countNodes(graph);
		std::vector<int> tmp(n,0);
		for(int i=0;i<n;i++)
			{
				traffic_matrix.push_back(tmp);
			}
	}

	/**
	*Az osztály alaphelyzetbe állítása
	*/
	void clear()
	{
		traffic_matrix.clear();
		std::vector<int> tmp(n, 0);
		for (int i = 0; i<n; i++)
		{
			traffic_matrix.push_back(tmp);
		}
		path_matrix.clear();
		pm.clear();
		std::pair <std::multimap<int, PathMatrix>::iterator, std::multimap<int, PathMatrix>::iterator> nul;
		end2end_paths = nul;
		alloc_pos = 0;
		SpectrumState s;
		for (ListGraph::EdgeIt it(graph); it != INVALID; ++it)
		{
			spectrum_map[it] = s;
		}
		global_key = 0;
		blokknum = 0;
		protection_blokknum = 0;	
	}
	//Allocól hívva beteszi az utat path_matrixba
	void insertPath(lemon::Path<ListGraph> &path,const int &width,const int &pos,const long int &timestamp)
	{
		global_key++;
		int key(0),s(0),t(0);		
		ListGraph::Node start=graph.source(path.front());
		ListGraph::Node end=graph.target(path.back());;
		s=graph.id(start);
		t=graph.id(end);
		if(s>t){int tmp=s;s=t;t=tmp;}
		key=s*(n-1)+t;
		PathMatrix tempMatrix(global_key, path, pos, width, timestamp);
		path_matrix.insert(std::pair<int,PathMatrix>(key,tempMatrix));				
	}
	
	// egy PathMatrix elem kiírása
	void print(const PathMatrix &pm)
	{
		lemon::Path<ListGraph> path; 
		path=pm.path;
			std::cout<<"[";
			  PathNodeIt<Path<ListGraph> > PN(graph,path);
			for(PN;PN!=INVALID;PN++)
				{
					std::cout<<", "<<graph.id(PN);
				}
			
		 std::cout<<"]";
		 std::cout<<std::endl;
		 std::cout<<pm.pos<<std::endl;
		 std::cout<<pm.width<<std::endl;
	}

	void printlinks(Node start,Node end)
	{
		int key(0),s(0),t(0);
		s=graph.id(start);
		t=graph.id(end);
		if(s>t){int tmp=s;s=t;t=tmp;}
		key=s*(n-1)+t;
		
		std::pair <std::multimap<int,PathMatrix>::iterator, std::multimap<int,PathMatrix>::iterator> its;
		its = path_matrix.equal_range(key);
		
		for (std::multimap<int,PathMatrix>::iterator it=its.first; it!=its.second; ++it){
			print(it->second);
		}

	}
	void printlinks()
	{
		for (std::multimap<int,PathMatrix>::iterator it=path_matrix.begin(); it!=path_matrix.end(); ++it){
			print(it->second);
		}

	}
	/**
	*Létezik-e már link a 2 pont között
	*ha igen azokat elmenti end2end_paths változóba
	*/
	bool linkcheck(Node start,Node end)
	{
		int key(0),s(0),t(0);  //key a path_matrix elemeit jelöli ki
		s=graph.id(start);
		t=graph.id(end);
		if(s>t){int tmp=s;s=t;t=tmp;}
		key=s*(n-1)+t; 
		if(path_matrix.count(key))
		{
			end2end_paths = path_matrix.equal_range(key);
			return true;
		}
		else
		{
			return false;
		}
		
		
	}
	// Megfeleő spektrumszelet 0-ról 1-re állítása az út mentén\
	Pathmatrixban elhelyzi a link adatait
	void Alloc(lemon::Path<ListGraph> &path,const int &width,const long int &timestamp,int index=1)
	{
		
		
		int n1,n2;
		n1=graph.id(graph.source(path.front()));
		n2=graph.id(graph.source(path.back()));  //itt mért nem target?		
		Path<ListGraph>::ArcIt arc_it(path);
		for(arc_it;arc_it!=INVALID;++arc_it)
		{
				Node t=graph.target(arc_it);
				Node s=graph.source(arc_it);
				Edge e=lemon::findEdge(graph,t,s);
			for(int i=0;i<width;i++){
			spectrum_map[e].carrier[alloc_pos+i]=index;
			}
		}
		if(n2>=n1){int tmp=n1; n1=n2; n2=tmp;}
		traffic_matrix[n1][n2]++;
		insertPath(path,width,alloc_pos,timestamp);
		alloc_pos = -1;
	}
	
	// Timechechk hívja meg, az adott 
	void Dealloc(const PathMatrix &matrix)
	{

		Path<ListGraph>::ArcIt arc_it(matrix.path);
		for(arc_it;arc_it!=INVALID;++arc_it)
		{
				Node t=graph.target(arc_it);
				Node s=graph.source(arc_it);
				Edge e=lemon::findEdge(graph,t,s);
				for(int i=0;i<matrix.width;i++)
			{
				spectrum_map[e].carrier[matrix.pos+i]=0;
			}
		}		
	}
	
	//Timcehcek hívja, védelmi út felszabadítása
	void Dealloc(const ProtectionPathMatrix &matrix)
	{

		Path<ListGraph>::ArcIt arc_it(matrix.path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph.target(arc_it);
			Node s = graph.source(arc_it);
			Edge e = lemon::findEdge(graph, t, s);
			for (int i = 0; i<matrix.width; i++)
			{
				spectrum_map[e].carrier[matrix.pos + i] = 0;
			}
		}
	}

	//visszaadja egy útvonal spektrumát
	SpectrumState PathSpectrum(const Path<ListGraph> &path)
	{
		SpectrumState spectrum;	
		Path<ListGraph>::ArcIt arc_it(path);
		for(arc_it;arc_it!=INVALID;++arc_it)
		{
				Node t=graph.target(arc_it);
				Node s=graph.source(arc_it);
				Edge e=lemon::findEdge(graph,t,s);
				spectrum.or( spectrum_map[e]);
			
		}
		return spectrum;
	}
	/*
	megnézi h van-e ilyen összeköttetés
	ha van megnézi h van elég szabad spektrum és mellé allokál,
	elhelyezi a linket path_matrixban
	ha nincs szabad spektrum false-al tér vissza
	*/
	bool EndToEnd(Node s,Node t,const int &width,const long int &timestamp)
	{
		SpectrumState spectrum;
		if (!linkcheck(s, t))
		{
			
			return false; //linkchek  adott ret-nek utakat
		}
		for (std::multimap<int, PathMatrix>::iterator it = end2end_paths.first; it != end2end_paths.second; ++it)
		{
			//egy-egy ut spektruma

			if(EndToEndSpectrumCheck(it->second,width))//visszatér egy true-val és beállítja allocpos-t ha van az út mellett hely
			{
				Alloc(it->second.path,width,timestamp,1);
				return true;
			}
		}
		return false;
	}
	// Megnézzük van e már út a 2 pont között ha van\
	megnézzük hogy élfüggetlen-e az üzemitől

	bool dedicated_EndToEnd(Node s, Node t, const int &width, const long int &timestamp,Path<ListGraph> &allocated_path)
	{
		SpectrumState spectrum;
		if (!linkcheck(s, t))return false; //linkchek  adott ret-nek utakat
		for (std::multimap<int, PathMatrix>::iterator it = end2end_paths.first; it != end2end_paths.second; ++it)
		{
			//egy-egy ut spektruma
			if (isEdgeDisjoint(allocated_path, it->second.path)){
				if (EndToEndSpectrumCheck(it->second, width))//visszatér egy true-val és beállítja allocpos-t ha van az út mellett hely
				{
					Alloc(it->second.path, width, timestamp);
					return true;
				}
			}
		}
		return false;
	}

	//Két Path<ListGraph> egyenlő-e ?  IGAZ HA KÜLÖNBÖZNEK\
	Először működjön jól aztán hatékonyan
	bool isNodeDisjoint(Path<ListGraph> &p1, Path<ListGraph> &p2){
		if (p1.length() != p2.length()) return true;
		PathNodeIt<Path<ListGraph> > pnit1(graph, p1);
		PathNodeIt<Path<ListGraph> > pnit2(graph, p2);
		std::set<int> nodeset1;
		std::set<int> nodeset2;
		for (pnit1, pnit2; pnit1 != INVALID, pnit2 != INVALID; ++pnit1, ++pnit1){
			nodeset1.insert(graph.id(pnit1));
			nodeset1.insert(graph.id(pnit2));
		}
		if (nodeset1 == nodeset2) return false;
		return true;
	}

	bool isEdgeDisjoint(Path<ListGraph> &p1, Path<ListGraph> &p2){
		PathNodeIt<Path<ListGraph> > pnit1(graph, p1);
		PathNodeIt<Path<ListGraph> > pnit2(graph, p1);
		Node temp;
		std::set<int> Edgeset1;
		std::set<int> Edgeset2;
		++pnit2;
		for (pnit1, pnit2; pnit1 != INVALID, pnit2 != INVALID; ++pnit1, ++pnit2){
			Edgeset1.insert(graph.id(findEdge(graph, pnit1, pnit2)));
		}
		PathNodeIt<Path<ListGraph> > pnit3(graph, p2);
		PathNodeIt<Path<ListGraph> > pnit4(graph, p2);
		++pnit4;
		for (pnit3, pnit4; pnit3 != INVALID, pnit4 != INVALID; ++pnit3, ++pnit4){
			Edgeset2.insert(graph.id(findEdge(graph, pnit3, pnit4)));
		}
		std::set<int> s3;
		std::insert_iterator<std::set<int> > s3ii(s3, s3.begin());
		std::set_intersection(Edgeset1.begin(), Edgeset1.end(), Edgeset2.begin(), Edgeset2.end(), s3ii);
		if (s3.empty())return true;
		return false;

	}

	// EndtoEnd hívja van-e elég szabad spektrum az út mellett\
	End-to-End Grooming
	bool EndToEndSpectrumCheck(PathMatrix &m,const int &width)
	{
		SpectrumState spectrum=PathSpectrum(m.path);
		int pos=m.pos;
		int band=m.width;
		int gapwidth(0);
		for(int i=((pos-width-1)>=0?(pos-width-1):(pos+band));i<((pos+band+width+1)<=CH::channel_num?(pos+band+width+1):CH::channel_num);i++)
		{
			if(!spectrum[i]) {++gapwidth;}
			else
			{
				gapwidth=0;
			}
			
			if(gapwidth==(width+1))
			{
				if(i<pos){
				alloc_pos=(i+1-width);
				return true;
				}
				else if(i>(pos+band))
				{
					alloc_pos=(i-width);
					return true;
				}
			}
		}
		return false;
		
	}

	// spektrum kitöltési stratégia kiválasztása
	bool checkSelector(int width,SpectrumState &spectrum)
	{
		//if (protection_round)
		//{
			//return OtherBaseSpectrumCheck(width, spectrum); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!4
		//}
		if(ALLOCMOD==0)
		{return BaseSpectrumCheck(width,spectrum);}
		if(ALLOCMOD==1)
		{return GapFillSpectrumCheck(width,spectrum);}
		if(ALLOCMOD==2)
		{return TwoSideSpectrumCheck(width,spectrum);}
	}
	//\
	spektrumkitöltési stratégiák az alloc_pos-t állítják  be\
	ótvonalhoz tartozó spektrumok kapnak
	
	static bool TwoSideSpectrumCheck(int width,SpectrumState &spectrum)
	{
		int gapwidth1(0),gapwidth2(0);
		for(int i=0,j=CH::channel_num-1;i<CH::channel_num&&j>0;i++,j--)
		{
			if(!spectrum[i]) {++gapwidth1;}
			else
			{
				gapwidth1=0;
			}
			
			if(gapwidth1==(width+2))
			{
				alloc_pos=(i-width);
				return true;
			}
			if(!spectrum[j]) {++gapwidth2;}
			else
			{
				gapwidth2=0;
			}
			
			if(gapwidth2==(width+2))
			{
				alloc_pos=(j+1);// itt kell máshogy beálltíani
				return true;
			}

		}
		return false;
	}

	// hézag kitöltési spektrum allokálás
	static bool GapFillSpectrumCheck(int width,SpectrumState &spectrum)
	{
		int gapwidth(0);
		std::multiset<GAP,GAPcmp> gaps;
		for(int i=0;i<CH::channel_num;i++)
		{
			if(!spectrum[i]) {++gapwidth;}
			else
			{
				if(gapwidth>width+2)
				{
					gaps.insert(GAP(i-gapwidth+1,gapwidth));
				}
				gapwidth=0;
			}
		}

	
		if(gapwidth>width+2)
			{
				gaps.insert(GAP(CH::channel_num-gapwidth,gapwidth));
			}	
				
		std::multiset<GAP,GAPcmp>::iterator it=gaps.begin();
		if(!gaps.empty()){
			int tpos=(int)it->pos;
			int tpos2=(int)it->width;
			alloc_pos=(int)it->pos+((int)it->width-width)/2; //a legnagyobb hézag közepe
			
			return true;
		}
		return false;
	}

	// egy irányú spektrum allokálás
	static bool BaseSpectrumCheck(int width,SpectrumState &spectrum)
	{
		int gapwidth(0);
		for(int i=0;i<CH::channel_num;i++)
		{
			if(!spectrum[i]) {++gapwidth;}
			else
			{
				gapwidth=0;
			}
			
			if(gapwidth==(width+2))
			{
				alloc_pos=(i-width);
				return true;
			}
		}
		return false;
	}

	static bool OtherBaseSpectrumCheck(int width, SpectrumState &spectrum)
	{
		int gapwidth(0);
		for (int i = CH::channel_num-1; i>0; i--)
		{
			if (!spectrum[i]) { ++gapwidth; }
			else
			{
				gapwidth = 0;
			}

			if (gapwidth == (width + 2))
			{
				alloc_pos = (i+1);
				return true;
			}
		}
		return false;
	}
	
	//utilites.h ModDijkstra setperm tagfüggveny használja
	
	static bool SetPermittingMap(const int &width,SpectrumState &spectrum)
	{
		int gapwidth(0);
		for(int i=0;i<CH::channel_num;i++)
		{
			if(!spectrum[i]) {++gapwidth;}
			else
			{
				gapwidth=0;
			}
			
			if(gapwidth==(width+1))
			{
				
				return true;
			}
		}
		return false;
	}
	// path_matrix összes elemén elenörzi hogy lejárt e az út\
	ha igen törli path_matrix-ból és felszabadítja a spektrumot
	void TimeCheck()
	{
		
		std::multimap<int,PathMatrix>::iterator eit;
		std::vector<std::multimap<int,PathMatrix>::iterator> veci;
		
		for(std::multimap<int,PathMatrix>::iterator it=path_matrix.begin();it!=path_matrix.end();it++)
		{
			
			it->second.timestamp--;
			if(it->second.timestamp==0)
			{			
			 eit=it;
			 veci.push_back(eit);
			 Dealloc(it->second);
			}
			
		}
		for(int i=0;i<veci.size();i++)
		{
			path_matrix.erase(veci[i]);
		}
		veci.clear();

		//protection-ra
		std::vector<int> obsolate;
		
		for (auto pit = pm.begin(); pit != pm.end(); ++pit)
		{

			pit->second.timestamp--;
			
			if (pit->second.timestamp == 0){ obsolate.push_back(pit->first);  }
		}
		
		p_dealloc(); //spektrumfelszabadítás
		for (int i = 0; i < obsolate.size(); i++){
			pm.erase(obsolate[i]); //út törlése a mátrixbol
		}
		
	}

	//Csinálunk egy ideiglenes map-ot ahol a még aktív védelmi utakhoz tartozó spektrum 1 a lejártat nem vesszeük figyelembe
	void p_dealloc(){
		ListGraph::EdgeMap<SpectrumState> temp_map(graph);
		for (auto pit = pm.begin(); pit != pm.end(); ++pit)
		{
			if (pit->second.timestamp > 0)
			{
				pathalloc(pit->second.path, temp_map,pit->second.pos,pit->second.width);
			}

		}
		
		ListGraph::EdgeIt eit(graph);
		for (eit; eit != INVALID; ++eit)
		{
			for (int i = 0; i < CH::channel_num; i++)
			{
				if (spectrum_map[eit][i] == -1){
					spectrum_map[eit][i] = temp_map[eit][i];
				}
			}
			
		}
	}
	//adott map-on lefoglalja az igényelt sávot egy path mentén
	void pathalloc(Path<ListGraph> &path, ListGraph::EdgeMap<SpectrumState> &temp_map,int pos,int width){
	
		Path<ListGraph>::ArcIt arc_it(path);
		for (arc_it; arc_it != INVALID; ++arc_it)
		{
			Node t = graph.target(arc_it);
			Node s = graph.source(arc_it);
			Edge e = lemon::findEdge(graph, t, s);
			for (int i = 0; i<width; i++){
				temp_map[e].carrier[pos + i] = -1;
			}
		}
	}
};

#endif	

