#include "Router.h"


int MHdistance( int x1, int y1, int x2, int y2 )
{
    if( y1==y2 ) return abs( x1 - x2 ) + abs( y1 - y2 );
    else return abs( x1 - x2 ) + abs( y1 - y2 ) + 2;
}


Router::~Router()
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo(); 
    if( _Grid != 0 ){
        for( int i=0; i<m; ++i ){
            for( int j=0; j<n; ++j ){
                delete [] _Grid[i][j];
                delete [] _Edges[i][j];
            }
            delete [] _Grid[i];
            delete [] _Edges[i];
        }
        delete [] _Grid;
        delete [] _Edges;
        delete [] _Black;
        delete [] _Grey;
    }
}

void Router::read()
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();

    // construct grid
    _Grid = new Node**[m];
    for( int i=0; i<m; ++i ){
        _Grid[i] = new Node*[n];
        for( int j=0; j<n; ++j )
            _Grid[i][j] = new Node[2];
    }
    for( int i=0; i<m; ++i ){
        for( int j=0; j<n; ++j ){
            for( int k=0; k<2; ++k )
                _Grid[i][j][k].set(i,j,k);
        }
    }

    // construc edges
    {
        int capaicty_h = db.GetLayerHoriCapacity(0), capacity_v = db.GetLayerVertiCapacity(1);
        _Edges = new vector<Edge>**[m];
        for( int i=0; i<m; ++i ){
            _Edges[i] = new vector<Edge>*[n];
            for( int j=0; j<n; ++j )
                _Edges[i][j] = new vector<Edge>[2];
        }
        for( int i=0; i<m; ++i ){
            for( int j=0; j<n; ++j ){
                // k=0
                if( i > 0 )
                    _Edges[i][j][0].emplace_back( &_Grid[i-1][j][0], capaicty_h );
                if( i < m-1 )
                    _Edges[i][j][0].emplace_back( &_Grid[i+1][j][0], capaicty_h );
                _Edges[i][j][0].emplace_back( &_Grid[i][j][1], INT_MAX );
                // k=1
                if( j > 0 )
                    _Edges[i][j][1].emplace_back( &_Grid[i][j-1][1], capacity_v );
                if( j < n-1 )
                    _Edges[i][j][1].emplace_back( &_Grid[i][j+1][1], capacity_v );
                _Edges[i][j][1].emplace_back( &_Grid[i][j][0], INT_MAX );
            }
        }
    }

    // adjust capacity
    for( int i=0; i<db.GetCapacityAdjustNo(); ++i ){
        int x1 = db.GetCapacityAdjust(i).GetGx1(), y1 = db.GetCapacityAdjust(i).GetGy1();
        int x2 = db.GetCapacityAdjust(i).GetGx2(), y2 = db.GetCapacityAdjust(i).GetGy2();
        int la = db.GetCapacityAdjust(i).GetLayer1() - 1;
        for( int j=0; j<_Edges[x1][y1][la].size(); ++j ){
            Edge &e = _Edges[x1][y1][la][j];
            if( e._node->_i == x2 && e._node->_j == y2 && e._node->_k == la ){
                e._capacity = db.GetCapacityAdjust(i).GetReduceCapacity();
                break;
            }
        }
        for( int j=0; j<_Edges[x2][y2][la].size(); ++j ){
            Edge &e = _Edges[x2][y2][la][j];
            if( e._node->_i == x1 && e._node->_j == y1 && e._node->_k == la ){
                e._capacity = db.GetCapacityAdjust(i).GetReduceCapacity();
                break;
            }
        }
    }
    
    // initial Black, Grey
    _Black = new bool[m*n*2];
    _Grey = new bool[m*n*2];
}


void Router::Routing_All()
{
    vector<int> NetPosition;        // Routing order
    for( int i=0; i<db.GetNetNo(); ++i ) NetPosition.emplace_back(i);
    sort( NetPosition.begin(), NetPosition.end(), NetTotalDistanceCompare() );

    for( int i=0; i<NetPosition.size(); ++i ){
        cout<<"\r"<<i;
        Routing_Net( db.GetNetByPosition( NetPosition[i] ), false );
    }
    cout<<endl;

    Reroute_init();
    for( int i=0; i<NetPosition.size(); ++i ){
        cout<<"\r"<<i;
        Routing_Net( db.GetNetByPosition( NetPosition[i] ), true );
    }
    outputFile.close();
    cout<<endl<<endl;
}

void Router::Routing_SubNet_init()
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    memset( _Black, false, sizeof(bool)*m*n*2 );
    memset( _Grey, false, sizeof(bool)*m*n*2 );
}

void Router::Routing_SubNet( SubNet& subnet )
{
    priority_queue< pair<double,Node*>, vector< pair<double,Node*> >, NodeDistanceCompare > pq;
    Routing_SubNet_init();
    Node *t = Pin2Node( subnet.GetTargetPin() );
    Node *min = Pin2Node( subnet.GetSourcePin() );

    {
        int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
        if( MHdistance( t->_i, t->_j, m/2, n/2 ) > MHdistance( min->_i, min->_j, m/2, n/2 ) ){
            Node* temp = t;
            t = min;
            min = temp;
        }
    }
    
    min->_d = 0;
    min->_Predecessor = 0;
    while( min!=t ){
        setBlack(min);
        vector<Edge> &e = _Edges[min->_i][min->_j][min->_k];
        for( int i=0; i<e.size(); ++i ){
            Node*& adjacent = e[i]._node;
            if( isBlack(adjacent) ) continue;
            double weight;
            if( e[i]._netid == subnet.GetNetUid() ) weight = 0;
            else weight = e[i]._weight;
            
            if( !isGrey(adjacent) ){
                adjacent->_d = (min->_d + weight);
                adjacent->_Predecessor = min;
                pq.emplace( adjacent->_d , adjacent );
                setGrey(adjacent);
            }
            else if( (min->_d + weight) < adjacent->_d ){
                adjacent->_d = (min->_d + weight);
                adjacent->_Predecessor = min;
                pq.emplace( adjacent->_d , adjacent );
            }
        }
        while( isBlack(min) ){
            min = pq.top().second;
            pq.pop();
        }
    }
}

void Router::traceback( SubNet& subnet, vector< pair<Node*,Node*> >& r, bool output )
{
    Node* target = Pin2Node( subnet.GetTargetPin() );
    if( target->_Predecessor == 0 ) target = Pin2Node( subnet.GetSourcePin() );
    while( target->_Predecessor != 0 ){
        Node*& p = target->_Predecessor;
        {
            vector<Edge>& e = _Edges[target->_i][target->_j][target->_k];
            for( int i=0; i<e.size(); ++i ){
                if( e[i]._node->_i == p->_i && e[i]._node->_j == p->_j && e[i]._node->_k == p->_k ){
                    if( e[i]._netid != subnet.GetNetUid() ){
                        e[i].IncreWeight();
                        e[i]._netid = subnet.GetNetUid();
                    }
                    break;
                }
            }
        }
        vector<Edge>& e = _Edges[p->_i][p->_j][p->_k];
        for( int i=0; i<e.size(); ++i ){
            if( e[i]._node->_i == target->_i && e[i]._node->_j == target->_j && e[i]._node->_k == target->_k ){
                if( e[i]._netid != subnet.GetNetUid() ){
                    e[i].IncreWeight();
                    e[i]._netid = subnet.GetNetUid();
                    if( output ) r.emplace_back(target,p);
                }
                break;
            }
        }
        target = target->_Predecessor;
    }
}

void Router::Routing_Net( Net& net, bool output )
{
    vector< pair<Node*,Node*> >result;
    for( int i=0; i < net.GetSubNetNo(); i++ ){
        SubNet& subnet = net.GetSubNet(i);
        Node* target = &_Grid[ subnet.GetTargetPinGx() ][ subnet.GetTargetPinGy() ][ subnet.GetTargetPinLayer()-1 ];
        Routing_SubNet( net.GetSubNet(i) );
        traceback( subnet, result, output );
    }
    
    
    if( output ){
        int x, y;
        outputFile<<net.GetName()<<" "<<net.GetUid()<<" "<<result.size()<<endl;
        for( int j=0; j<result.size(); ++j ){
            Node*& n1 = result[j].first, *&n2 = result[j].second;
            Grid2Coord( n1->_i, n1->_j, x, y );
            outputFile<<"("<<x<<","<<y<<","<<n1->_k+1<<")-";
            Grid2Coord( n2->_i, n2->_j, x, y );
            outputFile<<"("<<x<<","<<y<<","<<n2->_k+1<<")\n";
        }
        outputFile<<"!\n";
    }
}

bool Router::isBlack( Node*& node )
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    return _Black[ node->_i + m*node->_j + m*n*node->_k ];
}

bool Router::isGrey( Node*& node )
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    return _Grey[ node->_i + m*node->_j + m*n*node->_k ];
}

void Router::setBlack( Node*& node )
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    _Black[ node->_i + m*node->_j + m*n*node->_k ] = true;
}

void Router::setGrey( Node*& node )
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    _Grey[ node->_i + m*node->_j + m*n*node->_k ] = true;
}

void Router::Reroute_init()
{
    int m = db.GetHoriGlobalTileNo(), n = db.GetVertiGlobalTileNo();
    for( int i=0; i<m; ++i ){
        for( int j=0; j<n; ++j ){
            for( int k=0; k<2; ++k ){
                for( int l=0; l<_Edges[i][j][k].size(); ++l ){
                    _Edges[i][j][k][l]._netid = -1;
                    // if( _Edges[i][j][k][l]._weight <= 2 )
                    //     _Edges[i][j][k][l]._weight = 1;
                }
            }
        }
    }
}