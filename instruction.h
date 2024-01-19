
#ifndef instruction_h
#define instruction_h
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "registers.h"
using namespace std;
struct instruction
{
    string type;
    registers reg;
    int rd;
    int rs1;
    int rs2;
    int imm;
    string state;
    int result;
    bool ready;
    int ROB_ENTRY;
    int PC;
};
#endif /* instruction_h */
