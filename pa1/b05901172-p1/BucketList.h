#ifndef BUCKETLIST
#define BUCKETLIST

#include <cstring>
#include <climits>

using namespace std;

class BucketList;
class FM;


class Node
{
    friend class BucketList;
    friend class FM;
public:
    Node( int i ) :_id(i), _next(0), _prev(0), _gain(0) {}
    int getgain() { return _gain; }
    int getid() { return _id; }
private:
    const int _id;
    int _gain;
    Node* _next;
    Node* _prev;
};

class BucketList
{
public:
    BucketList() {}
    BucketList( int pmax ) { initialize( pmax ); }
    void initialize( int pmax ){
        _Pmax = pmax;
        _MaxGain = 0;
        _Size = 0;
        _Bucket = new Node*[ 2*_Pmax+1 ];
        memset( _Bucket, 0, sizeof(_Bucket) );
    }
    ~BucketList(){
        delete _Bucket;
    }

    BucketList& operator=(const BucketList& r){
        BucketList(r._Pmax);
        for( int i=0; i<(2*_Pmax+1); ++i )
            _Bucket[i] = r._Bucket[i];
    }
    Node*& operator[](int a){
        if( a<-_Pmax || a>_Pmax ){
            cerr<<"Index "<<a<<" out of range !\n";
            exit(1);
        }
        return _Bucket[ a+_Pmax ];
    }

    void insert( Node*, int );
    void remove( Node* );
    void update_gain( Node* n, int delta_g ){
        remove(n);
        insert( n, n->_gain+delta_g );
    }
    int get_maxgain(){
        return _MaxGain;
    }
    int size(){
        return _Size;
    }
    bool empty(){
        return ( _Size == 0 );
    }
    void clear(){
        for( int i=0; i<(2*_Pmax+1); ++i ) _Bucket[i] = 0;
        _Size = 0;
    }

private:
    int _Pmax;
    int _MaxGain;
    Node** _Bucket;
    int _Size;
};


void BucketList::insert( Node* n, int g )   // append the node to the end of the linked list
{
    if( g > _Pmax || g < -_Pmax ){
        cerr<<"Gain "<<g<<" out of range !\n";
        exit(1);
    }
    n->_gain = g;
    if( _Bucket[ g+_Pmax ] == 0 ){
        _Bucket[ g+_Pmax ] = n;
        n->_next = n;
        n->_prev = n;
        if( g > _MaxGain ) _MaxGain = g;
    }
    else{
        _Bucket[ g+_Pmax ]->_prev->_next = n;
        n->_prev = _Bucket[ g+_Pmax ]->_prev;
        _Bucket[ g+_Pmax ]->_prev = n;
        n->_next = _Bucket[ g+_Pmax ];
    }
    ++_Size;
}

void BucketList::remove( Node* n )
{
    if( _Size == 0 ){
        cerr<<"BucketList is empty !\n";
        exit(1);
    }
    if( n->_next == n ){
        _Bucket[ n->_gain + _Pmax ] = 0;
        if( n->_gain == _MaxGain ){
            if( _Size == 1 ) _MaxGain = INT_MIN;
            else{
                for( int i=_MaxGain-1; i>=-_Pmax; --i ){
                    if( _Bucket[ i+_Pmax ]!=0 ){
                        _MaxGain = i;
                        break;
                    }
                }
            }
        }
    }
    else{
        n->_prev->_next = n->_next;
        n->_next->_prev = n->_prev;
        if( _Bucket[ n->_gain+_Pmax ] == n )
            _Bucket[ n->_gain+_Pmax ] = n->_next;
    }
    --_Size;
}

#endif