#ifndef MODULE_H
#define MODULE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Module
{
public:
    Module() {}
    Module( int id, string& name, int x, int y ) { _id=id; _name=name; _x=x; _y=y; _type=true; }
    int getx_left() { return _x; }
    int gety_down() { return _y; }
    virtual int getx_right() { return _x; }
    virtual int gety_up() { return _y; }
    virtual double getx_center() { return double(_x); }
    virtual double gety_center() { return double(_y); }
    void setx( int x ) { _x=x; }
    void sety( int y ) { _y=y; }
    bool gettype() { return _type; }
    virtual int getw() { return 0; }
    virtual int geth() { return 0; }
    virtual bool getflip() { return 0; }
    virtual void setflip( bool b ) {}
    string getname() { return _name; }
    virtual void transpose() {}


protected:
    int _x, _y;
    int _id;
    string _name;
    bool _type;  // false: Block, True: Terminal
};

class Block: public Module
{
public:
    Block( int id, string& name, int w, int h, bool f ) { _id=id; _name=name; _width=w; _height=h; _type=false; _flip=f; }
    int getx_right() { return ( _x + ( _flip ? _height : _width ) ); }
    int gety_up() { return ( _y + ( _flip ? _width : _height ) ); }
    double getx_center(){ return ( _x + ( _flip ? double(_height)/2 : double(_width)/2 ) ); }
    double gety_center(){ return ( _y + ( _flip ? double(_width)/2 : double(_height)/2 ) ); }
    int getw() { return _width; }
    int geth() { return _height; }
    bool getflip() { return _flip; }
    void setflip( bool b ) { _flip=b; }

    void transpose() { int temp=_x; _x=_y; _y=temp; _flip=!_flip; }


private:
    int _width, _height;
    bool _flip;
};

class Net
{
public:
    Net() {}
    void push_back( int i ) { _Cells.push_back(i); }
    int operator[]( int i ) const { return _Cells[i]; }
    int size() const { return _Cells.size(); }
private:
    vector<int> _Cells;
};

#endif