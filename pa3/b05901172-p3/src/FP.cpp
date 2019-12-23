#include "FP.h"

void FP::Read( const string& FileBlock, const string& FileNet  )
{
    ifstream ifs( FileBlock );
    string token;
    ifs>>token>>token;
    _outlineX = stoi( token );
    ifs>>token;
    _outlineY = stoi( token );
    ifs>>token>>token;
    int numBlocks = stoi( token );
    ifs>>token>>token;
    int numTerminals = stoi( token );
    string cellname;
    int x, y, id=0;
    unordered_map<string,int> moduleIdMapper;
    for( int i=0; i<numBlocks; ++i ){
        ifs>>cellname;
        ifs>>token;
        x = stoi( token );
        ifs>>token;
        y = stoi( token );
        bool f = rand() % 2;
        moduleIdMapper.emplace( cellname, id );
        _Modules.emplace_back( new Block( id++, cellname, x, y, f ) );
        _BTree->NewandInsert_rand( f );
    }
    for( int i=0; i<numTerminals; ++i ){
        ifs>>cellname;
        ifs>>token>>token;
        x = stoi( token );
        ifs>>token;
        y = stoi( token );
        moduleIdMapper.emplace( cellname, id );
        _Modules.emplace_back( new Module( id++, cellname, x, y ) );
    }
    ifs.close();

    ifs.open( FileNet );
    ifs>>token>>token;
    int numNets = stoi( token );
    _Nets.reserve( numNets );
    int netsize;
    for( int i=0; i<numNets; ++i ){
        ifs>>token>>token;
        netsize = stoi( token );
        _Nets.emplace_back();
        for( int j=0; j<netsize; ++j ){
            ifs>>token;
            _Nets.back().push_back( moduleIdMapper[token] );
        }
    }
}

void FP::Write( const string& outputfile )
{
    ofstream ofs( outputfile );
    double totalwirelength = TotalWireLength();
    int area = _maxX * _maxY;
    double cost = alphagiven * double(area) + ( 1.0 - alphagiven ) * totalwirelength;
    ofs<<fixed<<cost<<endl;
    ofs<<totalwirelength<<endl;
    ofs<<area<<endl;
    ofs<<_maxX<<" "<<_maxY<<endl;
    ofs<< ( clock()-t1 ) / double( CLOCKS_PER_SEC ) <<endl;
    for( int i=0; i<_BTree->size(); ++i ){
        Module*& m = _Modules[i];
        ofs<<m->getname();
        ofs<<" "<<m->getx_left()<<" "<<m->gety_down()<<" "<<m->getx_right()<<" "<<m->gety_up()<<endl;
    }
    ofs.close();
}

void FP::fastSimulatedAnnealing()
{
    _Cost_best = 99999;
    EvaluateAverageArea();
    const int numIteration_perTemp = 40 * _BTree->size() + 40;
    EvaluateAverateCostChange();
    _T1 = -1 * _AverageCostChange / log( initUpHillAcceptingProb );
    Packing();
    EvaluateCost();
    
    for( int i=0; i<numIteration_perTemp; ++i ){
        BTree_Backup();
        Perturb();
        Packing();
        EvaluateCost();
        if( _Cost_backup < _Cost ){
            if( ( rand() % 1000 ) >= ( initUpHillAcceptingProb * 1000 ) )
                BTree_Recover();
            else
                BTree_KeepBest();
        }
    }
    BTree_KeepBest(false);

    double T, P;
    for( int tn=1; tn<maxTn; ++tn ){
        EvaluateAverateCostChange();
        T = _T1 * _AverageCostChange / tn;
        if( tn <= const_k ) T /= const_c;
        BTree_RecoverBest();
        Packing();
        EvaluateCost();
        _Cost_best = _Cost;
        for( int i=0; i<numIteration_perTemp; ++i ){
            BTree_Backup();
            Perturb();
            Packing();
            EvaluateCost();
            if( _Cost_backup < _Cost ){
                P = exp( (_Cost_backup-_Cost) / T );
                if( (rand() % 1000) >= P*1000 )
                    BTree_Recover();
                else
                    BTree_KeepBest();
            }
        }
        BTree_KeepBest(false);
    }
    BTree_RecoverBest();
    Packing();
}

void FP::Packing()
{
    _HoriContour.reset();
    int rootid = _BTree->getRoot()->getid();
    _Modules[rootid]->setx( 0 );
    _Modules[rootid]->sety( 0 );
    _Modules[rootid]->setflip( _BTree->getRoot()->getflip() );
    if( !_BTree->getRoot()->getflip() ){
        _HoriContour.setheight( 0, _Modules[rootid]->getw(), _Modules[rootid]->geth() );
        _maxX = _Modules[rootid]->getw();
        _maxY = _Modules[rootid]->geth();
    }
    else{
        _HoriContour.setheight( 0, _Modules[rootid]->geth(), _Modules[rootid]->getw() );
        _maxY = _Modules[rootid]->getw();
        _maxX = _Modules[rootid]->geth();
    }
    Packing_recur( _BTree->getRoot() );
}

void FP::Packing_recur( Node* n )
{
    Node *ln = n->getleft(), *rn = n->getright();
    if( ln != 0 ){
        int head = _Modules[ n->getid() ]->getx_left();
        int tail, height, dh;
        if( !n->getflip() ) head += _Modules[ n->getid() ]->getw();
        else head += _Modules[ n->getid() ]->geth();
        _Modules[ ln->getid() ]->setx( head );
        _Modules[ ln->getid() ]->setflip( ln->getflip() );
        if( !ln->getflip() ){
            tail = head + _Modules[ ln->getid() ]->getw();
            dh = _Modules[ ln->getid() ]->geth();
        }
        else{
            tail = head + _Modules[ ln->getid() ]->geth();
            dh = _Modules[ ln->getid() ]->getw();
        }
        height = _HoriContour.getmaxh( head, tail );
        _HoriContour.setheight( head, tail, height+dh );
        _Modules[ ln->getid() ]->sety( height );
        if( tail > _maxX ) _maxX = tail;
        if( height + dh > _maxY ) _maxY = height + dh;
        Packing_recur( ln );
    }
    if( rn != 0 ){
        int head = _Modules[ n->getid() ]->getx_left();
        int tail, height, dh;
        _Modules[ rn->getid() ]->setx( head );
        _Modules[ rn->getid() ]->setflip( rn->getflip() );
        if( !rn->getflip() ){
            tail = head + _Modules[ rn->getid() ]->getw();
            dh = _Modules[ rn->getid() ]->geth();
        }
        else{
            tail = head + _Modules[ rn->getid() ]->geth();
            dh = _Modules[ rn->getid() ]->getw();
        }
        height = _HoriContour.getmaxh( head, tail );
        _HoriContour.setheight( head, tail, height+dh );
        _Modules[ rn->getid() ]->sety( height );
        if( tail > _maxX ) _maxX = tail;
        if( height + dh > _maxY ) _maxY = height + dh;
        Packing_recur( rn );
    }
}

void FP::Perturb()
{
    int r = rand() % 3;
    if( r==0 )
        _BTree->Rotate_rand();
    else if( r==1 )
        _BTree->PopandInsert_rand();
    else
        _BTree->Swap_rand();
}

void FP::BTree_Backup()
{
    *_BTree_backup = *_BTree;
    _Cost_backup = _Cost;
}

void FP::BTree_Recover()
{
    BTree* temp = _BTree_backup;
    _BTree_backup = _BTree;
    _BTree = temp;
    _Cost = _Cost_backup;
}

void FP::BTree_KeepBest( bool frombackup )
{
    if( frombackup ){
        if( _Cost_backup < _Cost_best ){
            *_BTree_best = *_BTree_backup;
            _Cost_best = _Cost_backup;
        }
    }
    else{
        if( _Cost < _Cost_best ){
            *_BTree_best = *_BTree;
            _Cost_best = _Cost;
        }
    }
}

void FP::BTree_RecoverBest()
{
    *_BTree = *_BTree_best;
    _Cost = _Cost_best;
}

void FP::EvaluateCost()
{
    double AspectRatio, Ratio;
    AspectRatio = double( _outlineY ) / _outlineX;
    if( AspectRatio < 1.0 ){
        AspectRatio = 1 / AspectRatio;
    }
    Ratio = double( _maxY ) / _maxX;
    if( Ratio < 1.0 ){
        Ratio = 1 / Ratio;
    }
    double Area_norm = ( _maxX * _maxY ) / _AverageArea;
    double R2 = ( Ratio - AspectRatio ) * ( Ratio - AspectRatio );
    _Cost = 0.5*( alpha * Area_norm + ( 1 - alpha ) * R2 ) + 0.5*double( 1 -Feasible() );
}

void FP::EvaluateAverageArea()
{
    const int numIter = 20 * _BTree->size() + 20;
    Packing();
    double Sum_Area =  _maxX * _maxY;
    for( int i=1; i<numIter; ++i ){
        Perturb();
        Packing();
        Sum_Area += ( _maxX * _maxY );
    }
    _AverageArea = Sum_Area / numIter;
}

void FP::EvaluateAverateCostChange()
{
    const int numIter = 10 * _BTree->size() + 10;
    Packing();
    EvaluateCost();
    BTree_Backup();
    double sum_CostChange = 0;
    double prevCost = _Cost;
    for( int i=0; i<numIter; ++i ){
        Perturb();
        Packing();
        EvaluateCost();
        sum_CostChange += abs( _Cost - prevCost );
        prevCost = _Cost;
    }
    BTree_Recover();
    _AverageCostChange = sum_CostChange / numIter;
}

bool FP::Feasible()
{
    if( ( _maxX < _outlineX && _maxY < _outlineY ) || ( _maxX < _outlineY && _maxY < _outlineX ) )
        return true;
    else
        return false;
}

void FP::transposeFP()
{
    for( int i=0; i<_BTree->size(); ++i )
        _Modules[i]->transpose();
    int temp = _maxX;
    _maxX = _maxY;
    _maxY = temp;
}

bool FP::TranseposeorNot()
{
    if( _maxX < _outlineX && _maxY < _outlineY )
        return true;
    if( _maxX < _outlineY && _maxY < _outlineX ){
        transposeFP();
        return true;
    }
    return false;
}

double FP::EvaluateWireLength( const Net& n )
{
    double l=_outlineX, d=_outlineY, r=0, u=0;
    double x, y;
    for( int i=0; i<n.size(); ++i ){
        x = _Modules[ n[i] ]->getx_center();
        y = _Modules[ n[i] ]->gety_center();
        if( x < l ) l = x;
        if( x > r ) r = x;
        if( y < d ) d = y;
        if( y > u ) u = y;
    }
    return ( r - l ) + ( u - d );
}

double FP::TotalWireLength()
{
    double w = 0;
    for( int i=0; i<_Nets.size(); ++i ) w += EvaluateWireLength( _Nets[i] );
    return w;
}

void FP::gnuplot()
{
    Gnuplot gnu;
    gnu.reset_plot();
    gnu.set_xrange( 0, 1.2 * _outlineX ).set_yrange( 0, 1.2 * _outlineY );

    stringstream ssj, ss5, ss6, ss7, ss8;
    ssj << 20000;
    ss5 << 0;
    ss6 << 0;
    ss7 << _outlineX;
    ss8 << _outlineY;

    gnu << ("set object " + ssj.str() + " rect from "
            + ss5.str() + "," + ss6.str() + " to "
            + ss7.str() + "," + ss8.str() + " fc rgb \"#E0E0E0\" behind\n");
    
    for(unsigned i = 0; i < _BTree->size(); ++i){
        stringstream ssi, ss1, ss2, ss3, ss4, ssmx,ssmy;
        ssi << i+1;
        ss1 << _Modules[i]->getx_left();
        ss2 << _Modules[i]->gety_down();
        ss3 << _Modules[i]->getx_right();
        ss4 << _Modules[i]->gety_up();
        ssmx << _Modules[i]->getx_center();
        ssmy << _Modules[i]->gety_center();
        string name = _Modules[i]->getname();

        gnu << ("set object " + ssi.str() + " rect from "
            + ss1.str() + "," + ss2.str() + " to "
            + ss3.str() + "," + ss4.str() + " fc rgb \"#FFFF99\" front \n");

        gnu << ("set label \"" + _Modules[i]->getname() + "\" " + " at " 
            + ssmx.str() + "," + ssmy.str() + " " + " front center\n");

    }

    gnu.plot();
    cin.get();

}