#include "parser.h"
#include "routingdb.h"
#include "tree.h"
#include <string>
#include <cstdlib>
#include "Router.h"

using namespace std;

ifstream inputFile;
ofstream outputFile;

RoutingDB db;


void ShowSyntax()
{
    cout << "syntax: " << endl;
    cout << "./gr [input_file] [output_file] " << endl;
    cout << endl;

    cout << "example: " << endl; 
    cout << "./gr input/adaptec1.capo70.2d.35.50.90.gr a1.out" << endl;

    cout << endl;
}

void HandleArgument(const int argc, char** argv)
{
    if (argc < 3)
    { ShowSyntax(); exit(1); }

    int arg_no = 1;

    /* input file */
    inputFile.open(argv[arg_no], ios::in);
    if (!inputFile)
    {
	cerr << "Could not open input file: " << argv[arg_no] << endl;
	exit(1);
    }

    arg_no++;

    /* output file */
    outputFile.open(argv[arg_no], ios::out);
    if (!outputFile)
    {
	cerr << "Could not open output file: " << argv[arg_no] << endl;
	exit(1);
    }

    arg_no++;
}


int main(int argc, char** argv) 
{
    HandleArgument(argc, argv); 


    {
        /* Parser */
        cout << "[Parser]" << endl;
        Parser parser;
        parser.ReadISPD(inputFile);
    }
    cout << endl;

    {
        RoutingTree tree;
        tree.MinimumSpanningTreeConstruction();
    }

    Router router;
    router.Routing_All();
	


    {
        cout << "[Verify]" << endl;
        char cmd[100];

        //sprintf(cmd, "./eval2008.pl %s %s", argv[1], argv[2]);
        sprintf(cmd, "./eval.pl %s %s", argv[1], argv[2]);
        cout << cmd << endl;
        system(cmd);
    }

    cout << endl;
    cout << endl;

    return 0;
}