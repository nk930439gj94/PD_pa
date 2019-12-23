#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

class Node
{
friend class BTree;
public:
    Node() {}
    Node( int i, bool f ) { _id=i; _left = _right = _parent = 0; _flip=f; }
    Node& operator=( const Node& n ){
        _left = n._left;
        _right = n._right;
        _parent = n._parent;
        _id = n._id;
        _flip = n._flip;
        return *this;
    }
    int getid() { return _id; }
    Node* getleft() { return _left; }
    Node* getright() { return _right; }
    Node* getparent() { return _parent; }
    bool getflip() { return _flip; }
private:
    Node *_left, *_right, *_parent;
    int _id;
    bool _flip;
};

class BTree
{
public:
    BTree() { _Root=0; _dummy=new Node; }
    ~BTree(){
        for( int i=0; i<_TreeNodes.size(); ++i ) delete _TreeNodes[i];
        delete _dummy;
    }

    BTree& operator=( const BTree& );

    void NewandInsert_rand( bool );

    void Rotate_rand();
    void PopandInsert_rand();
    void Swap_rand();
    int size() const { return _TreeNodes.size(); }
    Node* getRoot() { return _Root; }


private:
    Node* _Root;
    Node* _dummy;
    vector<Node*> _TreeNodes;

    void insert( Node*, Node* );
    void pop( Node* );

};

BTree& BTree::operator=( const BTree& bt )
{
    for( int i=_TreeNodes.size(); i<bt.size(); ++i ) _TreeNodes.push_back( new Node( i, false ) );
    for( int i=_TreeNodes.size(); i>bt.size(); --i ){
        delete _TreeNodes.back();
        _TreeNodes.pop_back();
    }
    _Root = _TreeNodes[ bt._Root->_id ];
    _Root->_parent = 0;
    for( int i=0; i<_TreeNodes.size(); ++i ){
        if( bt._TreeNodes[i]->_left == 0 ) _TreeNodes[i]->_left = 0;
        else _TreeNodes[i]->_left = _TreeNodes[ bt._TreeNodes[i]->_left->_id ];
        if( bt._TreeNodes[i]->_right == 0 ) _TreeNodes[i]->_right = 0;
        else _TreeNodes[i]->_right = _TreeNodes[ bt._TreeNodes[i]->_right->_id ];
        if( bt._TreeNodes[i] != bt._Root )
            _TreeNodes[i]->_parent = _TreeNodes[ bt._TreeNodes[i]->_parent->_id ];
        _TreeNodes[i]->_flip = bt._TreeNodes[i]->_flip;
    }
}

void BTree::NewandInsert_rand( bool flip )
{
    if( _TreeNodes.empty() ){
        _Root = new Node( _TreeNodes.size(), flip );
        _TreeNodes.push_back(_Root);
        _Root->_left = _Root->_right = _Root->_parent = 0;
    }
    else{
        int id = _TreeNodes.size();
        _TreeNodes.push_back( new Node( id, flip ) );
        int r = rand() % (id+1);
        if( r==id ) insert( _TreeNodes[id], 0 );
        else insert( _TreeNodes[id], _TreeNodes[r] );
    }
}

void BTree::insert( Node* insertnode, Node* pos )
{
    // insert insertnode into the left child or right child of pos
    if( pos==0 ){
        if( _Root == 0 ){
            _Root = insertnode;
            _Root->_left = _Root->_right = _Root->_parent = 0;
        }
        else if( rand() % 2 ){
            insertnode->_right = _Root;
            insertnode->_left = 0;
        }
        else{
            insertnode->_left = _Root;
            insertnode->_right = 0;
        }
        _Root->_parent = insertnode;
        insertnode->_parent = 0;
        _Root = insertnode;
    }
    else{
        Node* nd;
        if( rand() % 2 ){
            nd = pos->_right;
            pos->_right = insertnode;
        }
        else{
            nd = pos->_left;
            pos->_left = insertnode;
        }
        if( rand() % 2 ){
            insertnode->_right = nd;
            insertnode->_left = 0;
        }
        else{
            insertnode->_left = nd;
            insertnode->_right = 0;
        }
        if( nd!=0 )
            nd->_parent = insertnode;
        insertnode->_parent = pos;
    }
}

void BTree::pop( Node* deletenode )
{
    if( deletenode == _Root ){
        if( deletenode->_left==0 && deletenode->_right==0 ) _Root = 0;
        else if( deletenode->_left == 0 ){
            _Root = deletenode->_right;
            _Root->_parent = 0;
            cout<<"";
        }
        else if( deletenode->_right == 0 ){
            _Root = deletenode->_left;
            _Root->_parent = 0;
            cout<<"";
        }
        else{
            Node *nd, *nd2;
            if( rand() % 2 ){
                nd = deletenode->_right;
                nd2 = deletenode->_left;
                *_dummy = *nd;
                nd->_right = _dummy;
                nd->_left = nd2;
            }
            else{
                nd = deletenode->_left;
                nd2 = deletenode->_right;
                *_dummy = *nd;
                nd->_left = _dummy;
                nd->_right = nd2;
            }
            nd2->_parent = nd;
            _dummy->_parent = nd;
            _Root = nd;
            _Root->_parent = 0;
            cout<<"";
            pop(_dummy);
        }
    }
    else{
        bool isleftchild = false;
        if( deletenode->_parent->_left == deletenode ) isleftchild = true;

        if( deletenode->_left==0 && deletenode->_right==0 ){
            if( isleftchild )
                deletenode->_parent->_left = 0;
            else
                deletenode->_parent->_right = 0;
            cout<<"";
        }
        else if( deletenode->_left == 0 )
        {
            if( isleftchild )
                deletenode->_parent->_left = deletenode->_right;
            else
                deletenode->_parent->_right = deletenode->_right;
            deletenode->_right->_parent = deletenode->_parent;
            cout<<"";
        }
        else if( deletenode->_right == 0 ){
            if( isleftchild )
                deletenode->_parent->_left = deletenode->_left;
            else
                deletenode->_parent->_right = deletenode->_left;
            deletenode->_left->_parent = deletenode->_parent;
            cout<<"";
        }
        else{
            Node *nd, *nd2, *p = deletenode->_parent;
            if( rand() % 2 ){
                nd = deletenode->_right;
                nd2 = deletenode->_left;
                *_dummy = *nd;
                nd->_right = _dummy;
                nd->_left = nd2;
            }
            else{
                nd = deletenode->_left;
                nd2 = deletenode->_right;
                *_dummy = *nd;
                nd->_left = _dummy;
                nd->_right = nd2;
            }
            nd2->_parent = nd;
            _dummy->_parent = nd;
            if( isleftchild )
                p->_left = nd;
            else
                p->_right = nd;
            nd->_parent = p;
            cout<<"";
            pop(_dummy);
        }
    }
}

void BTree::Rotate_rand()
{
    int r = rand() % _TreeNodes.size();
    _TreeNodes[r]->_flip = !_TreeNodes[r]->_flip;
}

void BTree::PopandInsert_rand()
{
    int r1 = rand() % _TreeNodes.size();
    int r2 = rand() % ( _TreeNodes.size()+1 );
    while( r2==r1 ) r2 = rand() % ( _TreeNodes.size()+1 );
    cout<<"";
    pop( _TreeNodes[r1] );
    cout<<"";
    if( r2 == _TreeNodes.size() ) insert( _TreeNodes[r1], 0 );
    else insert( _TreeNodes[r1], _TreeNodes[r2] );
    cout<<"";
}

void BTree::Swap_rand()
{
    int r1 = rand() % _TreeNodes.size();
    int r2 = rand() % _TreeNodes.size();
    while( r2==r1 ) r2 = rand() % ( _TreeNodes.size() );
    int i = _TreeNodes[r1]->_id;
    bool f = _TreeNodes[r1]->_flip;
    _TreeNodes[r1]->_id = _TreeNodes[r2]->_id;
    _TreeNodes[r1]->_flip = _TreeNodes[r2]->_flip;
    _TreeNodes[r2]->_id = i;
    _TreeNodes[r2]->_flip = f;
    Node* n = _TreeNodes[r1];
    _TreeNodes[r1] = _TreeNodes[r2];
    _TreeNodes[r2] = n;
}

#endif