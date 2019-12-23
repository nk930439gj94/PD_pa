#include "BTree.h"
#include "Module.h"
#include "FP.h"
#include "FP.cpp"

clock_t t1;
double alphagiven;
bool visualize;

void PrintUsage(){
    cout<<"[usage]"<<endl;
    cout<<"[executable] [alpha] [input.block] [input.net] [outputfile] { -v | -visualize }\n";
}

int main( int argc, char** argv )
{
    visualize = false;
    if( argc==6 && ( string(argv[5]) == "-v" || string(argv[5]) == "-visualize" ) )
        visualize = true;
    else if( argc!= 5 ){
        PrintUsage();
        exit(1);
    }
    t1 = clock();
    srand( time(0) );
    alphagiven = stod( argv[1] );
    FP fp;
    fp.Read( argv[2], argv[3] );
    fp.fastSimulatedAnnealing();
    while( !fp.Feasible() ) fp.fastSimulatedAnnealing();
    fp.TranseposeorNot();
    fp.Write( argv[4] );
    if( visualize ) fp.gnuplot();
}