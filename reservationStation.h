#ifndef reservationStation_h
#define reservationStation_h
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
struct reservationStation
{
    string type;
    bool busy; //0 if not busy, 1 if busy
    string op;
    int Vj;
    int Vk;
    string Qj;
    string Qk;
    int dest;
    int A;
    int No_cycles;
};
#endif /* reservationStation_h */
