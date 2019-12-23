#ifndef CONTOUR_H
#define CONTOUR_H

#include <iostream>

class Segment
{
friend class Contour;
public:
    Segment( int s ) { _end=s; _height=0; }
private:
    int _end;
    int _height;
    Segment *_next, *_prev;
};

class Contour
{
public:
    Contour() { _Head = new Segment(0); _Head->_next=_Head; _Head->_prev=_Head; }
    ~Contour(){
        Segment* n = _Head->_next;
        while( n != _Head ){
            n = n->_next;
            delete n->_prev;
        }
        delete _Head;
    }
    int getmaxh( int ,int );
    void setheight( int , int, int );
    void reset();

private:
    Segment* _Head;
};

int Contour::getmaxh( int h, int t )
{
    if( _Head->_prev->_end < t ){
        Segment* newSeg = new Segment( t );
        newSeg->_prev = _Head->_prev;
        _Head->_prev->_next = newSeg;
        newSeg->_next = _Head;
        _Head->_prev = newSeg;
    }
    Segment* seg = _Head->_next;
    while( seg->_end <= h ) seg = seg->_next;
    int maxh = seg->_height;
    seg = seg->_next;
    while( t > seg->_prev->_end ){
        if( seg->_height > maxh ) maxh = seg->_height;
        seg = seg->_next;
    }
    return maxh;
}

void Contour::setheight( int h, int t, int nh )
{
    if( _Head->_prev->_end < t ){
        Segment* newSeg = new Segment( t );
        newSeg->_prev = _Head->_prev;
        _Head->_prev->_next = newSeg;
        newSeg->_next = _Head;
        _Head->_prev = newSeg;
    }
    Segment* seg_extend = _Head->_next;
    while( seg_extend->_end <= h ) seg_extend = seg_extend->_next;
    if( h > seg_extend->_prev->_end  ){
        Segment* newSeg = new Segment( h );
        newSeg->_height = seg_extend->_height;
        seg_extend->_prev->_next = newSeg;
        newSeg->_prev = seg_extend->_prev;
        newSeg->_next = seg_extend;
        seg_extend->_prev = newSeg;
    }
    if( t < seg_extend->_end ){
        Segment* newseg = new Segment( seg_extend->_end );
        newseg->_height = seg_extend->_height;
        newseg->_next = seg_extend->_next;
        seg_extend->_next->_prev = newseg;
        seg_extend->_next = newseg;
        newseg->_prev = seg_extend;
        seg_extend->_end = t;
        seg_extend->_height = nh;
    }
    else{
        seg_extend->_end = t;
        seg_extend->_height = nh;
        Segment* seg_delete = seg_extend->_next;
        while( t >= seg_delete->_end ){
            if( seg_delete == _Head ) break;
            seg_extend->_next = seg_delete->_next;
            seg_delete->_next->_prev = seg_extend;
            Segment* temp = seg_delete;
            seg_delete = seg_delete->_next;
            delete temp;
        }
    }
}

void Contour::reset()
{
    Segment* n = _Head->_next;
    while( n != _Head ){
        n = n->_next;
        delete n->_prev;
    }
    _Head->_next = _Head;
    _Head->_prev = _Head;
}

#endif