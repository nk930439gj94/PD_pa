#include <iostream>
#include <limits>
#include <climits>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <windows.h>

using namespace std;


int main()
{
    time_t t1,t2;
    time(&t1);
    Sleep(5000);
    time(&t2);
    cout<<t2-t1<<endl;
}