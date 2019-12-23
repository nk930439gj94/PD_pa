#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main( int argc, char **argv )
{
    vector<int> vec(5,0);
    vec.assign(9,0);
    for( int i=0; i<vec.size(); ++i ){
        cout<<vec[i]<<" ";
    }
    cout<<endl;
}