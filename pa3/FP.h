#ifndef FP_H
#define FP_H

#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include "BTree.h"
#include "Module.h"
#include "Contour.h"
#include "gnuplot_i.h"

#define const_c 100
#define const_k 7
#define alpha 0.5
#define initUpHillAcceptingProb 0.95
#define maxTn 500

extern time_t t1;
extern double alphagiven;

class FP
{
public:
    FP() {
        _BTree=new BTree;
        _BTree_backup=new BTree;
        _BTree_best=new BTree;
    }
    ~FP() {
        for( int i=0; i<_Modules.size(); ++i ) delete _Modules[i];
        delete _BTree;
        delete _BTree_backup;
        delete _BTree_best;
    }
    void Read( const string&, const string& );
    void Write( const string& );

    void fastSimulatedAnnealing();

    void Packing();
    void Perturb();
    void BTree_Backup();
    void BTree_Recover();   // would destroy backup ( only exchange _BTree & _BTree_backup )

    void BTree_KeepBest( bool = true );
    void BTree_RecoverBest();
    
    void EvaluateCost();

    bool Feasible();

    bool TranseposeorNot();

    void gnuplot();

private:
    BTree *_BTree, *_BTree_backup, *_BTree_best;
    double _Cost, _Cost_backup, _Cost_best;
    vector<Module*> _Modules;
    vector<Net> _Nets;
    int _outlineX, _outlineY;
    Contour _HoriContour;
    int _maxX, _maxY;

    // overall
    double _T1;
    double _AverageArea;

    // current temparature
    double _AverageCostChange;

    void Packing_recur( Node* );

    double TotalWireLength();
    double EvaluateWireLength( const Net& );

    void EvaluateAverageArea();
    void EvaluateAverateCostChange();

    void transposeFP();
};


#endif