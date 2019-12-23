#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cstdlib>
#include "BucketList.h"
#include <cassert>

using namespace std;


class FM
{
public:
    ~FM(){
        for( int i=0; i<_Cells.size(); ++i ) delete _Cells[i];
    }
    void read( const string& );
    void random_partition();
    void compute();
    void write( const string& );
    int getcutsize();
    vector<bool> getPartition() { return _Partition; }
    void setPartition( const vector<bool> & );

private:
    double _balance_degree;
    BucketList _BucketList_A;
    BucketList _BucketList_B;
    vector< string > _Cellname;
    vector< vector<int> > _Nets;       // net id to cell id
    vector< vector<int> > _Cell2net;   // cell id to net id
    vector< Node* > _Cells;                 // cell id to cell address
    vector< bool > _Partition;              // cell id to group ( true/false: in B/A )
    int _size_A;
    vector< int > _Distribution;       // cell distribution ( number of cell in A of the given net )
    vector< bool > _locked;                 // cell is locked


    void initialize();      // initialize gain, locked
    void calculate_distr();
    void update_gain(Node*);       // updating gain

};


void FM::read( const string &f )
{
    ifstream ifs(f);
    if( !ifs.is_open() ){
        cerr<<"Cannot open file "<<f<<" !\n";
        exit(1);
    }
    string token;
    ifs>>token;
    _balance_degree = stod(token);
    map< string, int > cellidmapper;

    ifs>>token;
    int netid = 0;
    while( ifs ){
        ifs>>token>>token;
        _Nets.emplace_back();
        set< int > hash;
        while( token!=";" ){
            if( cellidmapper.count( token ) == 0 ){
                int cellid = _Cells.size();
                _Cells.push_back( new Node( cellid ) );
                cellidmapper[ token ] = cellid;
                _Cellname.push_back( token );
                _Cell2net.emplace_back();
            }
            int cellid = cellidmapper[ token ];
            if( hash.count( cellid ) == 0 ){
                hash.insert( cellid );
                _Cell2net[ cellid ].push_back( netid );
                _Nets[ netid ].push_back( cellidmapper[ token ] );
            }
            ifs>>token;
        }
        ++netid;
        ifs>>token;
    }
    ifs.close();


    // initialize buckets
    int pmax=0;
    for( int i=0; i<_Cell2net.size(); ++i ){
        if( _Cell2net[i].size() > pmax ) pmax = _Cell2net[i].size();
    }
    _BucketList_A.initialize( pmax );
    _BucketList_B.initialize( pmax );

    // initialize partition
    _size_A = _Cells.size()/2;
    for( int i=0; i<_Cells.size(); ++i ){
        if( i < _Cells.size()/2 ) _Partition.push_back(false);
        else _Partition.push_back(true);
    }
    calculate_distr();
}

void FM::initialize()
{
    // initialize gain
    _BucketList_A.clear();
    _BucketList_B.clear();
    for( int i=0; i<_Cells.size(); ++i ){
        if( _Partition[i] ) _BucketList_B.insert( _Cells[i], 0 );
        else _BucketList_A.insert( _Cells[i], 0 );
    }
    for( int i=0; i<_Nets.size(); ++i ){
        for( int j=0; j<_Nets[i].size(); ++j ){
            // from size = 1
            if( !_Partition[_Nets[i][j]] && _Distribution[i]==1 )
                _BucketList_A.update_gain( _Cells[_Nets[i][j]], 1 );
            if( _Partition[_Nets[i][j]] && (_Nets[i].size()-_Distribution[i])==1 )
                _BucketList_B.update_gain( _Cells[_Nets[i][j]], 1 );
            
            // to size = 0
            if( !_Partition[_Nets[i][j]] && _Nets[i].size()==_Distribution[i] )
                _BucketList_A.update_gain( _Cells[_Nets[i][j]], -1 );
            if( _Partition[_Nets[i][j]] && _Distribution[i]==0 )
                _BucketList_B.update_gain( _Cells[_Nets[i][j]], -1 );
        }
    }

    // initial locked
    _locked.assign( _Cells.size(), false );
}

void FM::update_gain(Node *n)
{
    const int cellid = n->getid();
    _locked[cellid] = true;
    if( !_Partition[cellid] ){
        _BucketList_A.remove(n);
        --_size_A;
    }
    else{
        _BucketList_B.remove(n);
        ++_size_A;
    }

    for( int i=0; i<_Cell2net[cellid].size(); ++i ){
        const int netid = _Cell2net[cellid][i];
        // check critical net before move

        // to size = 0: increment gain of every cell on the net
        if( !_Partition[cellid] && _Distribution[netid] == _Nets[netid].size() ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], 1 );
            }
        }
        else if( _Partition[cellid] && _Distribution[netid]==0 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], 1 );
            }
        }
        // to size = 1: decrement gain of only cell in "to block"
        else if( !_Partition[cellid] && _Distribution[netid] == ( _Nets[netid].size()-1 ) ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( _Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], -1 );
                    break;
                }
            }
        }
        else if( _Partition[cellid] && _Distribution[netid]==1 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], -1 );
                    break;
                }
            }
        }

        // change distribution
        if( !_Partition[cellid] ) --_Distribution[netid];
        else ++_Distribution[netid];
        // temporary move the cell
        _Partition[cellid] = !_Partition[cellid];

        // check for critical nets after the move

        // from size = 0: decrement all cell gain on the net
        if( _Partition[cellid] && _Distribution[netid]==0 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], -1 );
            }
        }
        else if( !_Partition[cellid] && _Distribution[netid] == _Nets[netid].size() ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], -1 );
            }
        }
        // from size = 1: increment gain of the only cell in "from block" on the net
        else if( _Partition[cellid] && _Distribution[netid]==1 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], 1 );
                    break;
                }
            }
        }
        else if( !_Partition[cellid] && _Distribution[netid] == ( _Nets[netid].size()-1 ) ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( _Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], 1 );
                    break;
                }
            }
        }
        // move back the cell
        _Partition[cellid] = !_Partition[cellid];
    }
    // really move the cell
    _Partition[cellid] = !_Partition[cellid];
}

void FM::random_partition()
{
    _size_A = 0;
    int sizeA=0, sizeB=0, totalsize=_Cells.size();
    for( int i=0; i<_Cells.size(); ++i ){
        if( sizeA >= double( totalsize ) / 2 || sizeB >= double( totalsize ) / 2 )
            break;
        int r = rand() % 2;
        _Partition[i] = r;
        if( r ) ++sizeB;
        else{
            ++sizeA;
            ++_size_A;
        }
    }
    bool b = false;     // block A is full
    if( sizeA >= double(totalsize ) / 2 ) b = true;
    if(b){
        for( int i=sizeA+sizeB; i<_Cells.size(); ++i ) _Partition[i] = true;
    }
    else{
        _size_A += _Cells.size() - ( sizeA+sizeB );
        for( int i=sizeA+sizeB; i<_Cells.size(); ++i ) _Partition[i] = false;
    }
    
    calculate_distr();
}

void FM::calculate_distr()
{
    _Distribution.assign( _Nets.size(), 0 );
    for( int i=0; i<_Nets.size(); ++i ){
        for( int j=0; j<_Nets[i].size(); ++j ){
            if( !_Partition[ _Nets[i][j] ] ) ++_Distribution[i];
        }
    }
}

void FM::compute()
{
    int x=0;
    for( int i=0; i<_Cells.size(); ++i ){
        if( !_Partition[i] ) ++x;
    }

    // lhs: the best so far, rhs: current progress
    vector< bool > Partition_Max( _Partition );
    vector< int > Distribution_Max( _Distribution );
    int size_A_Max = _size_A;
    int G, maxG;
    while( true ){
        initialize();
        G=0;
        maxG=0;
        while( true ){
            bool notbalance_1 = false, notbalance_2 = false;
            if( _size_A - 1 < (1-_balance_degree)/2*_Cells.size() )     // block A can't be from block
                notbalance_1 = true;
            if( _size_A + 1 > (1+_balance_degree)/2*_Cells.size() )     // block A can't be to block
                notbalance_2 = true;

            int maxgain;
            Node* node_move;

            if( notbalance_1 && notbalance_2 )
                break;   // can't move any cell
            else if( notbalance_1 && !notbalance_2 ){   // move a cell from B to A
                if( _BucketList_B.empty() )
                    break;      // all cells in B are locked
                maxgain = _BucketList_B.get_maxgain();
                node_move = _BucketList_B[maxgain];
            }
            else if( !notbalance_1 && notbalance_2 ){   // move a cell from A to B
                if( _BucketList_A.empty() )
                    break;      // all cells in A are locked
                maxgain = _BucketList_A.get_maxgain();
                node_move = _BucketList_A[maxgain];
            }
            else{                                       // can move a cell in both blocks
                notbalance_1 = false;   // just borrow the variable
                maxgain = _BucketList_A.get_maxgain();
                if( _BucketList_B.get_maxgain() > maxgain ){
                    maxgain = _BucketList_B.get_maxgain();
                    notbalance_1 = true;
                }
                if( maxgain == INT_MIN )
                    break;
                else if( !notbalance_1 ) node_move = _BucketList_A[maxgain];
                else node_move = _BucketList_B[maxgain];
            }

            update_gain( node_move );

            G += maxgain;
            if( G > maxG ){
                maxG = G;
                Partition_Max = _Partition;
                Distribution_Max = _Distribution;
                size_A_Max = _size_A;
                
            }
            else if( G == maxG && G>0 ){
                if( abs( _size_A - double( _Cells.size()/2 ) ) < abs( size_A_Max - double( _Cells.size()/2 ) ) ){
                    Partition_Max = _Partition;
                    Distribution_Max = _Distribution;
                    size_A_Max = _size_A;
                }
            }
        }
        if( maxG > 0 ){
            _Partition = Partition_Max;
            _Distribution = Distribution_Max;
            _size_A = size_A_Max;
        }
        else break;
    }
}

void FM::write( const string& s )
{


    ofstream ofs( s );
    ofs<<"Cutsize = "<<getcutsize()<<endl;
    ofs<<"G1 "<<_size_A<<endl;
    int x=0;
    for( int i=0; i<_Partition.size(); ++i ){
        if( !_Partition[i] ){
            ofs<<_Cellname[i]<<" ";
            ++x;
        }
    }
    ofs<<";\n";
    ofs<<"G2 "<<_Cells.size()-_size_A<<endl;
    for( int i=0; i<_Partition.size(); ++i ){
        if( _Partition[i] ) ofs<<_Cellname[i]<<" ";
    }
    ofs<<";";
    ofs.close();
}

int FM::getcutsize()
{
    int cutsize = 0;
    for( int i=0; i<_Nets.size(); ++i ){
        bool A = false, B = false;
        for( int j=0; j<_Nets[i].size(); ++j ){
            if( !_Partition[ _Nets[i][j] ] ) A = true;
            else B = true;
            if( A && B ){
                ++cutsize;
                break;
            }
        }
    }
    return cutsize;
}

void FM::setPartition( const vector<bool> &v )
{
    if( _Partition.size() != _Cells.size() ){
        cerr<<"Cannot set the partition with cell number "<<_Partition.size()<<" !!\n";
        exit(1);
    }
    _Partition = v;
    _size_A = 0;
    for( int i=0; i<_Partition.size(); ++i ){
        if( !_Partition[i] ) ++_size_A;
    }
}


int main( int argc, char** argv )
{
    if( argc!=3 ){
        cerr<<"Wrong command number\n";
        exit(1);
    }
    FM fm;
    fm.read( argv[1] );
    fm.compute();

    int threshold = 0;
    int cutsize_min = fm.getcutsize();
    vector<bool> partition_min = fm.getPartition();

    srand( time(0) );
    while( threshold < 3 ){
        fm.random_partition();
        fm.compute();
        int newcutsize = fm.getcutsize();
        if( newcutsize < cutsize_min ){
            cutsize_min = newcutsize;
            partition_min = fm.getPartition();
            threshold = 0;
        }
        else
            ++threshold;
    }

    fm.setPartition( partition_min );
    fm.write( argv[2] );
}