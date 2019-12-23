#ifndef ROUTER_H
#define ROUTER_H

#include <climits>
#include <limits>
#include <queue>
#include <cmath>
#include <algorithm>
#include "routingdb.h"
#include <vector>
#include <cstring>

using namespace std;

extern RoutingDB db;
extern ofstream outputFile;

int MHdistance( int, int, int, int );



class Node
{
friend class Router;
friend class Edge;
friend struct NodeDistanceCompare;
public:
    Node() {}
private:
    Node* _Predecessor;
    double _d;
    int _i,_j,_k;

    void set( int i, int j, int k ) { _i=i; _j=j; _k=k; }
    void showgrid() { cout<<"("<<_i<<","<<_j<<","<<_k<<")"; }
};



struct NodeDistanceCompare
{
    bool operator()( const pair<double,Node*> &l, const pair<double,Node*> &r ){
        return l.first > r.first;
    }
};


class Edge
{
friend class Router;
public:
    Edge( Node* n, int c ) {
        _node=n;
        _netid=-1;
        _capacity=c;
        if( c == 0 ) _weight = numeric_limits<double>::infinity();
        else _weight = 1;
    }
private:
    Node* _node;
    int _netid;
    int _capacity;
    double _weight;
    void IncreWeight(){
        if( _capacity != INT_MAX && _capacity != 0 )
            _weight *= pow( 2, 2.0/_capacity );
    }
    void showInfo() { _node->showgrid(); cout<<" "<<_capacity<<" "<<_weight<<" "<<_netid<<endl; }
};


class Router
{
public:
    Router() { read(); }
    ~Router();
    void read();
    void Routing_All();

private:
    Node ***_Grid;
    vector<Edge> ***_Edges;         // adjacent list

    bool* _Black;
    bool* _Grey;

    void Routing_SubNet_init();
    void Routing_SubNet( SubNet& );
    void traceback( SubNet&, vector< pair<Node*,Node*> >&, bool );
    void Routing_Net( Net&, bool );

    Node* Pin2Node( Pin &p ){
        return &( _Grid[ p.GetGx() ][ p.GetGy() ][ 0 ] );
    }

    void Grid2Coord( int gx, int gy, int& x, int& y ){
        x = db.GetLowerLeftX() + db.GetTileWidth()/2 + db.GetTileWidth() * gx;
        y = db.GetLowerLeftY() + db.GetTileHeight()/2 + db.GetTileHeight() * gy;
    }

    bool isBlack(Node*&);
    bool isGrey(Node*&);
    void setBlack(Node*&);
    void setGrey(Node*&);

    void Reroute_init();

    void showEdges(){
        cout<<"All Edges: (capacity weight netid)\n";
        int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
        for( int k=0; k<2; ++k ){
            for( int j=0; j<n; ++j ){
                for( int i=0; i<m; ++i ){
                    cout<<i<<" "<<j<<" "<<k<<endl;
                    for( int l=0; l<_Edges[i][j][k].size(); ++l ){
                        cout<<"  ";
                        _Edges[i][j][k][l].showInfo();
                    }
                }
            }
        }
        cout<<endl;
    }
};


struct NetTotalDistanceCompare
{
    bool operator() ( int i, int j ){
        Net& l = db.GetNetByPosition(i), &r = db.GetNetByPosition(j);
        return totaldistance(l) < totaldistance(r);
    }
    int totaldistance( Net& n ){
        int totaldistance = 0;
        for( int i=0; i<n.GetSubNetNo(); ++i ){
            SubNet& subnet = n.GetSubNet(i);
            totaldistance += MHdistance( subnet.GetSourcePinGx(), subnet.GetSourcePinGy(), subnet.GetTargetPinGx(), subnet.GetTargetPinGy() );
        }
        return totaldistance;
    }
};


#endif