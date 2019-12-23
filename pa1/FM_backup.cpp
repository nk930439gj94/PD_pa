#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include "BucketList.h"

using namespace std;


void strgettokens(string& str, vector<string>& v)       // for read: string to tokens
{
    size_t start=0;
    size_t end,temp;
    v.clear();
    while(true){
        while( str[start]==' ' || str[start]==';' ){
            ++start;
            if( start==string::npos ) return;
        }
        end = str.find(' ',start);
        if( str.find(';',start) < end )
            end = str.find(';',start);
        if( end==string::npos ) return;
        v.push_back( str.substr(start,end-start) );
        start=end+1;
    }
}


class FM
{
public:
    ~FM(){
        for( int i=0; i<_Cells.size(); ++i ) delete _Cells[i];
    }
    void read( const string& );
    void compute();
    void write( const string& );

private:
    double _balance_degree;
    BucketList _BucketList_A;
    BucketList _BucketList_B;
    vector< string > _Netname;
    vector< string > _Cellname;
    vector< vector<int> > _Nets;       // net id to cell id
    vector< vector<int> > _Cell2net;   // cell id to net id
    vector< Node* > _Cells;                 // cell id to cell address
    vector< bool > _Partition;              // cell id to group ( true/false: in B/A )
    int _size_A;
    vector< int > _Distribution;       // cell distribution ( number of cell in A of the given net )
    vector< bool > _locked;                 // cell is locked


    void initialize();      // initialize gain, locked
    void update_gain(Node*);       // updating gain

    int getcutsize();

};


void FM::read( const string &f )
{
    ifstream ifs(f);
    if( !ifs.is_open() ){
        cerr<<"Cannot open file "<<f<<" !\n";
        exit(1);
    }
    string line;
    getline(ifs,line);
    _balance_degree = stod(line);
    vector<string> tokens;
    unordered_map< string, int > cellidmapper;

    int netid = 0;
    while( getline(ifs,line) ){
        strgettokens(line,tokens);
        _Netname.push_back( tokens[1] );
        _Nets.push_back( vector<int>() );
        for( int i=2; i<tokens.size(); ++i ){
            if( cellidmapper.count(tokens[i]) == 0 ){
                _Cells.push_back( new Node( _Cells.size() ) );
                cellidmapper[ tokens[i] ] = cellidmapper.size();
                _Cellname.push_back( tokens[i] );
                _Cell2net.push_back( vector<int>() );
            }
            _Cell2net[ cellidmapper[ tokens[i] ] ].push_back( netid );
            _Nets[ netid ].push_back( cellidmapper[ tokens[i] ] );
        }
        ++netid;
    }
    ifs.close();

    // initialize partition
    _size_A = _Cells.size() / 2;
    for( int i=0; i<_Cell2net.size(); ++i ){
        if( 2*i < _Cell2net.size() ) _Partition.push_back(false);
        else _Partition.push_back(true);
    }

    // initialize buckets
    int pmax=0;
    for( int i=0; i<_Cell2net.size(); ++i ){
        if( _Cell2net[i].size() > pmax ) pmax = _Cell2net[i].size();
    }
    _BucketList_A.initialize( pmax );
    _BucketList_B.initialize( pmax );

    // initialize distribution
    for( int i=0; i<_Nets.size(); ++i )
        _Distribution.push_back(0);
    for( int i=0; i<_Nets.size(); ++i ){
        for( int j=0; j<_Nets[i].size(); ++j ){
            if( !_Partition[ _Nets[i][j] ] ) ++_Distribution[i];
        }
    }

    // initial locked
    for( int i=0; i<_Cells.size(); ++i ) _locked.push_back(false);
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
    for( int i=0; i<_locked.size(); ++i ) _locked[i] = false ;
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

        // check for critical nets after the move

        // from size = 0: decrement all cell gain on the net
        if( !_Partition[cellid] && _Distribution[netid]==0 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], -1 );
            }
        }
        else if( _Partition[cellid] && _Distribution[netid] == _Nets[netid].size() ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_locked[_Nets[netid][j]] )
                    _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], -1 );
            }
        }
        // from size = 1: increment gain of the only cell in "from block" on the net
        else if( !_Partition[cellid] && _Distribution[netid]==1 ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( !_Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_A.update_gain( _Cells[ _Nets[netid][j] ], 1 );
                    break;
                }
            }
        }
        else if( _Partition[cellid] && _Distribution[netid] == ( _Nets[netid].size()-1 ) ){
            for( int j=0; j<_Nets[netid].size(); ++j ){
                if( _Partition[_Nets[netid][j]] ){
                    if( !_locked[_Nets[netid][j]] )
                        _BucketList_B.update_gain( _Cells[ _Nets[netid][j] ], 1 );
                    break;
                }
            }
        }
    }
    
    _Partition[cellid] = !_Partition[cellid];
}

void FM::compute()
{
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
            if( _size_A < ( (1-_balance_degree)/2*_Cells.size() + 1 ) )     // block A can't be from block
                notbalance_1 = true;
            if( ( _size_A + 1 ) > (1+_balance_degree)/2*_Cells.size() )     // block A can't be to block
                notbalance_2 = true;

            int maxgain;
            Node* node_move;

            if( notbalance_1 && notbalance_2 ) break;   // can't move any cell
            else if( notbalance_1 && !notbalance_2 ){   // move a cell from B to A
                if( _BucketList_B.empty() ) break;      // all cells in B are locked
                maxgain = _BucketList_B.get_maxgain();
                node_move = _BucketList_B[maxgain];
            }
            else if( !notbalance_1 && notbalance_2 ){   // move a cell from A to B
                if( _BucketList_A.empty() ) break;      // all cells in A are locked
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
                if( maxgain == INT_MIN ) break;
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
    for( int i=0; i<_Partition.size(); ++i ){
        if( !_Partition[i] ) ofs<<_Cellname[i]<<" ";
    }
    ofs<<";\n";
    ofs<<"G2 "<<_Cells.size()-_size_A<<endl;
    for( int i=0; i<_Partition.size(); ++i ){
        if( _Partition[i] ) ofs<<_Cellname[i]<<" ";
    }
    ofs<<";";
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


int main( int argc, char** argv )
{
    if( argc!=3 ){
        cerr<<"Wrong command number\n";
        exit(1);
    }
    FM fm;
    fm.read( argv[1] );
    fm.compute();
    fm.write( argv[2] );
}